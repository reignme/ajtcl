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

extern uint8_t* AJ_NVRAM_BASE_ADDRESS;

#define AJ_NV_FILE_RD_ONLY         1
#define AJ_NV_FILE_WR_ONLY         2
#define AJ_NV_FILE_APPEND          3
#define AJ_NV_FILE_RD_UPDATE       4
#define AJ_NV_FILE_WR_UPDATE       5
#define AJ_NV_FILE_APPEND_UPDATE   6
#define AJ_NVRAM_END_ADDRESS (AJ_NVRAM_BASE_ADDRESS + AJ_NVRAM_SIZE)

void AJ_NVRAM_Layout_Print()
{
    int i = 0;
    uint16_t* data = (uint16_t*)(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET);
    uint16_t entryId = 0;
    uint16_t len = 0;
    AJ_Printf("============ AJ NVRAM Map ===========\n");
    for (i = 0; i < SENTINEL_OFFSET; i++) {
        AJ_Printf("%c", *((uint8_t*)(AJ_NVRAM_BASE_ADDRESS + i)));
    }
    AJ_Printf("\n");

    while ((uint8_t*)data < (uint8_t*)AJ_NVRAM_END_ADDRESS && *data != INVALID_DATA) {
        entryId = *data;
        len = *(data + 1);
        AJ_Printf("ID = %d, Data Length = %d\n", entryId, len);
        data += (ENTRY_HEADER_SIZE + WORD_ALIGN(len)) >> 1;
    }
    AJ_Printf("============ End ===========\n");
}

/**
 * Compactize the NVRAM storage by removing invalid entries whose id is 0
 */
AJ_Status AJ_CompactNVStorage();

/**
 * Find an entry in the NVRAM with the specific id
 *
 * @return Pointer pointing to an entry in the NVRAM if an entry with the specified id is found
 *         NULL otherwise
 */
uint16_t* AJ_FindNVEntry(uint16_t id) {
    uint16_t len = 0;
    uint16_t* data = (uint16_t*)(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET);
    while ((uint8_t*)data < (uint8_t*)AJ_NVRAM_END_ADDRESS) {
        if (*data != id) {
            len = *(data + 1);
            if (*data == INVALID_DATA) {
                break;
            }
            data += (ENTRY_HEADER_SIZE + WORD_ALIGN(len)) >> 1;
        } else {
            return data;
        }
    }
    return NULL;
}

void AJ_StoreDataSet(AJ_NV_FILE* handle)
{
    uint8_t* ptr = NULL;
    if (handle->inode) {
        AJ_InvalidateNVEntry(handle->inode);
        handle->inode = NULL;
    }
    ptr = (uint8_t*)AJ_FindNVEntry(INVALID_DATA);
    if (!ptr || ptr + ENTRY_HEADER_SIZE + handle->dataLen > (uint8_t*)AJ_NVRAM_END_ADDRESS) {
        AJ_Printf("Do storage compaction...\n");
        AJ_CompactNVStorage();

        ptr = (uint8_t*)AJ_FindNVEntry(INVALID_DATA);
        if (!ptr || ptr + ENTRY_HEADER_SIZE + handle->dataLen > (uint8_t*)AJ_NVRAM_END_ADDRESS) {
            AJ_Printf("Error: Do not have enough storage space.\n");
            return;
        }
    }

    AJ_AppendNVEntry(ptr, handle);
}

