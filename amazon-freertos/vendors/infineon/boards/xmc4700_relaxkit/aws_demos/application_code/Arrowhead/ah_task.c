/*
 * Copyright (C) 2021 Infineon Technologies AG.
 *
 * Licensed under the EVAL_XMC47_PREDMAIN_AA Evaluation Software License
 * Agreement V1.0 (the "License"); you may not use this file except in
 * compliance with the License.
 *
 * For receiving a copy of the License, please refer to:
 *
 * https://github.com/Infineon/pred-main-xmc4700-kit/LICENSE.txt
 *
 * Licensee acknowledges that the Licensed Items are provided by Licensor free
 * of charge. Accordingly, without prejudice to Section 9 of the License, the
 * Licensed Items provided by Licensor under this Agreement are provided "AS IS"
 * without any warranty or liability of any kind and Licensor hereby expressly
 * disclaims any warranties or representations, whether express, implied,
 * statutory or otherwise, including but not limited to warranties of
 * workmanship, merchantability, fitness for a particular purpose, defects in
 * the Licensed Items, or non-infringement of third parties' intellectual
 * property rights.
 * Created on: 28 Apr 2022
 *   Author: Johansson
 *
 */

#include <string.h>
#include <stdio.h>

#include "ah_task.h"
#include "ah_config.h"

#include "iot_config.h"
#include "iot_wifi.h"
#include "iot_https_client.h"
#include "iot_https_utils.h"
#include "http_parser.h"
//#include "iot_mqtt_agent.h"
//#include "types/iot_mqtt_types.h"

#include "iot_demo_logging.h"
#include "iot_network_manager_private.h"
#include "platform/iot_threads.h"
#include "iot_init.h"

#include "app_types.h"
#include "json/json_sensor.h"
#include "converting.h"
#include "base64.h"
#include "float_to_string.h"
#include "led.h"

#include "i2c_mux.h"
#include "DAVE.h"

#include "aws_nbiot.h"
#include "iot_secure_sockets.h"

#include "types/iot_mqtt_types.h"

/* Packaging for Sensor data */
static InfineonSensorsMessage_t xSensorsMessage;

static IotNetworkManagerSubscription_t subscription = IOT_NETWORK_MANAGER_SUBSCRIPTION_INITIALIZER;

static IotSemaphore_t xNetworkSemaphore;

static uint32_t ulConnectedNetwork = AWSIOT_NETWORK_TYPE_NONE;

/** Handle for the sensors queue */
QueueHandle_t xSensorAhMessageQueueHandle = NULL;

static InfineonSensorsMessage_t xSensorsMessage;

static uint8_t pcAHBuffer[ ahtaskSEND_BUFFER_SIZE ];

/** Handle for the task */
TaskHandle_t xAhTaskHandle = NULL;
TaskHandle_t xAhRobustTaskHandle = NULL;

/* Variables used to indicate the connected network. */
static IotMqttError_t xIotMqttState = IOT_MQTT_SUCCESS;

static eConnectionState_t eConnStatus = eNetworkError;
static IotMutex_t xNetworkMutex;

static uint8_t ucPingErrorCount = 0;
static uint8_t ucPublishErrorCount = 0;

static void prvAhTask( void *pvParameters );

//static BaseType_t prvCreateRobustTask ( void );
//static void prvDeleteRobustTask( void );
static BaseType_t prvNetworkConnectionRestart( void );
//static BaseType_t prvAhRegister( void );
static BaseType_t prvAhOrchestrate( void );
static BaseType_t prvAhReorchestrate( void );
//static ePingStatus_t prvPing( uint8_t *pucIPAddr, uint16_t usCount, uint32_t ulIntervalMS );

/** @brief Publishes service to Service Registry and completes Orchestration process */
static BaseType_t prvAhRegisterAndOrchestrate( void );

void vAhTaskStart( void )
{
    if( xAhTaskHandle == NULL )
    {
        xTaskCreate( prvAhTask, "AHTask", ahtaskSTACK_SIZE, NULL, ahtaskPRIORITY, &xAhTaskHandle );
    }
}

void vAhTaskDelete( void )
{
    configPRINTF( ("Deleting the AH task") );
    if( xAhTaskHandle != NULL )
    {
        vTaskSuspend( xAhTaskHandle );
    }
    if( xAhTaskHandle != NULL )
    {
        /* TDOD: Add unregister of service */

        /** Delete the queue. We need to do that atomically since the sensor task could try to send to the invalidated queue */
        taskENTER_CRITICAL();
        vQueueDelete( xSensorAhMessageQueueHandle );
        xSensorAhMessageQueueHandle = NULL;
        taskEXIT_CRITICAL();
    }
    /* Delete the task */
    if( xAhTaskHandle != NULL )
    {
        vTaskDelete( xAhTaskHandle );
        xAhTaskHandle = NULL;
    }
}

