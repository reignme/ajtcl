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

#include <assert.h>

#include "aj_host.h"
#include "aj_status.h"
#include "aj_bufio.h"

void AJ_IOBufInit(AJ_IOBuffer* ioBuf, uint8_t* buffer, uint32_t bufLen, uint8_t direction, void* context)
{
    ioBuf->bufStart = buffer;
    ioBuf->bufSize = bufLen;
    ioBuf->readPtr = buffer;
    ioBuf->writePtr = buffer;
    ioBuf->direction = direction;
    ioBuf->context = context;
}

void AJ_IOBufRebase(AJ_IOBuffer* ioBuf)
{
    int32_t unconsumed = AJ_IO_BUF_AVAIL(ioBuf);
    /*
     * Move any unconsumed data to the start of the I/O buffer
     */
    if (unconsumed) {
        if (ioBuf->direction == AJ_IO_BUF_RX) {
            memmove(ioBuf->bufStart, ioBuf->writePtr, unconsumed);
        } else {
            memmove(ioBuf->bufStart, ioBuf->readPtr, unconsumed);
        }
    }
    ioBuf->readPtr = ioBuf->bufStart;
    ioBuf->writePtr = ioBuf->bufStart + unconsumed;
}