// Compact the storage by removing invalid entries
AJ_Status AJ_CompactNVStorage()
{
    uint16_t numOfValidData = 0;
    uint16_t len = 0;
    uint16_t id = 0;
    uint16_t* data = (uint16_t*)(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET);
    uint16_t curPos = 0;
    uint8_t* bufPtr = NULL;
    uint8_t compact = FALSE;

    // calculate buffer size needed
    while ((uint8_t*)data < (uint8_t*)AJ_NVRAM_END_ADDRESS && *data != INVALID_DATA) {
        id = *data;
        len = *(data + 1);
        if (id != INVALID_ID) {
            numOfValidData += (ENTRY_HEADER_SIZE + WORD_ALIGN(len));
        } else {
            compact = TRUE;
        }
        data += (ENTRY_HEADER_SIZE + WORD_ALIGN(len)) >> 1;
    }

    // nothing needs to be done for compaction
    if (!compact) {
        return AJ_OK;
    }

    if (numOfValidData > 0) {
        bufPtr = (uint8_t*)AJ_Malloc(numOfValidData);
        if (!bufPtr) {
            AJ_Printf("Err: Out of memory \n");
            return AJ_ERR_RESOURCES;
        }

        data = (uint16_t*)(AJ_NVRAM_BASE_ADDRESS + SENTINEL_OFFSET);
        while ((uint8_t*)data < (uint8_t*)AJ_NVRAM_END_ADDRESS && *data != INVALID_DATA) {
            len = *(data + 1);
            if (*data != INVALID_ID) {
                memcpy(bufPtr + curPos, data, ENTRY_HEADER_SIZE + WORD_ALIGN(len));
                curPos += (ENTRY_HEADER_SIZE + WORD_ALIGN(len));
            }
            data += (ENTRY_HEADER_SIZE + WORD_ALIGN(len)) >> 1;
        }
    }

    // Update the NVRAM with the data in the buffer
    AJ_OverriteNVRAM(bufPtr, numOfValidData);

    if (numOfValidData > 0) {
        AJ_Free(bufPtr);
    }

    return AJ_OK;
}

// Allocate an handle to manage a data set. A data buffer is also allocated
AJ_NV_FILE* AJ_AllocNVHandle(uint16_t id, uint16_t minBufSize)
{
    uint32_t allocBufSize = (minBufSize > DEFAULT_ENTRY_BUF_SIZE) ? minBufSize : DEFAULT_ENTRY_BUF_SIZE;
    AJ_NV_FILE* handle = (AJ_NV_FILE*)AJ_Malloc(sizeof(AJ_NV_FILE));
    if (!handle) {
        AJ_Printf("AJ_AllocNVHandle() error: OutOfMemory. \n");
        return NULL;
    }

    handle->buf = (uint8_t*)AJ_Malloc(allocBufSize);
    if (!handle->buf) {
        AJ_Free(handle);
        AJ_Printf("AJ_AllocNVHandle() error: OutOfMemory. \n");
        return NULL;
    }

    handle->dataLen = 0;
    handle->bufSize = allocBufSize;
    handle->id = id;
    handle->writeBack = FALSE;
    handle->curPos = 0;
    handle->inode = NULL;
    return handle;
}

AJ_NV_FILE* AJ_NVRAM_Open(uint16_t id, char* mode)
{
    uint16_t* entry = NULL;
    AJ_NV_FILE* handle = NULL;
    uint8_t shouldExist = FALSE;
    uint8_t append = FALSE;
    uint8_t emptify = FALSE;
    uint8_t access = 0;
    uint16_t minBufSize = 0;

    if (!id) {
        AJ_Printf("Error: A valide id must not be 0.\n");
        return NULL;
    }

    if (0 == strcmp(mode, "r")) {
        access = AJ_NV_FILE_RD_ONLY;
        shouldExist = TRUE;
    } else if (0 == strcmp(mode, "w")) {
        access = AJ_NV_FILE_WR_ONLY;
        emptify = TRUE;
    } else if (0 == strcmp(mode, "a")) {
        access = AJ_NV_FILE_APPEND;
        append = TRUE;
    } else if (0 == strcmp(mode, "r+")) {
        access = AJ_NV_FILE_RD_UPDATE;
        shouldExist = TRUE;
    } else if (0 == strcmp(mode, "w+")) {
        access = AJ_NV_FILE_WR_UPDATE;
        emptify = TRUE;
    } else if (0 == strcmp(mode, "a+")) {
        access = AJ_NV_FILE_APPEND_UPDATE;
        append = TRUE;
    } else {
        AJ_Printf("Error: Unrecognized access mode %s\n", mode);
        return NULL;
    }

    entry = AJ_FindNVEntry(id);
    if (!entry && shouldExist) {
        AJ_Printf("Error: access mode %s requires the data set to exist\n", mode);
        return NULL;
    }

    if (entry && !emptify) {
        minBufSize = *(entry + 1);
        if (access != AJ_NV_FILE_RD_ONLY) {
            minBufSize += minBufSize >> 1; // enlarge the buffer by 50% in case write operation needs more space
        }
    }
    handle = AJ_AllocNVHandle(id, minBufSize);
    if (!handle) {
        return NULL;
    }

    handle->mode = access;
    if (entry && !emptify) {
        memcpy(handle->buf, (uint8_t*)entry + ENTRY_HEADER_SIZE, *(entry + 1));
        handle->dataLen = *(entry + 1);
        handle->inode = entry;
    }
    if (append) {
        handle->curPos = handle->dataLen;
    }
    if (entry && emptify) {
        AJ_InvalidateNVEntry(entry);
        handle->inode = NULL;
    }
    return handle;
}

