/*
 * ah_config.h
 *
 *  Created on: 28 Apr 2022
 *      Author: Johansson
 */

#ifndef AH_CONFIG_H_
#define AH_CONFIG_H_

#include "iot_config.h"

#define AH_HTTPS_ROOT_CA 												\
"-----BEGIN CERTIFICATE-----\n"											\
"MIIC3jCCAcagAwIBAgIEXNU+rTANBgkqhkiG9w0BAQsFADAXMRUwEwYDVQQDDAxh\n"	\
"cnJvd2hlYWQuZXUwHhcNMTkwNTEwMDkwNDQ1WhcNMjkwNTEwMDkwNDQ1WjAXMRUw\n"	\
"EwYDVQQDDAxhcnJvd2hlYWQuZXUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"	\
"AoIBAQCuB4z+wchXDKdfy9YFZha2U0khBAWuHYerBlBLM0Oqvr4c/YYcZpNwTWY2\n"	\
"tk6UXPUTQ8gI9V6Ob7DRXoAfLDhCRGKySN0BlGnjUJkItP25Sj0RfiTL3b8fFEIT\n"	\
"Z8pg+6pAfeFQgV0yz+ziyL+0uu69VZPv+RAEf1GKgshGGLJw3sOlcIdKuZaEAaA2\n"	\
"b0nDUn229VpKXb9cg47Ae1Yb0sJcTkyIBuhkQKln3uhG2xct9nDfVal05+229AJQ\n"	\
"Ly1f0UfEofvD/OLjFG3umF857T1Vr5azj8zFOvNi503gV458lH3wKC+9UHUf46sg\n"	\
"Hc8Tyrz1q8VAIamJe7BLUeRFwGvBAgMBAAGjMjAwMA8GA1UdEwQIMAYBAf8CAQMw\n"	\
"HQYDVR0OBBYEFJqKmR4xTB6y5i22mu3HHjHK3Tv+MA0GCSqGSIb3DQEBCwUAA4IB\n"	\
"AQCkFsqyeAjztDBkTQrPxAB0Vvx6KPINApHGIHkJj/9crKXZEQcNJcJr35hfLcgv\n"	\
"hSsmLMdeRFCeaG5QLmUKI6GFYIbX+6nawMLGzIPUTOGetNeuMauDXkq09Hu/UmjN\n"	\
"AOgoD5vWdtyTbItv21enJnUelClAJ7VXti2QpyRM2puPHpZMNi4FWgLGPo6hq5ka\n"	\
"d7KomzW8JLh2Vd67v/6mXGpST4EzyRe+Yb2FJZUmhxVWt68/MFaflPQ2toPIsIpW\n"	\
"5m4OS+rT7t+uxPKWU/ogCK7BOUfE/qf3Al/osWkNKnFQDtO/7x7InEmRoP9EUv07\n"	\
"JY4vK/+k5fJUZHpJzpuKxbBo\n"		\
"-----END CERTIFICATE-----"

#define AH_HTTPS_CLIENT_CA												\
"-----BEGIN CERTIFICATE-----\n" 										\
"MIIDMzCCAhugAwIBAgIEXNliGTANBgkqhkiG9w0BAQsFADAXMRUwEwYDVQQDDAxh\n"	\
"cnJvd2hlYWQuZXUwHhcNMTkwNTEzMTIyNDU3WhcNMjkwNTEzMTIyNDU3WjAoMSYw\n"	\
"JAYDVQQDDB10ZXN0Y2xvdWQyLmFpdGlhLmFycm93aGVhZC5ldTCCASIwDQYJKoZI\n"	\
"hvcNAQEBBQADggEPADCCAQoCggEBAJV8P9a3Qjl3jcm0kLwKMqwKux6CHVyojfRO\n"	\
"Zu0A6NLp0BZjcyzsH66Xj22Fub6gcUnMx7sg3DsYANXyTYZTaRCn/058BaLpD8we\n"	\
"2Q0+XT2xEKsh8PtMkuj3Ebge93W6E5a/fvl+Xx3Ggi8SCC9OscG3hzN8cpXt7vPg\n"	\
"AU6k/TgsAAnAnCbVqlrevMSAy6Opq5g2RM795J/mvom9seGmL0xOilXTlGK9gZBB\n"	\
"HQfcY7sQc0/SJOUwNVQc9xNgRyf85EVgLFq2kkW4Tjyc8JNa6PgjUTF67NJQasXV\n"	\
"BY+EydvqwZsdeKnaHrjRdIQxQ8uZLZXmnVSPtQhX3cG/+untN88CAwEAAaN2MHQw\n"	\
"DwYDVR0TBAgwBgEB/wIBAjBCBgNVHSMEOzA5gBSaipkeMUwesuYttprtxx4xyt07\n"	\
"/qEbpBkwFzEVMBMGA1UEAwwMYXJyb3doZWFkLmV1ggRc1T6tMB0GA1UdDgQWBBQE\n"	\
"ME0II5M93mY9Flqn0cleI6aXTzANBgkqhkiG9w0BAQsFAAOCAQEAP4kKGckmZk/t\n"	\
"+OrZ8D1P0hhBbAyNWlDijVexoP8JGxo1blEbftfEJ3FdYL+CKFJ1wa7l+jhvAiW9\n"	\
"EiIk6Z/DvH5b9NMxyc72kTy4P0yYmXuJ2/x/rA2hdBwFuSx4KYjFOrMaSGwuCXae\n"	\
"ec61odXhtxC9Y7HGoIao+pNjD1qa82sgzRA6ZQSMyTGI1NaVfBco6VMb1Hu9U6i0\n"	\
"HdbUtu9O3VmkgAI/U7nF/qDcuBHaHXEco4ApTh+PWo6BLmqaBOdcpgRD0F7vc5y/\n"	\
"JviipqjU4oHiX7CQShdGUC4lkPZmXFlfsoXACIpcPDF/49BX1OAy4GNCl1PeyU7A\n"	\
"6STgROIXpw==\n"														\
"-----END CERTIFICATE-----"

