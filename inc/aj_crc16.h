#ifndef _AJ_CRC16_H
#define _AJ_CRC16_H
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

#include "aj_target.h"


/**
 * Computes a 16-bit CRC on a buffer. The caller provides the context for the running CRC.
 *
 * @param buffer         buffer over which to compute the CRC
 * @param bufLen         length of the buffer in bytes
 * @param runningCrc     On input the current CRC, on output the updated CRC.
 */
void AJ_CRC16_Compute(const uint8_t* buffer,
                      uint16_t bufLen,
                      uint16_t* runningCrc);

/**
 * This function completes the CRC computation by rearranging the CRC bits and bytes
 * into the correct order.
 *
 * @param crc       computed crc as calculated by AJ_CRC16_Compute()
 * @param crcBlock  pointer to a 2-byte buffer where the resulting CRC will be stored
 */

void AJ_CRC16_Complete(uint16_t crc,
                       uint8_t* crcBlock);

#endif /* _AJ_CRC16_H */
