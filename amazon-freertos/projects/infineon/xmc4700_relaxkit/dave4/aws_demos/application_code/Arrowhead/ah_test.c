//BaseType_t prvAhRegister( void )
//{
//#include "iot_demo_https_s3_upload_sync.h"
//#include "core_http_client.h"
//
//	/* The transport layer interface used by the HTTP Client library. */
//	TransportInterface_t xTransportInterface;
//	/* The network context for the transport layer interface. */
//	NetworkContext_t xNetworkContext = { 0 };
//	TransportSocketStatus_t xNetworkStatus;
//	BaseType_t xIsConnectionEstablished = pdFALSE;
//	UBaseType_t uxDemoRunCount = 0UL;
//	SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
//
//
//
//	IotHttpsReturnCode_t xhttpsClientStatus;
//
//	IotNetworkInterface_t* pNetwork;
//	pNetwork = AwsIotNetworkManager_GetNetworkInterface(configENABLED_NETWORKS);
//
//	IotNetworkCredentials_t cred = { 0 };
//	cred.pClientCert = AH_HTTPS_CLIENT_CA;
//	cred.clientCertSize = sizeof(AH_HTTPS_ROOT_CA);
//	cred.pRootCa = AH_HTTPS_ROOT_CA;
//	cred.rootCaSize = sizeof(AH_HTTPS_ROOT_CA);
//	cred.pPrivateKey = AH_HTTPS_PRIVATE_KEY;
//	cred.privateKeySize = sizeof(AH_HTTPS_PRIVATE_KEY);
//
//	RunHttpsSyncUploadDemo(false, NULL, NULL, &cred, pNetwork);
//
//
//	//const char * pPath = AH_SERVICEREGISTRY_REGISTER_PATH;
//	//size_t pathLen = sizeof(AH_SERVICEREGISTRY_REGISTER_PATH);
//	const char* pPath = "/serviceregistry/echo";
//	size_t pathLen = sizeof("/serviceregistry/echo");
//
//	const char * pAddress = AH_SERVICEREGISTRY_ADDRESS;
//	size_t addressLen = sizeof(AH_SERVICEREGISTRY_ADDRESS);
//
//	uint32_t uiconnAttempt = 0;
//
//	IotHttpsConnectionInfo_t connInfo = IOT_HTTPS_CONNECTION_INFO_INITIALIZER;
//	IotHttpsConnectionHandle_t connHandle = IOT_HTTPS_CONNECTION_HANDLE_INITIALIZER;
//
//	uint8_t* pConnUserBuffer = (uint8_t*)malloc(connectionUserBufferMinimumSize);
//
//	xSrConnConfig.pAddress = pAddress;
//	xSrConnConfig.addressLen = addressLen;
//	xSrConnConfig.port = AH_SERVICEREGISTRY_PORT;
//	xSrConnConfig.flags |= IOT_HTTPS_IS_NON_TLS_FLAG | IOT_HTTPS_DISABLE_SNI;
//	xSrConnConfig.pCaCert = AH_HTTPS_ROOT_CA;
//	xSrConnConfig.caCertLen = sizeof(AH_HTTPS_ROOT_CA);
//	xSrConnConfig.userBuffer.pBuffer = pConnUserBuffer;
//	xSrConnConfig.userBuffer.bufferLen = connectionUserBufferMinimumSize;
//	xSrConnConfig.pClientCert = AH_HTTPS_CLIENT_CA;
//	xSrConnConfig.clientCertLen = sizeof(AH_HTTPS_CLIENT_CA);
//	xSrConnConfig.pPrivateKey = AH_HTTPS_PRIVATE_KEY;
//	xSrConnConfig.privateKeyLen = sizeof(AH_HTTPS_PRIVATE_KEY);
//
//	xSrConnConfig.pNetworkInterface = pNetwork;
//
//
//
//	IotHttpsSyncInfo_t reqSyncInfo = IOT_HTTPS_SYNC_INFO_INITIALIZER;
//
//	uint32_t bodyBufferLen = 0;
//	uint8_t* pBodyBuffer = (uint8_t*)malloc(bodyBufferLen);
//
//
//	reqSyncInfo.pBody = pBodyBuffer;
//	reqSyncInfo.bodyLen = bodyBufferLen;
//
//	IotHttpsRequestInfo_t reqConfig;
//
//	reqConfig.pPath = pPath;
//	reqConfig.pathLen = pathLen;
//	reqConfig.method = IOT_HTTPS_METHOD_GET;
//	reqConfig.pHost = AH_SERVICEREGISTRY_ADDRESS;
//	reqConfig.hostLen = sizeof(AH_SERVICEREGISTRY_ADDRESS);
//	reqConfig.isNonPersistent = true;
//	reqConfig.userBuffer.pBuffer = pRequestUserBuffer;
//	reqConfig.userBuffer.bufferLen = userBufferSize;
//	reqConfig.isAsync = false;
//	reqConfig.u.pSyncInfo = &reqSyncInfo;
//
//	xhttpsClientStatus = IotHttpsClient_Init();
//
//	if( xhttpsClientStatus != IOT_HTTPS_OK )
//	{
//        IotLogError( "An error occurred initializing the HTTPS library. Error code: %d", xhttpsClientStatus );
//        return pdFAIL;
//	}
//
//	IotHttpsRequestHandle_t reqHandle;
//
//	xhttpsClientStatus = IotHttpsClient_InitializeRequest( &reqHandle, &reqConfig );
//
//	if( xhttpsClientStatus != IOT_HTTPS_OK )
//	{
//		IotLogError( "An error occured when initializing the request to SR" );
//		configPRINTF(( "An error occured when initializing the request to SR\n" ));
//		return pdFAIL;
//	}
//
//	xhttpsClientStatus = IotHttpsClient_Connect( &connHandle, &xSrConnConfig );
//
//	if( xhttpsClientStatus != IOT_HTTPS_OK )
//	{
//		IotLogError( "An error occured when establishing a connection to SR" );
//		configPRINTF( ("An error occured when establishing a connection to SR\r\n") );
//		return pdFAIL;
//	}
//
//	IotHttpsResponseHandle_t respHandle;
//
//	uint32_t bodyRespBufferLen = 256;
//	uint8_t* pRespBodyBuffer = (uint8_t*)malloc(bodyRespBufferLen);
//
//	uint32_t headerRespBufferLen = 256;
//	uint8_t* pRespHeaderBuffer = (uint8_t*)malloc(headerRespBufferLen);
//
//	IotHttpsSyncInfo_t respSyncInfo;
//
//	respSyncInfo.pBody = pRespBodyBuffer;
//	respSyncInfo.bodyLen = bodyRespBufferLen;
//
//	IotHttpsResponseInfo_t respInfo = { 0 };
//	respInfo.pSyncInfo = &respSyncInfo;
//	respInfo.userBuffer.pBuffer = pRespHeaderBuffer;
//	respInfo.userBuffer.bufferLen = headerRespBufferLen;
//
//	xhttpsClientStatus = IotHttpsClient_SendSync( connHandle, reqHandle, &respHandle, &respInfo, 0 );
//
//	if( xhttpsClientStatus != IOT_HTTPS_OK )
//	{
//		IotLogError( "An error occured during the request to SR" );
//		configPRINTF(( "An error occured during the request to SR\n" ));
//		return pdFAIL;
//	}
//
//	uint16_t httpStatusCode;
//
//	xhttpsClientStatus = IotHttpsClient_ReadResponseStatus(respHandle, &httpStatusCode );
//
//	if( xhttpsClientStatus != IOT_HTTPS_OK )
//	{
//		IotLogError( "An error occured when reading the status code" );
//		configPRINTF(( "An error occured when reading the status code\n" ));
//		return pdFAIL;
//	}
//
//	if(httpStatusCode != IOT_HTTPS_STATUS_OK )
//	{
//		IotLogError( "Something went wrong with the request" );
//		configPRINTF(( "Something went wrong with the request\n" ));
//		return pdFAIL;
//	}
//	return pdPASS;
//}