#define AH_HTTPS_PUBLIC_KEY												\
"-----BEGIN PUBLIC KEY-----\n"											\
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvv5P08iimDY8v4jkMsuV\n"	\
"MAjvLoFjA2Vmgn0vrEbs0oYhTqAQUXczBIM300wyfZWCCa2JtyL5AZeDSIr8ktt4\n"	\
"fYtMhVYXKcspfuRDVccNo+rKtk+fw9teLS/SjlKwDWIM00N1xcLn8H6nHCaCWkb5\n"	\
"/+dFQUE2Ge38Iq7a4ebkOUtQVht+rEsfcxbyKGGa3Xyo/zRZN89Q78YSFqfxGK0l\n"	\
"zqW6c5ggi3d1ngKZ7unO3ZM4aZUX58HP57FyQeRbpm+eNkZK9Ej8deW8EpjIXoo5\n"	\
"LHnJhhXaihe2mhv8Gxxpv9Cfnsiau/q1AiqXxPvyk37hrhhYdTyth6pUAPdbaBhB\n"	\
"3wIDAQAB\n"															\
"-----END PUBLIC KEY-----\n"

#define AH_HTTPS_PRIVATE_KEY											\
"-----BEGIN ENCRYPTED PRIVATE KEY-----\n"								\
"MIIFHDBOBgkqhkiG9w0BBQ0wQTApBgkqhkiG9w0BBQwwHAQIUz3E0Cm6ut8CAggA\n"	\
"MAwGCCqGSIb3DQIJBQAwFAYIKoZIhvcNAwcECCIF1R7pyrkTBIIEyCklo7UGFU9X\n"	\
"UcpMf3NrdxPF1D5+5qrT1JDBs7zAkckhlkSr7hej6WwGOmzbZn7ZicuhW76Ej/ir\n"	\
"W3mw0oyiSfDPNpo6+GUuqq0Up8fVS8x/dXXOX+ojwy8+DF1ZJo6Am7YnKf/o3aAJ\n"	\
"7g2UGduO+nmch6K2XWBbFv/L9CGj5AQ8IbbTs85TpFSk40qK5Whu5vxOYnMNB+t4\n"	\
"Fy7w22S8TEynthW6DSvyHvKswFZJ7CVfDQ8uHMsaRkSbD+W/GmqrUGNqDBGZNvVv\n"	\
"9WdOHO0Xnfw2u1oFt/0vvWxosL6AYTys6ap33Cs1RHgf3VYuH3pCyyM6nk0i6J/+\n"	\
"/pUEqdWdxtad1CfDyqefxKWCbGr5dmxkOfk6MhhQyIOjjSKPYC/dF14mCIx3EdUG\n"	\
"gu1sxen3eTw6AYTi6axqrcgziKXMa4xl2uqVLXvUPu8S3bt4zXB1L+Bku5LaoHST\n"	\
"DBkk9Xib232TPP/ODDxnHXbrYmU79Sb5leQn6o026Pe7Oyo9sH5BpryVbxzhFMat\n"	\
"uuMG/R+JpBKyo3Eryg4ONsGy81vqUbFYyiOs+rhQOkKoMYabBwMI/ZqQAPOuMFuO\n"	\
"z04rshjUGGwCLU18+RlbFpnACr3fQ8p289DVl5ohMq+UnPYBOMQaNZnS7MJegLpO\n"	\
"uVpf3NMBrrQfjanFxx9ta/gN4Xj/AC93LRo+ap4vPfLmpoFQa5YQSSdK7YlN/SiN\n"	\
"PGMrF1ZgwxAoSfXI9IdUIz0xW11Res/sFq5mtme20szCChgWgFK+Anhn4U//YDjo\n"	\
"GBZO6zl+B6B6sBWA0AmZONg1JD9eeio2CyilfXbFIr47qmofLINyxHyuBBTKwcZg\n"	\
"HXm9+5syQUmBnEigM4AznegAABnFfD0aegLSXwmJX/8qpPmiOOB2FUxaYlKlyD80\n"	\
"qcPvsDyEFsbwKE6TmXs9S094JuIR+9TetdhGVeaWGDzCVwvvLXrGopvUGeYatjf7\n"	\
"hP0yjq2wPpiRKz8PVtuNPumN09qnI7FNZ1xBVCRbXGOgg0PWmV6L2c2wiiI+f3Lt\n"	\
"/fC3YVuOqpO29Uzx/7/7QC+0AzuYP25my5OgZPgr1hXZjcsMsafLTnkaYNFChKim\n"	\
"eAT14DZcw6mueMXuIZJhVCZ4MpQjJhD1t/aG82kNMmQF1Z2PHUMnDdPl4kZOM9vr\n"	\
"xGUlixGe7u0+WsqMbVJUMpVAbb2PcvYPTGCyRDd1zGghOzORw6djAaimtpR9Fv8d\n"	\
"gqavmHNB56dgkw3fSJtVnbae4igpOVYmBJYZ6zVAMCSCL2I44UcqQFINXSfkdjyd\n"	\
"xmaitqQgQDa1qshWuxI5Z8AxDo2cY3kuc40qZz9hEebO5EJvnJBr9yrqLchY5WS2\n"	\
"YruUsweidtZeki36m0gplwkmZZrmMaNVe9+pOXLQZMFpjK4aFdHDRmRmndhuEsiv\n"	\
"2TDAKuBFM8KGGlzPuNo0qLvxlNbn5qDHNf+NUbAfBjSXMNikYbg8GODKO2+czwSi\n"	\
"gdqfTu9YHwXXK+nVQdYjxCprKObjF0hCQUZR6Dp1wlZLq/xXqa5icr4+vND+Lj9H\n"	\
"C47zfN9w7WefVkHCBMmvVL6KKmn7bsQfqDBylCX9pMJadz/txngpH6z6QnFYNhtN\n"	\
"9jR0mI948VlyZ2XQinJ26A==\n"											\
"-----END ENCRYPTED PRIVATE KEY-----\n"

