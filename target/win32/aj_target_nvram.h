#ifndef _AJ_TARGET_NVRAM_H_
#define _AJ_TARGET_NVRAM_H_

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
#include "alljoyn.h"

/*
 * Identifies an AJ NVRAM block
 */
#define AJ_NV_SENTINEL ('A' | ('J' << 8) | ('N' << 16) | ('V' << 24))
#define INVALID_ID (0)
#define INVALID_DATA (0xFFFF)
#define INVALID_DATA_BYTE (0xFF)
#define SENTINEL_OFFSET (4)
#define WORD_ALIGN(x) ((x & 0x3) ? ((x >> 2) + 1) << 2 : x)
#define AJ_NVRAM_SIZE (2024)

typedef struct _NV_EntryHeader {
    uint16_t id;           /**< The unique id */
    uint16_t capacity;     /**< The data set size */
} NV_EntryHeader;

#define ENTRY_HEADER_SIZE (sizeof(NV_EntryHeader))
#define AJ_NVRAM_END_ADDRESS (AJ_NVRAM_BASE_ADDRESS + AJ_NVRAM_SIZE)

/**
 * Write a block of data to NVRAM
 *
 * @param dest  Pointer a location of NVRAM
 * @param buf   Pointer to data to be written
 * @param size  The number of bytes to be written
 */
void _AJ_NV_Write(void* dest, void* buf, uint16_t size);

/**
 * Read a block of data from NVRAM
 *
 * @param src   Pointer a location of NVRAM
 * @param buf   Pointer to data to be written
 * @param size  The number of bytes to be written
 */
void _AJ_NV_Read(void* src, void* buf, uint16_t size);


/**
 * Erase the whole NVRAM sector and write the sentinel data
 */
void _AJ_EraseNVRAM();

/**
 * Load NVRAM data from a file
 */
AJ_Status _AJ_LoadNVFromFile();

/**
 * Write NVRAM data to a file for persistent storage
 */
AJ_Status _AJ_StoreNVToFile();

#endif
