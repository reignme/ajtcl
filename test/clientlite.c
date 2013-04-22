/*
 * clientlite.c
 */

/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>

#include "alljoyn.h"
#include "aj_debug.h"
#include "aj_crypto.h"

static const char ServiceName[] = "org.alljoyn.svclite";
static const uint16_t ServicePort = 24;

/*
 * Use AJ_FLAG_ENCRYPTED for authFlag to request app-to-app authentication
 */
static uint8_t authFlag = 0;

static const char* testInterface[] = {
    "org.alljoyn.alljoyn_test",
    "?my_ping inStr<s outStr>s",
    "?sum_of_array_elements inArray<ay outSum>t",
    "?max_of_array_elements inArray<ay outMax>y",
    NULL
};


static const char* testValuesInterface[] = {
    "org.alljoyn.alljoyn_test.values",
    "@int_val=i",
    NULL
};

static const AJ_InterfaceDescription testInterfaces[] = {
    AJ_PropertiesIface,
    testInterface,
    testValuesInterface,
    NULL
};

/**
 * Objects implemented by the application
 */
static const AJ_Object ProxyObjects[] = {
    { "/org/alljoyn/alljoyn_test", testInterfaces },
    { NULL }
};

#define PRX_GET_PROP  AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_GET)
#define PRX_SET_PROP  AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_SET)
#define PRX_MY_PING   AJ_PRX_MESSAGE_ID(0, 1, 0)
#define PRX_SUM_OF_ARRAY_ELEMENTS   AJ_PRX_MESSAGE_ID(0, 1, 1)
#define PRX_MAX_OF_ARRAY_ELEMENTS   AJ_PRX_MESSAGE_ID(0, 1, 2)
#define PRX_GET_INT   AJ_PRX_PROPERTY_ID(0, 2, 0)
#define PRX_SET_INT   AJ_PRX_PROPERTY_ID(0, 2, 0)

static AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num);
static AJ_Status SendComputeSumOfArrayElements(AJ_BusAttachment* bus, uint32_t sessionId);
static AJ_Status SendComputeMaxOfArrayElements(AJ_BusAttachment* bus, uint32_t sessionId);

/*
 * Let the application do some work
 */
static int32_t g_iterCount = 0;
static void AppDoWork(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Printf("AppDoWork");
    /*
     * This function is called if there are no messages to unmarshal
     */
    g_iterCount = g_iterCount + 1;
    SendPing(bus, sessionId, g_iterCount);
}

static const char PWD[] = "ABCDEFGH";

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    memcpy(buffer, PWD, sizeof(PWD));
    return sizeof(PWD) - 1;
}

#define CONNECT_TIMEOUT    (1000 * 200)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

static const char PingString[] = "Ping String";

AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_MY_PING, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "s", PingString);
    }
    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

/*
 * Given that the type of field 'len' in AJ_Arg is declared as uint16_t,
 * the number of elements in an array can atmost be UINT16_MAX.
 * However, in case of AJ_DeliverMsgPartial, one needs to marshal
 * length (number of elements) and then marshal the array.
 * The length itself would need sizeof(uint16_t) = two octets.
 * Hence, the max length of array that can be sent across is (UINT16_MAX - 2).
 *
 * The max length of array that can be sent across using the regular means
 * of initializing a method argument is much lesser. The transmit buffer
 * has a limited size (1024 octets) and the AllJoyn headers comprising
 * of various flags, service name, object path, interface name and method name
 * take up space in that buffer. What remains after marshaling these,
 * is available for the app. For example, in the particular configuration
 * of this test app, the buffer space that remains is 820 octets.
 */
enum {MAX_ELEMENTS = 820};
static uint8_t Data8[MAX_ELEMENTS];

static uint64_t expected_sum = 0;

