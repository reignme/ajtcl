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

#include "aj_nvram.h"
#include "aj_target_nvram.h"

uint8_t AJ_EMULATED_NVRAM[AJ_NVRAM_SIZE];
uint8_t* AJ_NVRAM_BASE_ADDRESS;

void AJ_NVRAM_Init()
{
    AJ_NVRAM_BASE_ADDRESS = AJ_EMULATED_NVRAM;
    static uint8_t inited = FALSE;
    if (!inited) {
        inited = TRUE;
        AJ_EraseNVRAM();
    }
}

void AJ_InvalidateNVEntry(uint16_t* inode)
{
    *inode = INVALID_ID;
}

void AJ_AppendNVEntry(uint8_t* nvPtr, AJ_NV_FILE* handle)
{
    *((uint32_t*)nvPtr) = handle->dataLen << 16 | handle->id;
    nvPtr += ENTRY_HEADER_SIZE;
    memcpy(nvPtr, handle->buf, handle->dataLen);
}

void AJ_EraseNVRAM()
{
    memset((uint8_t*)AJ_NVRAM_BASE_ADDRESS, INVALID_DATA_BYTE, AJ_NVRAM_SIZE);
    *((uint32_t*)AJ_NVRAM_BASE_ADDRESS) = AJ_NV_SENTINEL;
}

void AJ_OverriteNVRAM(uint8_t* bufPtr, uint16_t bytes)
{
    AJ_EraseNVRAM();
    memcpy(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET, bufPtr, bytes);
}

