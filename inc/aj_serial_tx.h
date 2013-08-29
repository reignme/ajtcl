#ifndef _AJ_SERIAL_TX_H
#define _AJ_SERIAL_TX_H
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
#ifdef AJ_SERIAL_CONNECTION

#include "aj_target.h"
#include "aj_status.h"
#include "aj_serial.h"


/**
 * This function initializes the serial transport layer.
 */
AJ_Status AJ_SerialTX_Init(void);

/**
 * This function resets the transmit side of the H:5 transport.
 */
AJ_Status AJ_SerialTX_Reset(void);

/**
 * This function shuts down the serial transport layer.
 */
void AJ_SerialTX_Shutdown(void);


/**
 * This function adds a control packet that contains H:5 control information (link control or ACK)
 * to the transmit queue. This function assumes that the packet is statically allocated or is NULL.
 */
void AJ_SerialTX_EnqueueCtrl(const uint8_t* packet,
                             uint16_t pktLen,
                             uint8_t type);

/**
 * This function adds a packet that contains Bluetooth data, HCI command,
 * ACL data, or SCO data to the transmit queue.
 */
void AJ_SerialTX_EnqueuePkt(uint8_t type,
                            uint16_t header,
                            uint16_t length);

/**
 * This function is called by the receive layer when a data packet or an explicit ACK
 * has been received. The ACK value is one greater (modulo 8) than the seq number of the
 * last packet successfully received.
 */
void AJ_SerialTX_ReceivedAck(uint8_t ack);


/**
 * This function is called from the receive side with the sequence number of
 * the last packet received.
 */
void AJ_SerialTX_ReceivedSeq(uint8_t seq);

/**
 * This function is called from the state machine to resend any data packets
 * that have not yet been acked.
 */
void ResendPackets();

/**
 * This function is called from the state machine to ack a data packet recieved
 * in case this end doesnt have data to send.
 */
void SendAck();

/**
 * There is space available in the transmit queue handled by the transmit callback,
 * queue up packets if there are any.
 */
void AJ_FillTxBufferList();

#endif /* AJ_SERIAL_CONNECTION */
#endif /* _AJ_SERIAL_TX_H */