size_t AJ_NVRAM_Write(void* ptr, size_t size, AJ_NV_FILE* handle)
{
    uint16_t bytesWrite = 0;
    uint8_t* srcBuf = NULL;
    if (!handle || handle->mode == AJ_NV_FILE_RD_ONLY) {
        AJ_Printf("AJ_NVRAM_Write() error: The access mode does not allow to write.\n");
        return -1;
    }

    // If necessary, try to allocate a buffer of of bigger size * 1.5
    if (handle->curPos + size > handle->bufSize) {
        uint8_t* newBuf = (uint8_t*)AJ_Malloc(handle->bufSize + (handle->bufSize >> 1));
        if (newBuf) {
            memcpy(newBuf, handle->buf, handle->dataLen);
            AJ_Free(handle->buf);
            handle->buf = newBuf;
            handle->bufSize += handle->bufSize >> 1;
        }
    }

    srcBuf = (uint8_t*)ptr;
    bytesWrite = (handle->bufSize - handle->curPos) > size ? size : (handle->bufSize - handle->curPos);
    if (bytesWrite > 0) {
        memcpy(handle->buf + handle->curPos, srcBuf, bytesWrite);
        handle->curPos += (uint16_t)bytesWrite;
        handle->dataLen = (handle->dataLen > handle->curPos) ? handle->dataLen  : handle->curPos;
        handle->writeBack = TRUE;
    }
    return bytesWrite;
}

size_t AJ_NVRAM_Read(void* ptr, size_t size, AJ_NV_FILE* handle)
{
    uint8_t* destBuf = (uint8_t*)ptr;
    size_t bytesRead = 0;
    if (!handle || handle->mode == AJ_NV_FILE_WR_ONLY || handle->mode == AJ_NV_FILE_APPEND) {
        AJ_Printf("AJ_NVRAM_Read() error: The access mode does not allow to read.\n");
        return -1;
    }

    if (handle->dataLen == handle->curPos) {
        // EOF, no more data to be read
        return -1;
    }
    bytesRead = (handle->dataLen - handle->curPos) > size ? size : (handle->dataLen - handle->curPos);
    if (bytesRead > 0) {
        memcpy(destBuf, handle->buf + handle->curPos, bytesRead);
        handle->curPos += (uint16_t)bytesRead;
    }
    return bytesRead;
}

AJ_Status AJ_NVRAM_Close(AJ_NV_FILE* handle)
{
    if (!handle) {
        AJ_Printf("AJ_NVRAM_Close() error: Invalid handle. \n");
        return AJ_ERR_INVALID;
    }

    if (handle->writeBack) {
        AJ_StoreDataSet(handle);
    }
    AJ_Free(handle->buf);
    AJ_Free(handle);
    handle = NULL;
    return AJ_OK;
}

size_t AJ_NVRAM_Size(AJ_NV_FILE* handle)
{
    if (!handle) {
        AJ_Printf("AJ_NVRAM_Size() error: Invalid handle. \n");
        return -1;
    }

    return handle->dataLen;
}

uint8_t AJ_NVRAM_Exist(uint16_t id)
{
    if (!id) {
        return FALSE; // the unique id is not allowed to be 0
    }

    return (NULL != AJ_FindNVEntry(id));
}
