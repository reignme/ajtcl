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
#define DEFAULT_ENTRY_BUF_SIZE 64
#define INVALID_ID 0
#define INVALID_DATA 0xFFFF
#define INVALID_DATA_BYTE 0xFF
#define SENTINEL_OFFSET 4
#define ENTRY_HEADER_SIZE 4
#define WORD_ALIGN(x) ((x & 0x3) ? ((x >> 2) + 1) << 2 : x)

#define AJ_NVRAM_SIZE 512

/**
 * Invalidate an entry (data set) in NVRAM by setting the id to be 0
 *
 * @param inode  Address of an entry in the NVRAM
 */
void AJ_InvalidateNVEntry(uint16_t* inode);

/**
 * Append an entry (data set) to NVRAM
 *
 * @param nvPtr   The address to append the entry in NVRAM
 * @param handle  Handle that specified a data set
 */
void AJ_AppendNVEntry(uint8_t* nvPtr, AJ_NV_FILE* handle);

/**
 * Erase the whole NVRAM sector and write the sentinel data
 */
void AJ_EraseNVRAM();

/**
 * Erase the NVRAM and update it with data in the memory buffer
 *
 * @param bufPtr Pointer to a buffer containing data for writing to the NVRAM
 * @param bytes  The number of bytes of data in the buffer
 */
void AJ_OverriteNVRAM(uint8_t* bufPtr, uint16_t bytes);

/**
 * Load NVRAM data from a file
 */
AJ_Status AJ_LoadNVFromFile();

/**
 * Write NVRAM data to a file for persistent storage
 */
AJ_Status AJ_StoreNVToFile();

#endif