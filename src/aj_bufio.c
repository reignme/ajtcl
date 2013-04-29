/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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
        memmove(ioBuf->bufStart, ioBuf->readPtr, unconsumed);
    }

    ioBuf->readPtr = ioBuf->bufStart;
    ioBuf->writePtr = ioBuf->bufStart + unconsumed;
}
