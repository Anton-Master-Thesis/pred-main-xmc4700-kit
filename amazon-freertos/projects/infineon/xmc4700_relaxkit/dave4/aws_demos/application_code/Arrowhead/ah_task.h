/*
 * ah_task.h
 *
 *  Created on: 28 Apr 2022
 *      Author: Johansson
 */

#ifndef APPLICATION_CODE_ARROWHEAD_AH_TASK_H_
#define APPLICATION_CODE_ARROWHEAD_AH_TASK_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "app_error.h"
#include "app_types.h"

#include "statistic.h"
#include "base64.h"
#include "iot_network_manager_private.h"

/* Defining message format */
#define AH_OUTPUT_FORMAT_JSON                         1

/** Timeout for the TLS negotiation */
//#define mqtttaskMQTT_ECHO_TLS_NEGOTIATION_TIMEOUT       pdMS_TO_TICKS( 15000 )
/** Timeout for MQTT operations */
//#define mqtttaskMQTT_TIMEOUT                            pdMS_TO_TICKS( 3000 )
/** The number of items in the receive queue */
#define ahtaskSENSOR_RECEIVE_QUEUE_LENGTH                    ( 2 )
/** Size of the buffer in which messages to the broker will be generated */
#define ahtaskSEND_BUFFER_SIZE                        ( 4096 )
/** Stack allocated for the task */
#define ahtaskSTACK_SIZE                              ( 4096 )
/** Priority of the task */
#define ahtaskPRIORITY                                ( tskIDLE_PRIORITY + 4 )

/** Stack allocated for the task */
#define robusttaskPRIORITY                              ( tskIDLE_PRIORITY + 3 )
/** Priority of the task */
#define ROBUST_TASK_STACK_SIZE                   	    ( 1024 )
/** Robust Task ping period */
#if NBIOT_ENABLED
#define ROBUST_DELAY                              	    ( 5000 )
#else
#define ROBUST_DELAY                              	    ( 3000 )
#endif
/** ping timeout */
#define PING_TIMEOUT                            	    ( 100 )
/** ping retries */
#define PING_COUNT                            	 	    ( 3 )

/** Network connection reestablishing retries period */
#define RECONNECT_DELAY							 	    ( 10 )

/* Number of ping attempt */
#define ATTEMPTS_COUNT									( 3 )


typedef enum {
	eConnEstablished2 = 0,	/* Connection established. */
	eNetworkError2,			/* Network error. */
	eSRError2,				/* Service Registry error. */
	eORError2,				/* Orchestrator error */
	eEHError2				/* Event Handler error. */

} eConnectionState_t2;


typedef enum {
	ePingSuccess2 = 0,	/* Ping Success. */
	ePingFail2			/* Ping Fail. */

} ePingStatus_t2;

/* Queue for messages between sensors and Arrowhead tasks */
extern QueueHandle_t xSensorAhMessageQueueHandle;


/** @brief Starts the Arrowhead task */
void vAhTaskStart( void );
/** @brief Deletes the Arrowhead task */
void vAhTaskDelete( void );

/* Initialize network, manager and libraries */
uint8_t ucAhTaskNetworkInitialize( void );

BaseType_t prvAhRegister( void );
BaseType_t prvAhOrchestrate( void );

BaseType_t prvSendRequest(
		const char* method,
		const char* host,
		int port,
		const char* path,
		const char* reqBody,
		int* respCode,
		char* respBody);

#endif /* APPLICATION_CODE_ARROWHEAD_AH_TASK_H_ */
