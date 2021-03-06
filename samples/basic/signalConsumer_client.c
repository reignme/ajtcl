/*
 * @file
 */

/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "alljoyn.h"

/**
 * Static constants.
 */
static const char InterfaceName[] = "org.alljoyn.Bus.signal_sample";
static const char ServiceName[] = "org.alljoyn.Bus.signal_sample";
static const char ServicePath[] = "/";
static const uint16_t ServicePort = 25;

/**
 * The interface name followed by the method signatures.
 * This sample receives a signal of a property change in the sample signal_service.
 *
 * See also .\inc\aj_introspect.h
 */
static const char* const sampleInterface[] = {
    InterfaceName,              /* The first entry is the interface name. */
    "!nameChanged newName>s",   /* Signal at index 0 with an output string of the new name. */
    NULL
};

/**
 * A NULL terminated collection of all interfaces.
 */
static const AJ_InterfaceDescription sampleInterfaces[] = {
    sampleInterface,
    NULL
};

/**
 * Objects implemented by the application. The first member in the AJ_Object structure is the path.
 * The second is the collection of all interfaces at that path.
 */
static const AJ_Object AppObjects[] = {
    { ServicePath, sampleInterfaces },
    { NULL }
};

/*
 * The value of the arguments are the indices of the object path in AppObjects (above),
 * interface in sampleInterfaces (above), and member indices in the interface.
 * The 'name' index is 0 because the first entry in sampleInterface is the interface name.
 *
 * Encode the property id from the object path, interface, and member indices.
 *
 * See also .\inc\aj_introspect.h
 */
#define NAMECHANGE_SIGNAL AJ_APP_MESSAGE_ID(0, 0, 0)

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

AJ_Status ReceiveNewName(AJ_Message*msg)
{
    AJ_Arg arg;
    AJ_Status status = AJ_UnmarshalArg(msg, &arg);

    if (status == AJ_OK) {
        printf("--==## signalConsumer: Name Changed signal Received ##==--\n");
        printf("\tNew name: '%s'.\n", arg.val.v_string);
    }

    return status;
}

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint8_t done = FALSE;
    uint32_t sessionId = 0;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(NULL, AppObjects);

    while (!done) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartClient(&bus,
                                    NULL,
                                    CONNECT_TIMEOUT,
                                    ServiceName,
                                    ServicePort,
                                    &sessionId,
                                    NULL);

            if (status == AJ_OK) {
                printf("StartClient returned %d, sessionId=%u.\n", status, sessionId);
                connected = TRUE;
            } else {
                printf("StartClient returned 0x%04x.\n", status);
                break;
            }
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        switch (status) {
        case AJ_OK:
            /*
             * The contents of the message are meaningful, only when
             * the message was unmarshaled successfully.
             */
            switch (msg.msgId) {
            case NAMECHANGE_SIGNAL:
                ReceiveNewName(&msg);
                break;

            case AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION):
                printf("JoinSession SUCCESS (Session id=%d).\n", sessionId);
                break;

            case AJ_SIGNAL_SESSION_LOST:
                /* Force a disconnect. */
                status = AJ_ERR_READ;
                break;

            default:
                /* Pass to the built-in handlers. */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
            break;

        case AJ_ERR_NULL:
            printf("AJ_UnmarshalMsg() returned 'Unexpected NULL pointer'.\n");
            break;

        case AJ_ERR_UNEXPECTED:
            printf("AJ_UnmarshalMsg() returned 'An operation was unexpected at this time'.\n");
            break;

        case AJ_ERR_INVALID:
            printf("AJ_UnmarshalMsg() returned 'A value was invalid'.\n");
            break;

        case AJ_ERR_IO_BUFFER:
            printf("AJ_UnmarshalMsg() returned 'An I/O buffer was invalid or in the wrong state'.\n");
            break;

        case AJ_ERR_READ:
            printf("AJ_UnmarshalMsg() returned 'An error while reading data from the network'.\n");
            break;

        case AJ_ERR_WRITE:
            printf("AJ_UnmarshalMsg() returned 'An error while writing data to the network'.\n");
            break;

        case AJ_ERR_TIMEOUT:
            printf("AJ_UnmarshalMsg() returned 'A timeout occurred'.\n");
            break;

        case AJ_ERR_MARSHAL:
            printf("AJ_UnmarshalMsg() returned 'Marshaling failed due to badly constructed message argument'.\n");
            break;

        case AJ_ERR_UNMARSHAL:
            printf("AJ_UnmarshalMsg() returned 'Unmarshaling failed due to a corrupt or invalid message'.\n");
            break;

        case AJ_ERR_END_OF_DATA:
            printf("AJ_UnmarshalMsg() returned 'No enough data'.\n");
            break;

        case AJ_ERR_RESOURCES:
            printf("AJ_UnmarshalMsg() returned 'Insufficient memory to perform the operation'.\n");
            break;

        case AJ_ERR_NO_MORE:
            printf("AJ_UnmarshalMsg() returned 'Attempt to unmarshal off the end of an array'.\n");
            break;

        case AJ_ERR_SECURITY:
            printf("AJ_UnmarshalMsg() returned 'Authentication or decryption failed'.\n");
            break;

        case AJ_ERR_CONNECT:
            printf("AJ_UnmarshalMsg() returned 'Network connect failed'.\n");
            break;

        case AJ_ERR_UNKNOWN:
            printf("AJ_UnmarshalMsg() returned 'A unknown value'.\n");
            break;

        case AJ_ERR_NO_MATCH:
            printf("AJ_UnmarshalMsg() returned 'Something didn't match'.\n");
            break;

        case AJ_ERR_SIGNATURE:
            printf("AJ_UnmarshalMsg() returned 'Signature is not what was expected'.\n");
            break;

        case AJ_ERR_DISALLOWED:
            printf("AJ_UnmarshalMsg() returned 'An operations was not allowed'.\n");
            break;

        case AJ_ERR_FAILURE:
            printf("AJ_UnmarshalMsg() returned 'A failure has occured'.\n");
            break;

        case AJ_ERR_RESTART:
            printf("AJ_UnmarshalMsg() returned 'The OEM event loop must restart'.\n");
            break;

        case AJ_ERR_LINK_TIMEOUT:
            printf("AJ_UnmarshalMsg() returned 'The bus link is inactive too long'.\n");
            break;

        case AJ_ERR_DRIVER:
            printf("AJ_UnmarshalMsg() returned 'An error communicating with a lower-layer driver'.\n");
            break;
        }

        /* Messages MUST be discarded to free resources. */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            printf("AllJoyn disconnect.\n");
            AJ_Disconnect(&bus);
            exit(0);
        }
    }

    printf("signalConsumer_Client exiting with status 0x%04x.\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