#define AH_SERVICEREGISTRY_ADDRESS 			"192.168.2.120"
#define AH_SERVICEREGISTRY_PORT				8443
#define AH_SERVICEREGISTRY_REGISTER_PATH 	"/serviceregistry/register"
#define AH_SERVICEREGISTRY_UNREGISTER_PATH 	"/serviceregistry/unregister?address=127.0.0.1&port=8911&service_definition=xensive&service_uri=eventhandler&system_name=xensivekit"

#define AH_ORCHESTRATOR_ADDRESS				"192.168.2.120"
#define AH_ORCHESTRATOR_PORT				8441
#define AH_ORCHESTRATOR_ORCHESTRATE_PATH	"/orchestrator/orchestration"

#define AH_SERVICEREGISTRY_REQUEST_BODY				\
	"{"												\
		"\"serviceDefinition\": \"xensive\","		\
		"\"serviceUri\": \"eventhandler\", "		\
		"\"secure\": \"NOT_SECURE\", "				\
		"\"interfaces\": [\"HTTP-INSECURE-JSON\"],"	\
		"\"providerSystem\": {"						\
			"\"systemName\": \"xensivekit\", "		\
			"\"address\": \"127.0.0.1\", "			\
			"\"port\": 8911, "						\
			"\"authenticationInfo\": \"\", "		\
			"\"metadata\": {}"						\
		"}"											\
	"}"

#define AH_ORCHESTRATE_REQUEST_BODY															\
	"{"																						\
		"\"requesterSystem\": {"															\
			"\"systemName\": \"xensivekit\","												\
			"\"address\": \"127.0.0.1\", "													\
			"\"port\": 8911, "																\
			"\"authenticationInfo\": \"\", "												\
			"\"metadata\": {}"																\
		"},"																				\
		"\"requestedService\": { "															\
			"\"serviceDefinitionRequirement\": \"event-publish\","							\
			"\"interfaceRequirements\": [\"HTTP-SECURE-JSON\", \"HTTP-INSECURE-JSON\"],"	\
			"\"securityRequirements\": [\"CERTIFICATE\", \"NOT_SECURE\"], "					\
			"\"metadataRequirements\": {\"req1\": \"req2\"}"								\
		"}, "																				\
			"\"orchestrationFlags\": {\"overrideStore\": true}"								\
	"}"


#define AH_EVENTHANDLER_EVENT_TYPE	"xensive-sensor"
#define AH_EVENTHANDLER_EVENT_METADATA				\
	"{"												\
        "\"floor\": \"1\"," 						\
        "\"section\": \"300\","						\
        "\"device\": \"3\","						\
        "\"sensor\": \"xensive1\""					\
    "}"

#define AH_EVENTHANDLER_EVENT_SOURCE				\
	"{"												\
		"\"systemName\": \"xensivekit\","			\
		"\"address\": \"127.0.0.1\", "				\
		"\"port\": 8911, "							\
		"\"authenticationInfo\": \"\""				\
	"}"


#define AH_EVENTHANDLER_EVENT_TIMESTAMP "2022-06-12T09:56:05Z"

#define LIBRARY_LOG_NAME    ( "AH" )

#define LIBRARY_LOG_LEVEL    LOG_DEBUG

#include "iot_logging_setup.h"

#endif /* APPLICATION_CODE_ARROWHEAD_AH_CONFIG_H_ */
