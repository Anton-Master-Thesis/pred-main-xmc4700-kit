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

//#include "iot_demo_logging.h"
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

#include "cJSON.h"
#include "core_http_config.h"

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
//static BaseType_t prvAhOrchestrate( void );

//static ePingStatus_t prvPing( uint8_t *pucIPAddr, uint16_t usCount, uint32_t ulIntervalMS );

/** @brief Publishes service to Service Registry and completes Orchestration process */
static BaseType_t prvAhRegisterAndOrchestrate( void );

static BaseType_t prvParseOrchResponse(const char* responseBody);
//static BaseType_t prvEhPublish( const char* publishPayload);
static BaseType_t prvBuildEhPublishRequest(const char* publishPayload, char* requestBody);


typedef struct Ah_address
{
	const char* pHost;
	size_t hostSize;
	uint16_t port;
	const char* pPath;
	size_t pathSize;
}EHInfo_t;

EHInfo_t ehInfo = { 0 };

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

BaseType_t prvAhRegister( void )
{
	int statusCode;
	char respBody[2048];

	BaseType_t xReqSuccess = pdFAIL;
	while (xReqSuccess == pdFAIL)
	{
		xReqSuccess = prvSendRequest(
				"POST",
				AH_SERVICEREGISTRY_ADDRESS,
				AH_SERVICEREGISTRY_PORT,
				AH_SERVICEREGISTRY_REGISTER_PATH,
				AH_SERVICEREGISTRY_REQUEST_BODY,
				&statusCode,
				&respBody);
		IotLogInfo("Sent request with response %d", statusCode);
	}
	IotLogInfo("Contact with ServiceRegistry established");
	return xReqSuccess;
}

BaseType_t prvAhOrchestrate( void )
{
	BaseType_t xReqSuccess = pdFAIL;

	int statusCode;
	char respBody[2048] = { 0 };
	while (xReqSuccess == pdFAIL)
	{
		xReqSuccess = prvSendRequest(
				"POST",
				AH_ORCHESTRATOR_ADDRESS,
				AH_ORCHESTRATOR_PORT,
				AH_ORCHESTRATOR_ORCHESTRATE_PATH,
				AH_ORCHESTRATE_REQUEST_BODY,
				&statusCode,
				&respBody);
	}

	xReqSuccess = prvParseOrchResponse(&respBody);

	return xReqSuccess;
}

BaseType_t prvParseOrchResponse(const char* responseBody)
{
	cJSON* pBody = cJSON_Parse(responseBody);
	if (pBody == NULL) {
		const char* pJsonError = cJSON_GetErrorPtr();
		if (pJsonError != NULL)
		{
			IotLogInfo("Error whilst parsing before %s\n", pJsonError);
		}
		cJSON_Delete(pBody);
		return pdFAIL;
	}

	cJSON* pResponses = cJSON_DetachItemFromObjectCaseSensitive(pBody, "response");
	if ( pResponses == NULL ) {
		cJSON_Delete(pBody);
		return pdFAIL;
	}

	cJSON* pResponse = NULL;
	cJSON_ArrayForEach( pResponse, pResponses )
	{
		cJSON* pProvider = cJSON_GetObjectItemCaseSensitive(pResponse, "provider");
		cJSON* pServiceUri = cJSON_GetObjectItemCaseSensitive(pResponse, "serviceUri");
		if ( pProvider == NULL )
		{
			cJSON_Delete(pBody);
			return pdFAIL;
		}

		cJSON* pAddress = cJSON_GetObjectItemCaseSensitive(pProvider, "address");
		cJSON* pPort = cJSON_GetObjectItemCaseSensitive(pProvider, "port");
		if ( !cJSON_IsNumber(pPort) || !cJSON_IsString(pAddress) || !cJSON_IsString(pServiceUri) )
		{
			cJSON_Delete(pBody);
			return pdFAIL;
		}

		ehInfo.pHost = pAddress->valuestring;
		ehInfo.hostSize = strlen(pAddress->valuestring);
		ehInfo.port = pPort->valuedouble;
		ehInfo.pPath = pServiceUri->valuestring;
		ehInfo.pathSize = strlen(pServiceUri->valuestring);

		return pdPASS;
	}

	return pdFAIL;
}

