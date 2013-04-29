#ifndef _AJ_NET_H
#define _AJ_NET_H
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
#include "aj_status.h"
#include "aj_bufio.h"

#define AJ_ADDR_IPV4  0x04      /**< ip4 address */
#define AJ_ADDR_IPV6  0x60      /**< ip6 address */

/**
 * Abstracts a network socket
 */
typedef struct _AJ_NetSocket {
    AJ_IOBuffer tx;             /**< transmit network socket */
    AJ_IOBuffer rx;             /**< receive network socket */
} AJ_NetSocket;

/**
 * Must be called before networking can be used
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Net_Up();

/**
 * Call when the network is not longer needed
 *
 * @return        Return AJ_Status
 */
void AJ_Net_Down();

/**
 * Connect to bus at an IPV4 or IPV6 address
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Net_Connect(AJ_NetSocket* netSock, uint16_t port, uint8_t addrType, const uint32_t* addr);

/**
 * Disconnect from the bus
 */
void AJ_Net_Disconnect(AJ_NetSocket* netSock);

/**
 * Enable multicast data (for discover and advertising)
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Net_MCastUp(AJ_NetSocket* netSock);

/**
 * Disable multicast data (for discover and advertising)
 */
void AJ_Net_MCastDown(AJ_NetSocket* netSock);

/**
 * Send from an I/O buffer
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Net_Send(AJ_IOBuffer* txBuf);

/**
 * Send into an I/O buffer
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Net_Recv(AJ_IOBuffer* rxBuf, uint32_t len, uint32_t timeout);

#endif
