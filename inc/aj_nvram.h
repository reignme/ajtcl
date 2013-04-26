#ifndef _AJ_NVRAM_H_
#define _AJ_NVRAM_H_

/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include "alljoyn.h"

typedef struct _AJ_NV_FILE {
    uint16_t id;           /* The unique id of a data set */
    uint8_t mode;          /* The access mode of a data set */
    uint8_t writeBack;     /* If need to write back to the storage */
    uint8_t* buf;          /* A buffer for read/write operations to a data set */
    uint16_t dataLen;      /* The number of data available in a data set */
    uint16_t bufSize;      /* The buffer size for read/write operations to a data set */
    uint16_t curPos;       /* The current position for read/write operations to a data set */
    uint16_t* inode;       /* Point to a location in the NVRAM storage where the data set lives */
} AJ_NV_FILE;

/**
 * Open a data set
 *
 * @param id  A unique id for a data set. The value must not be 0.
 * @param mode C string containing a data set access mode. It can be:
 *    "r"  : read: Open data set for input operations. The data set must exist.
 *    "w"  : write: Create an empty data set for output operations. If a data set with the same id already exists, its contents are discarded.
 *    "a"  : append: Open data set for output at the end of a data set. The data set is created if it does not exist.
 *    "r+" : read/update: Open a data set for update (both for input and output). The data set must exist.
 *    "w+" : write/update: Create an empty data set and open it for update (both for input and output). If a data set with the same id already exists its contents are discarded.
 *    "a+" : append/update: Open a data set for update (both for input and output) with all output operations writing data at the end of the data set. The data set is created if it does not exist.
 *
 * @return A handle that specifies the data set. NULL if the open operation fails.
 */
AJ_NV_FILE* AJ_NVRAM_Open(uint16_t id, char* mode);

/**
 * Write to the data set specified by a handle
 *
 * @param ptr Pointer to a block of memory with a size of at least size bytes to be written to NVRAM.
 * @param size Size, in bytes, to be written. size_t is an unsigned integral type.
 * @param handle Pointer to a AJ_NV_FILE object that specifies a data set.
 *
 * @return The number of byte of data written to the data set
 */
size_t AJ_NVRAM_Write(void* ptr, size_t size, AJ_NV_FILE* handle);

/**
 * Read from the data set specified by a handle
 *
 * @param ptr Pointer to a block of memory with a size of at least size bytes to be read from NVRAM.
 * @param size Size, in bytes, to be read. size_t is an unsigned integral type.
 * @param handle Pointer to a AJ_NV_FILE object that specifies a data set.
 *
 * @return The number of byte of data read from the data set if there are data available, otherwise -1(End-of-file).
 */
size_t AJ_NVRAM_Read(void* ptr, size_t size, AJ_NV_FILE* handle);

/**
 * Commit changes to the data set specified by a handle to NVRAM storage and close the data set
 *
 * @param handle Pointer to a AJ_NV_FILE object that specifies a data set.
 *
 * @return AJ_ERR_INVALID if the handle is invalid, otherwise AJ_OK.
 */
AJ_Status AJ_NVRAM_Close(AJ_NV_FILE* handle);

/**
 * Get the data size of the data set specified by a handle
 *
 * @param handle Pointer to a AJ_NV_FILE object that specifies a data set.
 *
 * @return The number of byte of data available in the data set. -1 if the handle is NULL.
 */
size_t AJ_NVRAM_Size(AJ_NV_FILE* handle);

/**
 * Check if a data set with a unique id exists
 *
 * @param id A unique ID for a data set. A valid id must not be 0.
 *
 * @return 1 if a data set with the specified id exists
 *         0 if not.
 */
uint8_t AJ_NVRAM_Exist(uint16_t id);

#endif

