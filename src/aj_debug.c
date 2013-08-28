/**
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

#include "aj_debug.h"

#ifndef NDEBUG

#include "aj_target.h"

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
        AJ_Printf("%s:\n", tag);
    }
    ascii[CHUNKING] = '\0';
    for (i = 0; i < len; i += CHUNKING) {
        uint32_t j;
        for (j = 0; j < CHUNKING; ++j, ++data) {
            if ((i + j) < len) {
                uint8_t n = *data;
                ascii[j] = Printable(n);
                if (n < 0x10) {
                    AJ_Printf("0%x ", n);
                } else {
                    AJ_Printf("%x ", n);
                }
            } else {
                ascii[j] = '\0';
                AJ_Printf("   ");
            }
        }
        ascii[j] = '\0';
        AJ_Printf("    %s\n", ascii);
    }
}

static const char* msgType[] = { "INVALID", "CALL", "REPLY", "ERROR", "SIGNAL" };

void AJ_DumpMsg(const char* tag, AJ_Message* msg, uint8_t body)
{
#if DUMP_MSG_RAW
    uint8_t* p = (uint8_t*)msg->hdr + sizeof(AJ_MsgHeader);
    uint32_t hdrBytes = ((msg->hdr->headerLen + 7) & ~7);
#endif
    AJ_Printf("%s message[%d] type %s sig=\"%s\"\n", tag, msg->hdr->serialNum, msgType[(msg->hdr->msgType <= 4) ? msg->hdr->msgType : 0], msg->signature);
    switch (msg->hdr->msgType) {
    case AJ_MSG_SIGNAL:
    case AJ_MSG_METHOD_CALL:
        AJ_Printf("%s::%s\n", msg->iface, msg->member);
        break;

    case AJ_MSG_ERROR:
        AJ_Printf("Error %s\n", msg->error);

    case AJ_MSG_METHOD_RET:
        AJ_Printf("Reply serial %d\n", msg->replySerial);
        break;
    }
    AJ_Printf("hdr len=%d\n", msg->hdr->headerLen);
#if DUMP_MSG_RAW
    AJ_DumpBytes(NULL, p,  hdrBytes);
    AJ_Printf("body len=%d\n", msg->hdr->bodyLen);
    if (body) {
        AJ_DumpBytes(NULL, p + hdrBytes, msg->hdr->bodyLen);
    }
    AJ_Printf("-----------------------\n");
#endif
}

#endif

#define AJ_CASE(_status) case _status: return # _status

const char* AJ_StatusText(AJ_Status status)
{
#ifdef NDEBUG
    /* Expectation is that thin client status codes will NOT go beyond 255 */
    static char code[4];

#ifdef _WIN32
    _snprintf(code, sizeof(code), "%03u", status);
#else
    snprintf(code, sizeof(code), "%03u", status);
#endif

    return code;
#else
    switch (status) {
        AJ_CASE(AJ_OK);
        AJ_CASE(AJ_ERR_NULL);
        AJ_CASE(AJ_ERR_UNEXPECTED);
        AJ_CASE(AJ_ERR_INVALID);
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
        AJ_CASE(AJ_ERR_LINK_TIMEOUT);
        AJ_CASE(AJ_ERR_DRIVER);
        AJ_CASE(AJ_ERR_OBJECT_PATH);

    default:
        return "<unknown>";
    }
#endif
}
