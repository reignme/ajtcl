#ifndef _AJ_TARGET_NVRAM_H_
#define _AJ_TARGET_NVRAM_H_

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

#define AJ_NVRAM_SIZE 512
uint8_t AJ_NVRAM_BASE_ADDRESS[AJ_NVRAM_SIZE];

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