BaseType_t prvAhPublish(const char* publishPayload)
{
	BaseType_t xReqSuccess = pdFAIL;
	char requestBodyBuffer[2048] = { 0 };
	char respBody[2048] = { 0 };

	xReqSuccess = prvBuildEhPublishRequest(publishPayload, &requestBodyBuffer);

	int statusCode;
	do
	{
		xReqSuccess = prvSendRequest(
				"POST",
				ehInfo.pHost,
				ehInfo.port,
				ehInfo.pPath,
				&requestBodyBuffer,
				&statusCode,
				&respBody);
	} while(xReqSuccess == pdFAIL);

	return xReqSuccess;

}

BaseType_t prvBuildEhPublishRequest(const char* publishPayload, char* requestBody)
{
	cJSON* pRequestBody = cJSON_CreateObject();
	if (pRequestBody == NULL)
	{
		return pdFAIL;
	}

	cJSON* pPayload = cJSON_CreateObject();
	if (pPayload == NULL)
	{
		return pdFAIL;
	}
	cJSON* pData = cJSON_CreateString(publishPayload);
	if (pData == NULL)
	{
		return pdFAIL;
	}

	cJSON* pSignature = cJSON_CreateString("");
	if (pSignature == NULL)
	{
		return pdFAIL;
	}

	cJSON_AddItemToObject(pPayload, "data", pData);
	cJSON_AddItemToObject(pPayload, "signature", pSignature);

	cJSON* pSource = cJSON_Parse(AH_EVENTHANDLER_EVENT_SOURCE);
	if (pSource == NULL) {
		const char* pJsonError = cJSON_GetErrorPtr();
		if (pJsonError != NULL)
		{
			IotLogInfo("Error whilst parsing before %s\n", pJsonError);
		}
		cJSON_Delete(pRequestBody);
		cJSON_Delete(pSource);
		return pdFAIL;
	}

	cJSON* pMetadata = cJSON_Parse(AH_EVENTHANDLER_EVENT_METADATA);
	if (pMetadata == NULL) {
		const char* pJsonError = cJSON_GetErrorPtr();
		if (pJsonError != NULL)
		{
			IotLogInfo("Error whilst parsing before %s\n", pJsonError);
		}
		cJSON_Delete(pRequestBody);
		cJSON_Delete(pSource);
		cJSON_Delete(pMetadata);
		return pdFAIL;
	}

	cJSON* pEventType = cJSON_CreateString(AH_EVENTHANDLER_EVENT_TYPE);
	if (pEventType == NULL)
	{
		return pdFAIL;
	}

	cJSON* pTimeStamp = cJSON_CreateString(AH_EVENTHANDLER_EVENT_TIMESTAMP);
	if (pTimeStamp == NULL)
	{
		return pdFAIL;
	}

	cJSON_AddItemToObject(pRequestBody, "eventType", pEventType);
	cJSON_AddItemToObject(pRequestBody, "metaData", pMetadata);
	cJSON_AddItemToObject(pRequestBody, "payload", pPayload);
	cJSON_AddItemToObject(pRequestBody, "source", pSource);
	cJSON_AddItemToObject(pRequestBody, "timeStamp", pTimeStamp);

	requestBody = cJSON_Print(pRequestBody);
	if (requestBody == NULL)
	{
		return pdFAIL;
	}

	cJSON_Delete(pRequestBody);
	cJSON_Delete(pSource);
	cJSON_Delete(pMetadata);

	return pdPASS;

}

/* Check that the root CA certificate is defined. */
#ifndef democonfigROOT_CA_PEM
    #define democonfigROOT_CA_PEM    AH_HTTPS_ROOT_CA
#endif

/* Check that a size for the user buffer is defined. */
#ifndef democonfigUSER_BUFFER_LENGTH
    #define democonfigUSER_BUFFER_LENGTH    ( 2048 )
#endif

/**
 * @brief Time in ticks to wait between each cycle of the demo implemented
 * by RunCoreHttpMutualAuthDemo().
 */
#define httpexampleDELAY_BETWEEN_DEMO_ITERATIONS_TICKS    ( pdMS_TO_TICKS( 5000U ) )

#define FREERTOS_TCP

#include "iot_demo_https_s3_upload_sync.h"
#include "core_http_client.h"
#include "transport_interface.h"
#include "transport_secure_sockets.h"
#include <string.h>
#include "float_to_string.h"

struct NetworkContext
{
    SecureSocketsTransportParams_t * pParams;
};

static BaseType_t prvConnectToServer( NetworkContext_t * pxNetworkContext,const char* host, int port );
static BaseType_t prvSendHttpRequest(
		const TransportInterface_t * pxTransportInterface,
		const char * pcMethod,
		size_t xMethodLen,
		const char * pcPath,
		size_t xPathLen,
		const char* pHost,
		const char* pBody,
		int* respCode,
		const char* respBody );

