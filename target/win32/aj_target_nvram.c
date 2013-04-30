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

void AJ_NVRAM_Init()
{
    AJ_LoadNVFromFile();
    if (*((uint32_t*)AJ_NVRAM_BASE_ADDRESS) != AJ_NV_SENTINEL) {
        AJ_EraseNVRAM();
        AJ_StoreNVToFile();
    }
}

void AJ_InvalidateNVEntry(uint16_t* inode)
{
    *inode = INVALID_ID;
    AJ_StoreNVToFile();
}

void AJ_AppendNVEntry(uint8_t* nvPtr, AJ_NV_FILE* handle)
{
    *((uint32_t*)nvPtr) = handle->dataLen << 16 | handle->id;
    nvPtr += ENTRY_HEADER_SIZE;
    memcpy(nvPtr, handle->buf, handle->dataLen);
    AJ_StoreNVToFile();
}

void AJ_EraseNVRAM()
{
    memset((uint8_t*)AJ_NVRAM_BASE_ADDRESS, INVALID_DATA_BYTE, AJ_NVRAM_SIZE);
    *((uint32_t*)AJ_NVRAM_BASE_ADDRESS) = AJ_NV_SENTINEL;
    AJ_StoreNVToFile();
}

void AJ_OverriteNVRAM(uint8_t* bufPtr, uint16_t bytes)
{
    AJ_EraseNVRAM();
    memcpy(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET, bufPtr, bytes);
    AJ_StoreNVToFile();
}

AJ_Status AJ_LoadNVFromFile()
{
    FILE* f = fopen("ajlite.nvram", "r");
    if (f == NULL) {
        AJ_Printf("Error: LoadNVFromFile() failed\n");
        return AJ_ERR_FAILURE;
    }

    memset(AJ_NVRAM_BASE_ADDRESS, INVALID_DATA_BYTE, AJ_NVRAM_SIZE);
    fread(AJ_NVRAM_BASE_ADDRESS, AJ_NVRAM_SIZE, 1, f);
    fclose(f);
    return AJ_OK;
}

AJ_Status AJ_StoreNVToFile()
{
    FILE* f = fopen("ajlite.nvram", "w");
    if (!f) {
        AJ_Printf("Error: StoreNVToFile() failed\n");
        return AJ_ERR_FAILURE;
    }

    fwrite(AJ_NVRAM_BASE_ADDRESS, AJ_NVRAM_SIZE, 1, f);
    fclose(f);
    return AJ_OK;
}
