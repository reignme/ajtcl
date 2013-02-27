#ifndef _AJ_NET_H
#define _AJ_NET_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

#include "aj_host.h"
#include "aj_status.h"
#include "aj_bufio.h"

#define AJ_ADDR_IPV4  0x04
#define AJ_ADDR_IPV6  0x60

typedef struct _AJ_NetSocket {
    AJ_IOBuffer tx;
    AJ_IOBuffer rx;
} AJ_NetSocket;

/**
 * Must be called before networking can be used
 */
AJ_Status AJ_Net_Up();

/**
 * Call when the network is not longer needed
 */
void AJ_Net_Down();

/**
 * Connect to bus at an IPV4 or IPV6 address
 */
AJ_Status AJ_Net_Connect(AJ_NetSocket* netSock, uint16_t port, uint8_t addrType, const uint32_t* addr);

/**
 * Disconnect from the bus
 */
void AJ_Net_Disconnect(AJ_NetSocket* netSock);

/**
 * Enable multicast data (for discover and advertising)
 */
AJ_Status AJ_Net_MCastUp(AJ_NetSocket* netSock);

/**
 * Disable multicast data (for discover and advertising)
 */
void AJ_Net_MCastDown(AJ_NetSocket* netSock);

/**
 * Send from an I/O buffer
 */
AJ_Status AJ_Net_Send(AJ_IOBuffer* txBuf);

/**
 * Send into an I/O buffer
 */
AJ_Status AJ_Net_Recv(AJ_IOBuffer* rxBuf, uint32_t len, uint32_t timeout);

#endif