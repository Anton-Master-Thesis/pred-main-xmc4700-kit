#include "iot_https_client.h"
#include "iot_https_utils.h"
#include "aws_demo_config.h"
#include "platform/iot_network.h"
#include "private/iot_error.h"
#include "iot_demo_https_common.h"
#include "platform/iot_clock.h"

/* Declaration of demo function. */
int RunHttpsSyncUploadDemo( bool awsIotMqttMode,
                            const char * pIdentifier,
                            void * pNetworkServerInfo,
                            void * pNetworkCredentialInfo,
                            const IotNetworkInterface_t * pNetworkInterface );