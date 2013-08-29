#ifndef _AJ_SERIAL_RX_H
#define _AJ_SERIAL_RX_H
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
 * This function initializes the receive path
 */
AJ_Status AJ_SerialRX_Init(void);

/**
 * This function shuts down the receive path
 */
void AJ_SerialRX_Shutdown(void);

/**
 * This function resets the receive path
 */
AJ_Status AJ_SerialRX_Reset(void);

/**
 * Process the buffers read by the Receive callback - called by the StateMachine.
 */
void AJ_ProcessRxBufferList();

#endif /* AJ_SERIAL_CONNECTION */
#endif /* _AJ_SERIAL_RX_H */