static AJ_Status SendComputeSumOfArrayElements(AJ_BusAttachment* bus, uint32_t sessionId) {
    /* Reset value of expected_sum before doing anything */
    expected_sum = 0;

    const uint16_t num_elements_in_array = ArraySize(Data8);
    uint32_t full_size_of_message = num_elements_in_array + sizeof(num_elements_in_array);

    uint16_t i = 0; /* array index */

    /* Initialize the array and compute its sum for our verification */
    for (i = 0; i < ArraySize(Data8); i++) {
        uint8_t random_byte = 0;
        AJ_RandBytes(&random_byte, 1);
        Data8[i] = random_byte;

        expected_sum += Data8[i];
    }
    AJ_Printf("INFO: The expected sum of elements is %lu.\n", expected_sum);

    AJ_Status status = AJ_ERR_FAILURE;

    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_SUM_OF_ARRAY_ELEMENTS, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (AJ_OK != status) {
        AJ_Printf("ERROR: MarshalMethodCall failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeSumOfArrayElementsErrorExit;
    }

    /*
     * The signature of the method call is "ay" (array of bytes).
     * Hence we need to first marshal the length and then
     * marshal the elements one by one.
     * To do that, we need to know the full size of the message
     * we will marshal.
     */

    status = AJ_DeliverMsgPartial(&msg, full_size_of_message);
    if (AJ_OK != status) {
        AJ_Printf("ERROR: DeliverMsgPartial failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeSumOfArrayElementsErrorExit;
    }

    /* Marshal the lenth of the array */
    status = AJ_MarshalRaw(&msg, &num_elements_in_array, sizeof(num_elements_in_array));
    if (AJ_OK != status) {
        AJ_Printf("ERROR: Marshaling length failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeSumOfArrayElementsErrorExit;
    }

    for (i = 0; i < num_elements_in_array; i++) {
        status = AJ_MarshalRaw(&msg, &(Data8[i]), sizeof(Data8[i]));
        if (AJ_OK != status) {
            AJ_Printf("ERROR: Marshaling element (index: %d) failed. Got status: %s.\n", i, AJ_StatusText(status));
            goto SendComputeSumOfArrayElementsErrorExit;
        }
    }

    status = AJ_DeliverMsg(&msg);
    if (AJ_OK != status) {
        AJ_Printf("ERROR: DeliverMsg failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeSumOfArrayElementsErrorExit;
    }

SendComputeSumOfArrayElementsErrorExit:

    AJ_CloseMsg(&msg);
    return status;
}

static uint8_t expected_max = 0;

static AJ_Status SendComputeMaxOfArrayElements(AJ_BusAttachment* bus, uint32_t sessionId) {
    /* Reset value of expected_max before doing anything */
    expected_max = 0;

    uint16_t i = 0; /* Array index */

    /* Initialize the array and compute its sum for our verification */
    for (i = 0; i < ArraySize(Data8); i++) {
        uint8_t random_byte = 0;
        AJ_RandBytes(&random_byte, 1);
        Data8[i] = random_byte;

        expected_max = (expected_max >= Data8[i]) ? expected_max : Data8[i];
    }
    AJ_Printf("INFO: The expected max of elements is %u.\n", expected_max);

    AJ_Status status = AJ_ERR_FAILURE;

    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_MAX_OF_ARRAY_ELEMENTS, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (AJ_OK != status) {
        AJ_Printf("ERROR: MarshalMethodCall failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeMaxOfArrayElementsErrorExit;
    }

    AJ_Arg byte_array;
    status = AJ_MarshalArg(&msg, AJ_InitArg(&byte_array, AJ_ARG_BYTE, AJ_ARRAY_FLAG, Data8, ArraySize(Data8)));
    if (AJ_OK != status) {
        AJ_Printf("ERROR: MarshalArg failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeMaxOfArrayElementsErrorExit;
    }

    status = AJ_DeliverMsg(&msg);
    if (AJ_OK != status) {
        AJ_Printf("ERROR: DeliverMsg failed. Got status: %s.\n", AJ_StatusText(status));
        goto SendComputeMaxOfArrayElementsErrorExit;
    }

SendComputeMaxOfArrayElementsErrorExit:

    AJ_CloseMsg(&msg);
    return status;
}

AJ_Status SendGetProp(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_GET_PROP, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        AJ_MarshalPropertyArgs(&msg, PRX_GET_INT);
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

AJ_Status SendSetProp(AJ_BusAttachment* bus, uint32_t sessionId, int val)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_SET_PROP, ServiceName, sessionId, 0, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_MarshalPropertyArgs(&msg, PRX_SET_INT);

        if (status == AJ_OK) {
            status = AJ_MarshalArgs(&msg, "i", val);
        } else {
            AJ_Printf(">>>>>>>>In SendSetProp() AJ_MarshalPropertyArgs() returned status = 0x%04x\n", status);
        }

        if (status == AJ_OK) {
            status = AJ_DeliverMsg(&msg);
        } else {
            AJ_Printf(">>>>>>>>In SendSetProp() AJ_MarshalArgs() returned status = 0x%04x\n", status);
        }
    }

    return status;
}

void AuthCallback(const void* context, AJ_Status status)
{
    *((AJ_Status*)context) = status;
}

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint32_t sessionId = 0;
    AJ_Status authStatus = AJ_ERR_NULL;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(ProxyObjects);
    AJ_RegisterObjects(NULL, ProxyObjects);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartClient(&bus, NULL, CONNECT_TIMEOUT, ServiceName, ServicePort, &sessionId, NULL);
            if (status == AJ_OK) {
                AJ_Printf("StartClient returned %d, sessionId=%u\n", status, sessionId);
                AJ_Printf("Connected to Daemon:%s\n", AJ_GetUniqueName(&bus));
                connected = TRUE;
                if (authFlag) {
                    AJ_BusSetPasswordCallback(&bus, PasswordCallback);
                    status = AJ_BusAuthenticatePeer(&bus, ServiceName, AuthCallback, &authStatus);
                    if (status != AJ_OK) {
                        AJ_Printf("AJ_BusAuthenticatePeer returned %d\n", status);
                    }
                } else {
                    authStatus = AJ_OK;
                }
            } else {
                AJ_Printf("StartClient returned %d\n", status);
                break;
            }
        }

        if (authStatus != AJ_ERR_NULL) {
            if (authStatus != AJ_OK) {
                AJ_Disconnect(&bus);
                break;
            }
            authStatus = AJ_ERR_NULL;
            SendPing(&bus, sessionId, 1);
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork(&bus, sessionId);
                continue;
            }
        } else {
            switch (msg.msgId) {

            case AJ_REPLY_ID(PRX_MY_PING):
                {
                    AJ_Arg arg;
                    AJ_UnmarshalArg(&msg, &arg);
                    status = SendComputeSumOfArrayElements(&bus, sessionId);
                }
                break;

            case AJ_REPLY_ID(PRX_SUM_OF_ARRAY_ELEMENTS):
                {
                    uint64_t actual_sum = 0;
                    status = AJ_UnmarshalArgs(&msg, "t", &actual_sum);
                    if (AJ_OK != status) {
                        AJ_Printf("ERROR: UnmarshalArgs of method reply failed. Got status: %s.\n", AJ_StatusText(status));
                    } else {
                        AJ_Printf("%s: ", (expected_sum == actual_sum) ? "SUCCESS" : "FAILURE");
                        AJ_Printf("The sum returned by method call is %lu and it %s with expected sum %lu.\n", actual_sum, (expected_sum == actual_sum) ? "matches" : "does NOT match", expected_sum);
                    }
                    status = SendComputeMaxOfArrayElements(&bus, sessionId);
                }
                break;

            case AJ_REPLY_ID(PRX_MAX_OF_ARRAY_ELEMENTS):
                {
                    uint8_t actual_max = 0;
                    status = AJ_UnmarshalArgs(&msg, "y", &actual_max);
                    if (AJ_OK != status) {
                        AJ_Printf("ERROR: UnmarshalArgs of method reply failed. Got status: %s.\n", AJ_StatusText(status));
                    } else {
                        AJ_Printf("%s: ", (expected_max == actual_max) ? "SUCCESS" : "FAILURE");
                        AJ_Printf("The max returned by method call is %u and it %s with expected max %u.\n", actual_max, (expected_max == actual_max) ? "matches" : "does NOT match", expected_max);
                    }
                    status = SendGetProp(&bus, sessionId);
                }
                break;

            case AJ_REPLY_ID(PRX_GET_PROP):
                {
                    const char* sig;
                    status = AJ_UnmarshalVariant(&msg, &sig);
                    if (status == AJ_OK) {
                        status = AJ_UnmarshalArgs(&msg, sig, &g_iterCount);
                        AJ_Printf("Get prop reply %d\n", g_iterCount);

                        if (status == AJ_OK) {
                            g_iterCount = g_iterCount + 1;
                            status = SendSetProp(&bus, sessionId, g_iterCount);
                        }
                    }
                }
                break;

            case AJ_REPLY_ID(PRX_SET_PROP):
                AJ_Printf("Set prop reply\n");
                break;

            case AJ_SIGNAL_SESSION_LOST:
                /*
                 * Force a disconnect
                 */
                status = AJ_ERR_READ;
                break;

            default:
                /*
                 * Pass to the built-in handlers
                 */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
        }
        /*
         * Messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            AJ_Printf("AllJoyn disconnect\n");
            AJ_Printf("Disconnected from Daemon:%s\n", AJ_GetUniqueName(&bus));
            AJ_Disconnect(&bus);
            return status;
        }
    }
    AJ_Printf("clientlite EXIT %d\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    // authFlag = AJ_FLAG_ENCRYPTED;
    return AJ_Main();
}
#endif

