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

#include "aj_helper.h"
#include "alljoyn.h"

#define UNMARSHAL_TIMEOUT (100 * 1000)
#define CONNECT_TIMEOUT   (60 * 1000)
#define CONNECT_PAUSE     (10 * 1000)

AJ_Status AJ_StartService(AJ_BusAttachment* bus,
                          const char* daemonName,
                          uint32_t timeout,
                          uint16_t port,
                          const char* name,
                          uint32_t flags,
                          const AJ_SessionOpts* opts
                          )
{
    return AJ_StartService2(bus, daemonName, timeout, FALSE, port, name, flags, opts);
}

AJ_Status AJ_StartService2(AJ_BusAttachment* bus,
                           const char* daemonName,
                           uint32_t timeout,
                           uint8_t connected,
                           uint16_t port,
                           const char* name,
                           uint32_t flags,
                           const AJ_SessionOpts* opts
                           )
{
    AJ_Status status;
    AJ_Time timer;
    uint8_t serviceStarted = FALSE;
    uint8_t initial = TRUE;
    AJ_InitTimer(&timer);

    while (TRUE) {
        if (AJ_GetElapsedTime(&timer, TRUE) > timeout) {
            return AJ_ERR_TIMEOUT;
        }
        if (!initial || !connected) {
            initial = FALSE;
            AJ_Printf("Attempting to connect to bus\n");
            status = AJ_Connect(bus, daemonName, CONNECT_TIMEOUT);
            if (status != AJ_OK) {
                AJ_Printf("Failed to connect to bus sleeping for %d seconds\n", CONNECT_PAUSE / 1000);
                AJ_Sleep(CONNECT_PAUSE);
                continue;
            }
            AJ_Printf("AllJoyn service connected to bus\n");
        }
        /*
         * Kick things off by binding a session port
         */
        status = AJ_BusBindSessionPort(bus, port, opts);
        if (status == AJ_OK) {
            break;
        }
        AJ_Printf("Failed to send bind session port message\n");
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
        AJ_Printf("AllJoyn disconnect bus status=%d\n", status);
        AJ_Disconnect(bus);
    }
    return status;
}

AJ_Status AJ_StartClient(AJ_BusAttachment* bus,
                         const char* daemonName,
                         uint32_t timeout,
                         const char* name,
                         uint16_t port,
                         uint32_t* sessionId,
                         const AJ_SessionOpts* opts
                         )
{
    return AJ_StartClient2(bus, daemonName, timeout, FALSE, name, port, sessionId, opts);
}

AJ_Status AJ_StartClient2(AJ_BusAttachment* bus,
                          const char* daemonName,
                          uint32_t timeout,
                          uint8_t connected,
                          const char* name,
                          uint16_t port,
                          uint32_t* sessionId,
                          const AJ_SessionOpts* opts
                          )
{
    AJ_Status status = AJ_OK;
    AJ_Time timer;
    uint8_t foundName = FALSE;
    uint8_t clientStarted = FALSE;
    uint8_t initial = TRUE;
    AJ_InitTimer(&timer);

    while (TRUE) {
        if (AJ_GetElapsedTime(&timer, TRUE) > timeout) {
            return AJ_ERR_TIMEOUT;
        }
        if (!initial || !connected) {
            initial = FALSE;
            AJ_Printf("Attempting to connect to bus\n");
            status = AJ_Connect(bus, daemonName, CONNECT_TIMEOUT);
            if (status != AJ_OK) {
                AJ_Printf("Failed to connect to bus sleeping for %d seconds\n", CONNECT_PAUSE / 1000);
                AJ_Sleep(CONNECT_PAUSE);
                continue;
            }
            AJ_Printf("AllJoyn client connected to bus\n");
        }
        /*
         * Kick things off by finding the service names
         */
        status = AJ_BusFindAdvertisedName(bus, name, AJ_BUS_START_FINDING);
        if (status == AJ_OK) {
            break;
        }
        AJ_Printf("FindAdvertisedName failed\n");
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
        case AJ_REPLY_ID(AJ_METHOD_FIND_NAME_BY_TRANSPORT):
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
                AJ_Printf("FoundAdvertisedName(%s)\n", arg.val.v_string);
                foundName = TRUE;
                status = AJ_BusJoinSession(bus, arg.val.v_string, port, opts);
            }
            break;

        case AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION):
            {
                uint32_t replyCode;

                if (msg.hdr->msgType == AJ_MSG_ERROR) {
                    status = AJ_ERR_FAILURE;
                } else {
                    status = AJ_UnmarshalArgs(&msg, "uu", &replyCode, sessionId);
                    if (replyCode == AJ_JOINSESSION_REPLY_SUCCESS) {
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
        AJ_Printf("AllJoyn disconnect bus status=%d\n", status);
        AJ_Disconnect(bus);
    }
    return status;
}