//BaseType_t prvAhRegister( void )
//{
//
//
//	/* The transport layer interface used by the HTTP Client library. */
//	TransportInterface_t xTransportInterface;
//	/* The network context for the transport layer interface. */
//	NetworkContext_t xNetworkContext = { 0 };
//	TransportSocketStatus_t xNetworkStatus;
//	BaseType_t xIsConnectionEstablished = pdFALSE;
//	UBaseType_t uxDemoRunCount = 0UL;
//
//	TlsTransportParams_t socketTransportParams = { 0 };
//
//	xNetworkContext.pParams = &socketTransportParams;
//	//SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
//
//
//    /* Upon return, pdPASS will indicate a successful demo execution.
//    * pdFAIL will indicate some failures occurred during execution. The
//    * user of this demo must check the logs for any failure codes. */
//    BaseType_t xDemoStatus = pdPASS;
//
//   do
//   {
//	   /**************************** Connect. ******************************/
//
//	   /* Attempt to connect to the HTTP server. If connection fails, retry
//		* after a timeout. The timeout value will be exponentially increased
//		* until either the maximum number of attempts or the maximum timeout
//		* value is reached. The function returns pdFAIL if the TCP connection
//		* cannot be established with the broker after the configured number of
//		* attempts. */
//	   xDemoStatus = prvConnectToServer( &xNetworkContext );
//
//	   if( xDemoStatus == pdPASS )
//	   {
//		   /* Set a flag indicating that a TLS connection exists. */
//		   xIsConnectionEstablished = pdTRUE;
//
//		   /* Define the transport interface. */
//		   xTransportInterface.pNetworkContext = &xNetworkContext;
//		   xTransportInterface.send = SecureSocketsTransport_Send;
//		   xTransportInterface.recv = SecureSocketsTransport_Recv;
//	   }
//	   else
//	   {
//		   /* Log error to indicate connection failure after all
//			* reconnect attempts are over. */
//		   IotLogError( ( "Failed to connect to HTTP server %.*s.",
//					   ( int32_t ) httpexampleAWS_IOT_ENDPOINT_LENGTH,
//					   democonfigAWS_IOT_ENDPOINT ) );
//	   }
//
//	   /*********************** Send HTTP request.************************/
//
//	   if( xDemoStatus == pdPASS )
//	   {
//		   xDemoStatus = prvSendHttpRequest( &xTransportInterface,
//											 HTTP_METHOD_GET,
//											 httpexampleHTTP_METHOD_POST_LENGTH,
//											 democonfigPOST_PATH,
//											 httpexamplePOST_PATH_LENGTH );
//	   }
//
//	   /**************************** Disconnect. ******************************/
//
//	   /* Close the network connection to clean up any system resources that the
//		* demo may have consumed. */
//	   if( xIsConnectionEstablished == pdTRUE )
//	   {
//		   /* Close the network connection.  */
//		   xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );
//
//		   if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
//		   {
//			   xDemoStatus = pdFAIL;
//			   IotLogError( ( "SecureSocketsTransport_Disconnect() failed to close the network connection. "
//						   "StatusCode=%d.", ( int ) xNetworkStatus ) );
//		   }
//	   }
//
//	   /* Increment the demo run count. */
//	   uxDemoRunCount++;
//
//	   if( xDemoStatus == pdPASS )
//	   {
//		   IotLogInfo( ( "Demo iteration %lu was successful.", uxDemoRunCount ) );
//	   }
//	   /* Attempt to retry a failed demo iteration for up to #httpexampleMAX_DEMO_COUNT times. */
//	   else if( uxDemoRunCount < httpexampleMAX_DEMO_COUNT )
//	   {
//		   IotLogWarn( ( "Demo iteration %lu failed. Retrying...", uxDemoRunCount ) );
//		   vTaskDelay( httpexampleDELAY_BETWEEN_DEMO_ITERATIONS_TICKS );
//	   }
//	   /* Failed all #httpexampleMAX_DEMO_COUNT demo iterations. */
//	   else
//	   {
//		   IotLogError( ( "All %d demo iterations failed.", httpexampleMAX_DEMO_COUNT ) );
//		   break;
//	   }
//   } while( xDemoStatus != pdPASS );
//
//   if( xDemoStatus == pdPASS )
//   {
//	   IotLogInfo( ( "Demo completed successfully." ) );
//   }
//
//
//
//	return pdPASS;
//}
