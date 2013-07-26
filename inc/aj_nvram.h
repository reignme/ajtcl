#ifndef _AJ_NVRAM_H_
#define _AJ_NVRAM_H_

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

#define AJ_NVRAM_ID_CREDS_MAX        0x0FFF   /**< Last NVRAM ID reserved for AllJoyn credentials management */
#define AJ_NVARM_ID_RESERVED_MAX     0x7FFF   /**< Last NVRAM ID reserved for AllJoyn framework and services use */
#define AJ_NVRAM_ID_FOR_APPS         0x8000   /**< First NVRAM ID available for application used */

#define AJ_NV_DATASET_MODE_READ      'r'      /**< Data set is in read mode */
#define AJ_NV_DATASET_MODE_WRITE     'w'      /**< Data set is in write mode */

/**
 * AllJoyn NVRAM dataset handle
 */
typedef struct _AJ_NV_DATASET {
    uint8_t mode;          /**< The access mode (read or write) of a data set */
    uint16_t curPos;       /**< The current read/write offset of a data set */
    uint16_t capacity;     /**< The capacity of the data set established by AJ_NVRAM_Open() */
    uint16_t id;           /**< The unique id of a data set */
    uint8_t* inode;        /**< Pointer or offset to a location of the data set in the NVRAM */
} AJ_NV_DATASET;

/**
 * Initialize NVRAM
 */
void AJ_NVRAM_Init();

/**
 * Open a data set
 *
 * @param id  A unique id for a data set. The value must not be 0.
 * @param mode C string containing a data set access mode. It can be:
 *    "r"  : read: Open data set for input operations. The data set must exist.
 *    "w"  : write: Create an empty data set for output operations. If a data set with the same id already exists, its contents are discarded.
 * @param capacity The reserved space size for the data set. Only used for "w" access mode.
 *
 * @return A handle that specifies the data set. NULL if the open operation fails.
 */
AJ_NV_DATASET* AJ_NVRAM_Open(uint16_t id, char* mode, uint16_t capacity);

/**
 * Write to the data set specified by a handle
 *
 * @param ptr   Pointer to a block of memory with a size of at least size bytes to be written to NVRAM.
 * @param size  Size, in bytes, to be written.
 * @param handle Pointer to an AJ_NV_DATASET object that specifies a data set.
 *
 * @return The number of byte of data written to the data set
 *         -1 if the offset is out of the bound of the data set
 */
size_t AJ_NVRAM_Write(void* ptr, uint16_t size, AJ_NV_DATASET* handle);

/**
 * Read from the data set specified by a handle
 *
 * @param ptr   Pointer to a block of memory with a size of at least size bytes to be read from NVRAM.
 * @param size  Size, in bytes, to be read.
 * @param handle Pointer to an AJ_NV_DATASET object that specifies a data set.
 *
 * @return The number of byte of data read from the data set.
 *         -1 if the offset is out of the bound of the data set
 */
size_t AJ_NVRAM_Read(void* ptr, uint16_t size, AJ_NV_DATASET* handle);

/**
 * Close the data set and release the handle
 *
 * @param handle Pointer to an AJ_NV_DATASET object that specifies a data set.
 *
 * @return AJ_ERR_INVALID if the handle is invalid, otherwise AJ_OK.
 */
AJ_Status AJ_NVRAM_Close(AJ_NV_DATASET* handle);

/**
 * Check if a data set with a unique id exists
 *
 * @param id A unique ID for a data set. A valid id must not be 0.
 *
 * @return 1 if a data set with the specified id exists
 *         0 if not.
 */
uint8_t AJ_NVRAM_Exist(uint16_t id);

/**
 * Delete a data set specified by the id
 *
 * @param id A unique id for a data set.
 *
 * @return AJ_OK if the data set is deleted successfully
 *         AJ_ERR_FAILURE if the data set does not exist.
 */
AJ_Status AJ_NVRAM_Delete(uint16_t id);

#endif