void prvAhTask( void *pvParameters )
{
	( void )pvParameters;

    if( I2C_MUX_cWait( I2C_MUX_TIMEOUT ) < 0 )
    {
    	vFullReset( APP_ERROR_I2C_MUX_NOT_RELEASED );
    }

    LED_xStatus( MQTT, START );

    /* Create an Publish service to SR and Orchestrate */
    BaseType_t xStatus = prvAhRegisterAndOrchestrate();
    if( xStatus == pdPASS )
    {
    	LED_xStatus( MQTT, SUCCESS );
    }
    else
    {
    	LED_xStatus( MQTT, FAILED );
    	vFullReset( MQTT_AGENT_INIT_ERROR );
    }

    // TODO: Add EventHandler connection config for HTTPS request

    if( xStatus == pdPASS )
	{
		/** Initialize the Sensors Data Queue */
		xSensorAhMessageQueueHandle = xQueueCreate( ahtaskSENSOR_RECEIVE_QUEUE_LENGTH, sizeof( InfineonSensorsMessage_t ) );

		if( xSensorAhMessageQueueHandle == NULL )
		{
			xStatus = pdFAIL;
		}
	}

    if( xStatus == pdPASS )
	{
		/** Create non-recursive mutex */
		if( IotMutex_Create( &xNetworkMutex, false ) != true )
		{
			IotLogError( "Failed to create semaphore to wait for a network connection." );
			xStatus = pdFAIL;
		}
	}

    if( xStatus == pdPASS )
    {
    	eConnStatus = eConnEstablished;
    	//xStatus = prvCreateRobustTask();
    }

    if ( xStatus == pdPASS )
    {
    	vSensorTaskStart();

    	for( ;; )
    	{
    		if( eConnStatus == eConnEstablished )
    		{
    			if( xQueueRecieve( xSensorAhMessageQueueHandle, &xSensorsMessage, portMAX_DELAY ) )
    			{
    				configPRINTF( ("Queue Receive\r\n") );

    				// Might have to split json
#if AH_OUTPUT_FORMAT_JSON
    				bool bRet = JSON_bGenerateToSend( &xSensorsMessage, (char*)pcAHBuffer, sizeof(pcAHBuffer) );
    				if( !bRet )
    				{
    					configPRINTF( ("Generate JSON failed\r\n") );
    				}
#else
    				CSV_vGenerateToSend( &xSensorMessage, pcAHBuffer );
#endif

    				//TODO send to eventhandler
    				if( xIotMqttState == IOT_MQTT_SUCCESS )
    				{
    					IotMutex_Lock( &xNetworkMutex );

    					// xStatus = EH_Publish
    					if( xStatus == 0 /* http code ok*/) {
    						LED_xStatus( MESSAGE, SUCCESS );
    						configPRINTF( ("Message sent successfully\r\n") );
    						ucPublishErrorCount = 0;
    					}
    					else
    					{
    						LED_xStatus( MESSAGE, FAILED );
							configPRINTF( ("Message was not sent: %d \r\n", xStatus) );

							if( ++ucPublishErrorCount >= ATTEMPTS_COUNT )
							{
								xIotMqttState = IOT_MQTT_NETWORK_ERROR;
								eConnStatus = eEHError2;

								ucPublishErrorCount = 0;
							}
    					}

    					IotMutexUnlock( &xNetworkMutex );
    				}
    			}
    			else
    			{
    				configPRINTF( ("Stopping AH task \r\n") );
    				break;
    			}
    		} /* if( eConnStatus == eConnEstablished ) */
    		else
    		{
    			IotMutex_Lock( &xNetworkMutex );

    			switch( eConnStatus )
    			{
    			case eNetworkError2:
    				configPRINTF( ("eNetworkError\r\n") );
    				if( prvNetworkConnectionRestart() == pdPASS )
    				{
    					eConnStatus = eConnEstablished2;
    				}
    				else
    				{
    					vFullReset( NETWORK_INIT_ERROR );
    				}
    				break;
    			case eSRError2:
    				if( prvAhRegisterAndOrchestrate() == pdPASS )
    				{
    					xIotMqttState = IOT_MQTT_SUCCESS;
    					eConnStatus = eConnEstablished2;
    				}
    				else
    				{
    					eConnStatus = eNetworkError2;
    				}
    				break;
    			case eORError2:
    			case eEHError2:
    				if( prvAhReorchestrate() == pdPASS )
    				{
    					eConnStatus = eConnEstablished;
    				}
    				else
    				{
    					eConnStatus = eSRError2;
    				}
    				break;
    			default:
    				configASSERT( false );
    			} /* switch( eConnStatus) */

    			vTaskDelay( pdMS_TO_TICKS( RECONNECT_DELAY ) );

    			IotMutex_Unlock( &xNetworkMutex );
    		} /* else ( eConnStatus == eConnEstablished ) */
    	} /* for( ;; ) */
    } /* if( xStatus == pdPASS ) */

    vTaskDelete( NULL );

}