static uint8_t ucUserBuffer[ democonfigUSER_BUFFER_LENGTH ];

BaseType_t prvSendRequest(const char* method, const char* host, int port, const char* path, const char* reqBody, int* respCode, char* respBody)
{
	/* The transport layer interface used by the HTTP Client library. */
	TransportInterface_t xTransportInterface;
	/* The network context for the transport layer interface. */
	NetworkContext_t xNetworkContext = { 0 };
	TransportSocketStatus_t xNetworkStatus;
	BaseType_t xIsConnectionEstablished = pdFALSE;

	SecureSocketsTransportParams_t socketTransportParams = { 0 };

	xNetworkContext.pParams = &socketTransportParams;
	//SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };


    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;
	/**************************** Connect. ******************************/

	/* Attempt to connect to the HTTP server. If connection fails, retry
	* after a timeout. The timeout value will be exponentially increased
	* until either the maximum number of attempts or the maximum timeout
	* value is reached. The function returns pdFAIL if the TCP connection
	* cannot be established with the broker after the configured number of
	* attempts. */
	xDemoStatus = prvConnectToServer( &xNetworkContext , host, port);

	if( xDemoStatus == pdPASS )
	{
	   /* Set a flag indicating that a TLS connection exists. */
	   xIsConnectionEstablished = pdTRUE;

	   /* Define the transport interface. */
	   xTransportInterface.pNetworkContext = &xNetworkContext;
	   xTransportInterface.send = SecureSocketsTransport_Send;
	   xTransportInterface.recv = SecureSocketsTransport_Recv;
	}
	else
	{
	   /* Log error to indicate connection failure after all
		* reconnect attempts are over. */
	   IotLogError( ( "Failed to connect to HTTP server %.*s.",
				   ( int32_t ) strlen(host),
				   host ) );
	}

	// Convert int port to string
	char* pPortStr = malloc(5);
	pPortStr[4] = '\0';
	int portCopy = port;
	for(int i = 3; i >= 0; i--) {
		pPortStr[i] = portCopy % 10 + '0';
		portCopy = portCopy / 10;
	}

	//Concat host and port, example: "192.168.0.2:8443"
	char* pPathUri ;
	if((pPathUri = malloc(strlen(host)+strlen(pPortStr)+2)) != NULL){
	    pPathUri[0] = '\0';   // ensures the memory is an empty string
	    strcat(pPathUri,host);
	    strcat(pPathUri,":");
	    strcat(pPathUri,pPortStr);
	} else {
	    // exit?
	}

	/*********************** Send HTTP request.************************/
	if( xDemoStatus == pdPASS )
	{
	   xDemoStatus = prvSendHttpRequest( &xTransportInterface,
										 method,
										 strlen(method),
										 path,
										 strlen(path),
										 pPathUri,
										 reqBody,
										 respCode,
										 respBody);
	}

	/**************************** Disconnect. ******************************/
	/* Close the network connection to clean up any system resources that the
	* demo may have consumed. */
	if( xIsConnectionEstablished == pdTRUE )
	{
	   /* Close the network connection.  */
	   xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );

	   //Free memory
	   free(pPathUri);
	   free(pPortStr);

	   if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
	   {
		   xDemoStatus = pdFAIL;
		   IotLogError( ( "SecureSocketsTransport_Disconnect() failed to close the network connection. "
					   "StatusCode=%d.", ( int ) xNetworkStatus ) );
	   }
	}

	return xDemoStatus;
}

static BaseType_t prvConnectToServer( NetworkContext_t * pxNetworkContext,const char* host, int port )
{
    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_CONNECT_FAILURE;

    int hostSize = strlen(host);
    /* Initializer server information. */
    xServerInfo.pHostName = host;
    xServerInfo.hostNameLength = hostSize;
    xServerInfo.port = port;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = false;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = true;
    xSocketsConfig.pRootCa = democonfigROOT_CA_PEM;
    xSocketsConfig.rootCaSize = sizeof( democonfigROOT_CA_PEM );
    xSocketsConfig.sendTimeoutMs = 200;
    xSocketsConfig.recvTimeoutMs = 200;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
    IotLogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int32_t ) host,
               hostSize,
               port ) );

    /* Attempt to create a mutually authenticated TLS connection. */
    //xNetworkStatus = TLS_FreeRTOS_Connect(pxNetworkContext, democonfigAWS_IOT_ENDPOINT, democonfigAWS_HTTP_PORT, &xNetCreds, 1000, 1000);
    xNetworkStatus = SecureSocketsTransport_Connect(pxNetworkContext, &xServerInfo, &xSocketsConfig);

    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

