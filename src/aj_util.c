/**
 * @file
 */
/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

#include <assert.h>

#include "aj_target.h"
#include "aj_util.h"

static uint8_t A2H(char hex, AJ_Status* status)
{
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    }
    hex |= 0x20;
    if (hex >= 'a' && hex <= 'f') {
        return 10 + hex - 'a';
    } else {
        *status = AJ_ERR_INVALID;
        return 0;
    }
}

int32_t AJ_StringFindFirstOf(const char* str, char* chars)
{
    if (str) {
        const char* p = str;
        do {
            const char* c = chars;
            while (*c) {
                if (*p == *c++) {
                    return (int32_t)(p - str);
                }
            }
        } while (*(++p));
    }
    return -1;
}

AJ_Status AJ_RawToHex(const uint8_t* raw, size_t rawLen, char* hex, size_t hexLen)
{
    static const char nibble[] = "0123456789ABCDEF";
    char* h = hex + 2 * rawLen;
    const uint8_t* a = raw + rawLen;

    if ((2 * rawLen + 1) > hexLen) {
        return AJ_ERR_RESOURCES;
    }
    h[0] = '\0';
    /*
     * Running backwards encode each byte in inStr as a pair of ascii hex digits.
     * Going backwards allows the raw and hex buffers to be the same buffer.
     */
    while (rawLen--) {
        uint8_t n = *(--a);
        h -= 2;
        h[0] = nibble[n >> 4];
        h[1] = nibble[n & 0xF];
    }
    return AJ_OK;
}

AJ_Status AJ_HexToRaw(const char* hex, size_t hexLen, uint8_t* raw, size_t rawLen)
{
    AJ_Status status = AJ_OK;
    char* p = (char*)raw;
    size_t sz = hexLen ? hexLen : strlen(hex);
    size_t i;

    /*
     * Length of encoded hex must be an even number
     */
    if (sz & 1) {
        return AJ_ERR_UNEXPECTED;
    }
    if (rawLen < (sz / 2)) {
        return AJ_ERR_RESOURCES;
    }
    for (i = 0; (i < sz) && (status == AJ_OK); i += 2, hex += 2) {
        *p++ = (A2H(hex[0], &status) << 4) | A2H(hex[1], &status);
    }
    return status;
}
