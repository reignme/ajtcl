/**
 * @file
 */
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

#ifndef NDEBUG

#include <stdio.h>

#include "aj_host.h"
#include "aj_debug.h"

/*
 * Set to see the raw message bytes
 */
#define DUMP_MSG_RAW  0

#define Printable(c) (((c) >= ' ') && ((c) <= '~')) ? (c) : '.'

#define CHUNKING 16

void AJ_DumpBytes(const char* tag, const uint8_t* data, uint32_t len)
{
    uint32_t i;
    char ascii[CHUNKING + 1];

    if (tag) {
        printf("%s:\n", tag);
    }
    ascii[CHUNKING] = '\0';
    for (i = 0; i < len; i += CHUNKING) {
        uint32_t j;
        for (j = 0; j < CHUNKING; ++j, ++data) {
            if ((i + j) < len) {
                uint8_t n = *data;
                ascii[j] = Printable(n);
                printf((n < 0x10) ? "0%x " : "%x ", n);
            } else {
                ascii[j] = '\0';
                printf("   ");
            }
        }
        ascii[j] = '\0';
        printf("    %s\n", ascii);
    }
}

static const char* msgType[] = { "INVALID", "CALL", "REPLY", "ERROR", "SIGNAL" };

void AJ_DumpMsg(const char* tag, AJ_Message* msg, uint8_t body)
{
    uint8_t* p = (uint8_t*)msg->hdr + sizeof(AJ_MsgHeader);
    uint32_t hdrBytes = ((msg->hdr->headerLen + 7) & ~7);
    printf("%s message[%d] type %s sig=\"%s\"\n", tag, msg->hdr->serialNum, msgType[(msg->hdr->msgType <= 4) ? msg->hdr->msgType : 0], msg->signature);
    switch (msg->hdr->msgType) {
    case AJ_MSG_SIGNAL:
    case AJ_MSG_METHOD_CALL:
        printf("%s::%s\n", msg->iface, msg->member);
        break;

    case AJ_MSG_ERROR:
        printf("Error %s\n", msg->error);

    case AJ_MSG_METHOD_RET:
        printf("Reply serial %d\n", msg->replySerial);
        break;
    }
    printf("hdr len=%d\n", msg->hdr->headerLen);
#if DUMP_MSG_RAW
    AJ_DumpBytes(NULL, p,  hdrBytes);
    printf("body len=%d\n", msg->hdr->bodyLen);
    if (body) {
        AJ_DumpBytes(NULL, p + hdrBytes, msg->hdr->bodyLen);
    }
    printf("-----------------------\n");
#endif
}

#define AJ_CASE(_status) case _status: return # _status

const char* AJ_StatusText(AJ_Status status)
{
    switch (status) {
        AJ_CASE(AJ_OK);
        AJ_CASE(AJ_ERR_NULL);
        AJ_CASE(AJ_ERR_UNEXPECTED);
        AJ_CASE(AJ_ERR_IO_BUFFER);
        AJ_CASE(AJ_ERR_READ);
        AJ_CASE(AJ_ERR_WRITE);
        AJ_CASE(AJ_ERR_TIMEOUT);
        AJ_CASE(AJ_ERR_MARSHAL);
        AJ_CASE(AJ_ERR_UNMARSHAL);
        AJ_CASE(AJ_ERR_END_OF_DATA);
        AJ_CASE(AJ_ERR_RESOURCES);
        AJ_CASE(AJ_ERR_NO_MORE);
        AJ_CASE(AJ_ERR_SECURITY);
        AJ_CASE(AJ_ERR_CONNECT);
        AJ_CASE(AJ_ERR_UNKNOWN);
        AJ_CASE(AJ_ERR_NO_MATCH);
        AJ_CASE(AJ_ERR_SIGNATURE);
        AJ_CASE(AJ_ERR_DISALLOWED);
        AJ_CASE(AJ_ERR_FAILURE);
        AJ_CASE(AJ_ERR_RESTART);

    default:
        return "<unknown>";
    }
}

#endif