static BaseType_t prvSendHttpRequest( const TransportInterface_t * pxTransportInterface,
                                      const char * pcMethod,
                                      size_t xMethodLen,
                                      const char * pcPath,
                                      size_t xPathLen,
									  const char* pHost,
									  const char* pBody,
									  int* respCode,
									  const char* respBody)
{
    /* Return value of this method. */
    BaseType_t xStatus = pdPASS;

    /* Configurations of the initial request headers that are passed to
     * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* Return value of all methods from the HTTP Client library API. */
    HTTPStatus_t xHTTPStatus = HTTPSuccess;

    configASSERT( pcMethod != NULL );
    configASSERT( pcPath != NULL );

    /* Initialize all HTTP Client library API structs to 0. */
    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
    ( void ) memset( &xResponse, 0, sizeof( xResponse ) );
    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );

    /* Initialize the request object. */

    xRequestInfo.pHost = pHost;
    xRequestInfo.hostLen = strlen(pHost);
    xRequestInfo.pMethod = pcMethod;
    xRequestInfo.methodLen = strlen(pcMethod);
    xRequestInfo.pPath = pcPath;
    xRequestInfo.pathLen = strlen(pcPath);

    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
     * can be sent over the same established TCP connection. */
    xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    /* Set the buffer used for storing request headers. */
    xRequestHeaders.pBuffer = ucUserBuffer;
    xRequestHeaders.bufferLen = democonfigUSER_BUFFER_LENGTH;

    xHTTPStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                       &xRequestInfo );

    if (xHTTPStatus == HTTPSuccess )
    {
    	// Add content type to header
    	const char* pKey = "Content-Type";
    	const char* pValue = "application/json";
    	xHTTPStatus = HTTPClient_AddHeader(&xRequestHeaders, pKey, strlen(pKey), pValue, strlen(pValue));
    }

    if( xHTTPStatus == HTTPSuccess )
    {
        /* Initialize the response object. The same buffer used for storing
         * request headers is reused here. */
        xResponse.pBuffer = ucUserBuffer;
        xResponse.bufferLen = democonfigUSER_BUFFER_LENGTH;

        IotLogInfo( ( "Sending HTTP %.*s request to %.*s%.*s...",
                   ( int32_t ) xRequestInfo.methodLen, xRequestInfo.pMethod,
                   ( int32_t ) strlen(pHost), pHost,
                   ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath ) );
        IotLogInfo( ( "Request Headers:\n%.*s\n"
                    "Request Body:\n%.*s\n",
                    ( int32_t ) xRequestHeaders.headersLen,
                    ( char * ) xRequestHeaders.pBuffer,
                    ( int32_t ) strlen(pBody), pBody ) );

        /* Send the request and receive the response. */
        xHTTPStatus = HTTPClient_Send( pxTransportInterface,
                                       &xRequestHeaders,
                                       ( uint8_t * ) pBody,
                                       strlen(pBody),
                                       &xResponse,
                                       0 );
    }
    else
    {
        IotLogError( ( "Failed to initialize HTTP request headers: Error=%s.",
                    HTTPClient_strerror( xHTTPStatus ) ) );
    }

    if( xHTTPStatus == HTTPSuccess )
    {
        IotLogInfo( ( "Received HTTP response from %.*s%.*s...\n",
                   ( int32_t ) strlen(pHost), pHost,
                   ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath ) );
        IotLogDebug( ( "Response Headers:\n%.*s\n",
                    ( int32_t ) xResponse.headersLen, xResponse.pHeaders ) );
        IotLogDebug( ( "Status Code:\n%u\n",
                    xResponse.statusCode ) );
        IotLogDebug( ( "Response Body:\n%.*s\n",
                    ( int32_t ) xResponse.bodyLen, xResponse.pBody ) );
        memcpy(respBody, xResponse.pBody, xResponse.bodyLen);
        *respCode = xResponse.statusCode;
    }
    else
    {
        IotLogError( ( "Failed to send HTTP %.*s request to %.*s%.*s: Error=%s.",
                    ( int32_t ) xRequestInfo.methodLen, xRequestInfo.pMethod,
                    ( int32_t ) strlen(pHost), pHost,
                    ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath,
                    HTTPClient_strerror( xHTTPStatus ) ) );
    }

    if( xHTTPStatus != HTTPSuccess )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