BaseType_t prvNetworkConnectionRestart( void )
{
	int status = EXIT_SUCCESS;
	ulConnectedNetwork = AWSIOT_NETWORK_TYPE_NONE;

	if( WIFI_ENABLED ) configPRINTF( ("Restart WiFi connection\r\n") );
	else if( NBIOT_ENABLED ) configPRINTF( ("Restart NB-IoT connection\r\n") );

	/** Disable networks. */
	vTaskDelay( 1 );


	if( true /* if deregistered from SR */ )
	{
		configPRINTF( ("Disabling network\r\n") );
		if( AwsIotNetworkManager_DisableNetwork( configENABLED_NETWORKS ) != configENABLED_NETWORKS )
		{
			IotLogError( "Failed to disable network.\r\n" );
			status = EXIT_FAILURE;
//			vFullReset(NETWORK_INIT_ERROR);
		}
		else
		{
			configPRINTF( ("Network disabled.\r\n") );
			status = EXIT_SUCCESS;
		}
	}

	/** Initialize all the  networks configured for the device. */
	if( status == EXIT_SUCCESS )
	{
		configPRINTF( ("Connecting to network\r\n") );
		if( AwsIotNetworkManager_EnableNetwork( configENABLED_NETWORKS ) != configENABLED_NETWORKS )
		{
			IotLogError( "Failed to initialize all the networks configured for the device." );
			status = EXIT_FAILURE;
//			vFullReset(NETWORK_INIT_ERROR);
		}
	}

	if( status == EXIT_SUCCESS )
	{
		/** Wait for network configured for the demo to be initialized. */
		ulConnectedNetwork = AwsIotNetworkManager_GetConnectedNetworks() & configENABLED_NETWORKS;

		if( ulConnectedNetwork == AWSIOT_NETWORK_TYPE_NONE )
		{
			/* Network not yet initialized. Block for a network to be intialized. */
			configPRINTF( ("No networks connected for the demo. Waiting for a network connection. \r\n") );
			IotLogInfo( "No networks connected for the demo. Waiting for a network connection. " );
			IotSemaphore_Wait( &xNetworkSemaphore );
			ulConnectedNetwork = AwsIotNetworkManager_GetConnectedNetworks() & configENABLED_NETWORKS;
		}

		/* Try to connect */
		{
			/* Create an MQTT agent and connect to the broker */
			BaseType_t xStatus = prvAhRegisterAndOrchestrate();
			vTaskDelay( 1 );

			return xStatus;
		}
	}

	return ( status == EXIT_SUCCESS ) ? pdPASS : pdFAIL;
}

static IotHttpsConnectionInfo_t xSrConnConfig = { 0 };

