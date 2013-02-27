/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

#include "aj_helper.h"
#include "alljoyn.h"

#define UNMARSHAL_TIMEOUT (100 * 1000)


#define JOINSESSION_REPLY_SUCCESS              1   /**< JoinSession reply: Success */
#define JOINSESSION_REPLY_NO_SESSION           2   /**< JoinSession reply: Session with given name does not exist */
#define JOINSESSION_REPLY_UNREACHABLE          3   /**< JoinSession reply: Failed to find suitable transport */
#define JOINSESSION_REPLY_CONNECT_FAILED       4   /**< JoinSession reply: Connect to advertised address */
#define JOINSESSION_REPLY_REJECTED             5   /**< JoinSession reply: The session creator rejected the join req */
#define JOINSESSION_REPLY_BAD_SESSION_OPTS     6   /**< JoinSession reply: Failed due to session option incompatibilities */
#define JOINSESSION_REPLY_ALREADY_JOINED       7   /**< JoinSession reply: Caller has already joined this session */
#define JOINSESSION_REPLY_FAILED              10   /**< JoinSession reply: Failed for unknown reason */

#define CONNECT_TIMEOUT   (60 * 1000)
#define CONNECT_PAUSE     (10 * 1000)

AJ_Status AJ_StartService(AJ_BusAttachment* bus,
                          const char* daemonName,
                          uint32_t timeout,
                          uint16_t port,
                          const char* name,
                          uint32_t flags)
{
    AJ_Status status;
    AJ_Time timer;
    uint8_t serviceStarted = FALSE;

    AJ_InitTimer(&timer);

    while (TRUE) {
        if (AJ_GetElapsedTime(&timer, TRUE) > timeout) {
            return AJ_ERR_TIMEOUT;
        }
        printf("Attempting to connect to bus\n");
        status = AJ_Connect(bus, daemonName, CONNECT_TIMEOUT);
        if (status != AJ_OK) {
            printf("Failed to connect to bus sleeping for %d seconds\n", CONNECT_PAUSE / 1000);
            AJ_Sleep(CONNECT_PAUSE);
            continue;
        }
        printf("AllJoyn service connected to bus\n");
        /*
         * Kick things off by binding a session port
         */
        status = AJ_BusBindSessionPort(bus, port, NULL);
        if (status == AJ_OK) {
            break;
        }
        printf("Failed to send bind session port message\n");
        AJ_Disconnect(bus);
    }

    while (!serviceStarted && (status == AJ_OK)) {
        AJ_Message msg;

        status = AJ_UnmarshalMsg(bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            break;
        }

        switch (msg.msgId) {
        case AJ_REPLY_ID(AJ_METHOD_BIND_SESSION_PORT):
            if (msg.hdr->msgType == AJ_MSG_ERROR) {
                status = AJ_ERR_FAILURE;
            } else {
                status = AJ_BusRequestName(bus, name, flags);
            }
            break;

        case AJ_REPLY_ID(AJ_METHOD_REQUEST_NAME):
            if (msg.hdr->msgType == AJ_MSG_ERROR) {
                status = AJ_ERR_FAILURE;
            } else {
                status = AJ_BusAdvertiseName(bus, name, AJ_TRANSPORT_ANY, AJ_BUS_START_ADVERTISING);
            }
            break;

        case AJ_REPLY_ID(AJ_METHOD_ADVERTISE_NAME):
            if (msg.hdr->msgType == AJ_MSG_ERROR) {
                status = AJ_ERR_FAILURE;
            } else {
                serviceStarted = TRUE;
            }
            break;

        default:
            /*
             * Pass to the built-in bus message handlers
             */
            status = AJ_BusHandleBusMessage(&msg);
            break;
        }
        AJ_CloseMsg(&msg);
    }

    if (status != AJ_OK) {
        printf("AllJoyn disconnect bus status=%d\n", status);
        AJ_Disconnect(bus);
    }
    return status;
}

AJ_Status AJ_StartClient(AJ_BusAttachment* bus,
                         const char* daemonName,
                         uint32_t timeout,
                         const char* name,
                         uint16_t port,
                         uint32_t* sessionId)
{
    AJ_Status status = AJ_OK;
    AJ_Time timer;
    uint8_t foundName = FALSE;
    uint8_t clientStarted = FALSE;

    AJ_InitTimer(&timer);

    while (TRUE) {
        if (AJ_GetElapsedTime(&timer, TRUE) > timeout) {
            return AJ_ERR_TIMEOUT;
        }
        printf("Attempting to connect to bus\n");
        status = AJ_Connect(bus, daemonName, CONNECT_TIMEOUT);
        if (status != AJ_OK) {
            printf("Failed to connect to bus sleeping for %d seconds\n", CONNECT_PAUSE / 1000);
            AJ_Sleep(CONNECT_PAUSE);
            continue;
        }
        printf("AllJoyn client connected to bus\n");
        /*
         * Kick things off by finding the service names
         */
        status = AJ_BusFindAdvertisedName(bus, name, AJ_BUS_START_FINDING);
        if (status == AJ_OK) {
            break;
        }
        printf("FindAdvertisedName failed\n");
        AJ_Disconnect(bus);
    }

    *sessionId = 0;

    while (!clientStarted && (status == AJ_OK)) {
        AJ_Message msg;

        if (AJ_GetElapsedTime(&timer, TRUE) > timeout) {
            return AJ_ERR_TIMEOUT;
        }
        status = AJ_UnmarshalMsg(bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            /*
             * Timeouts are expected until we find a name
             */
            if ((status == AJ_ERR_TIMEOUT) && !foundName) {
                status = AJ_OK;
            }
            continue;
        }

        switch (msg.msgId) {

        case AJ_REPLY_ID(AJ_METHOD_FIND_NAME):
            if (msg.hdr->msgType == AJ_MSG_ERROR) {
                status = AJ_ERR_FAILURE;
            } else {
                uint32_t disposition;
                AJ_UnmarshalArgs(&msg, "u", &disposition);
                if ((disposition != AJ_FIND_NAME_STARTED) && (disposition != AJ_FIND_NAME_ALREADY)) {
                    status = AJ_ERR_FAILURE;
                }
            }
            break;

        case AJ_SIGNAL_FOUND_ADV_NAME:
            {
                AJ_Arg arg;
                AJ_UnmarshalArg(&msg, &arg);
                printf("FoundAdvertisedName(%s)\n", arg.val.v_string);
                foundName = TRUE;
                status = AJ_BusJoinSession(bus, arg.val.v_string, port, NULL);
            }
            break;

        case AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION):
            {
                uint32_t replyCode;

                if (msg.hdr->msgType == AJ_MSG_ERROR) {
                    status = AJ_ERR_FAILURE;
                } else {
                    status = AJ_UnmarshalArgs(&msg, "uu", &replyCode, sessionId);
                    if (replyCode == JOINSESSION_REPLY_SUCCESS) {
                        clientStarted = TRUE;
                    } else {
                        status = AJ_ERR_FAILURE;
                    }
                }
            }
            break;

        case AJ_SIGNAL_SESSION_LOST:
            /*
             * Force a disconnect
             */
            status = AJ_ERR_READ;
            break;

        default:
            /*
             * Pass to the built-in bus message handlers
             */
            status = AJ_BusHandleBusMessage(&msg);
            break;
        }
        AJ_CloseMsg(&msg);
    }
    if (status != AJ_OK) {
        printf("AllJoyn disconnect bus status=%d\n", status);
        AJ_Disconnect(bus);
    }
    return status;
}