BaseType_t prvAhRegister( void )
{
	IotHttpsReturnCode_t xhttpsClientStatus;

	IotNetworkInterface_t* pNetwork;
	pNetwork = AwsIotNetworkManager_GetNetworkInterface(configENABLED_NETWORKS);


	//const char * pPath = AH_SERVICEREGISTRY_REGISTER_PATH;
	//size_t pathLen = sizeof(AH_SERVICEREGISTRY_REGISTER_PATH);
	const char* pPath = "/serviceregistry/echo";
	size_t pathLen = sizeof("/serviceregistry/echo");

	const char * pAddress = AH_SERVICEREGISTRY_ADDRESS;
	size_t addressLen = sizeof(AH_SERVICEREGISTRY_ADDRESS);

	uint32_t uiconnAttempt = 0;

	IotHttpsConnectionInfo_t connInfo = IOT_HTTPS_CONNECTION_INFO_INITIALIZER;
	IotHttpsConnectionHandle_t connHandle = IOT_HTTPS_CONNECTION_HANDLE_INITIALIZER;

	uint8_t* pConnUserBuffer = (uint8_t*)malloc(connectionUserBufferMinimumSize);

	xSrConnConfig.pAddress = pAddress;
	xSrConnConfig.addressLen = addressLen;
	xSrConnConfig.port = AH_SERVICEREGISTRY_PORT;
	xSrConnConfig.flags = 0;
	xSrConnConfig.pCaCert = AH_HTTPS_ROOT_CA;
	xSrConnConfig.caCertLen = sizeof(AH_HTTPS_ROOT_CA);
	xSrConnConfig.userBuffer.pBuffer = pConnUserBuffer;
	xSrConnConfig.userBuffer.bufferLen = connectionUserBufferMinimumSize;
	xSrConnConfig.pClientCert = AH_HTTPS_CLIENT_CA;
	xSrConnConfig.clientCertLen = sizeof(AH_HTTPS_CLIENT_CA);
	xSrConnConfig.pPrivateKey = AH_HTTPS_PRIVATE_KEY;
	xSrConnConfig.privateKeyLen = sizeof(AH_HTTPS_PRIVATE_KEY);

	xSrConnConfig.pNetworkInterface = pNetwork;

	uint32_t userBufferSize = connectionUserBufferMinimumSize + 256;
	uint8_t* pRequestUserBuffer = (uint8_t*)malloc(userBufferSize);

	IotHttpsSyncInfo_t reqSyncInfo = IOT_HTTPS_SYNC_INFO_INITIALIZER;

	uint32_t bodyBufferLen = 4096;
	uint8_t* pBodyBuffer = (uint8_t*)malloc(bodyBufferLen);


	reqSyncInfo.pBody = pBodyBuffer;
	reqSyncInfo.bodyLen = bodyBufferLen;

	IotHttpsRequestInfo_t reqConfig;

	reqConfig.pPath = AH_SERVICEREGISTRY_REGISTER_PATH;
	reqConfig.pathLen = sizeof(AH_SERVICEREGISTRY_REGISTER_PATH);
	reqConfig.method = IOT_HTTPS_METHOD_POST;
	reqConfig.pHost = AH_SERVICEREGISTRY_ADDRESS;
	reqConfig.hostLen = sizeof(AH_SERVICEREGISTRY_ADDRESS);
	reqConfig.isNonPersistent = true;
	reqConfig.userBuffer.pBuffer = pRequestUserBuffer;
	reqConfig.userBuffer.bufferLen = userBufferSize;
	reqConfig.isAsync = false;
	reqConfig.u.pSyncInfo = &reqSyncInfo;

	IotHttpsReturnCode_t httpsClientStatus = IotHttpsClient_Init();

	if( httpsClientStatus != IOT_HTTPS_OK )
	{
        IotLogError( "An error occurred initializing the HTTPS library. Error code: %d", httpsClientStatus );
        return pdFAIL;
	}

	xhttpsClientStatus = IotHttpsClient_Connect( &connHandle, &xSrConnConfig );

	if( httpsClientStatus != IOT_HTTPS_OK )
	{
		IotLogError( "An error occured when establishing a connection to SR" );
		return pdFAIL;
	}

	IotHttpsRequestHandle_t reqHandle;

	xhttpsClientStatus = IotHttpsClient_InitializeRequest( &reqHandle, &reqConfig );

	if( xhttpsClientStatus != IOT_HTTPS_OK )
	{
		IotLogError( "An error occured when initializing the request to SR" );
		return pdFAIL;
	}

	IotHttpsResponseHandle_t respHandle;

	uint32_t bodyRespBufferLen = 0;
	uint8_t* pRespBodyBuffer = NULL;

	IotHttpsSyncInfo_t respSyncInfo;

	respSyncInfo.pBody = pRespBodyBuffer;
	respSyncInfo.bodyLen = bodyRespBufferLen;

	xhttpsClientStatus = IotHttpsClient_SendSync( connHandle, reqHandle, &respHandle, &respSyncInfo, 0 );

	if( xhttpsClientStatus != IOT_HTTPS_OK )
	{
		IotLogError( "An error occured during the request to SR" );
		return pdFAIL;
	}

	uint16_t httpStatusCode;

	xhttpsClientStatus = IotHttpsClient_ReadResponseStatus(respHandle, &httpStatusCode );

	if( xhttpsClientStatus != IOT_HTTPS_OK )
	{
		IotLogError( "An error occured when reading the status code" );
		return pdFAIL;
	}

	if(httpStatusCode != IOT_HTTPS_STATUS_OK )
	{
		IotLogError( "Something went wrong with the request" );
		return pdFAIL;
	}
	return pdPASS;
}
