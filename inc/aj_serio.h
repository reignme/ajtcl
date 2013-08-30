#ifndef _AJ_SERIO_H
#define _AJ_SERIO_H
/**
 * @file aj_serio.h
 * @defgroup aj_serio Serial Input/Output
 * @{
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

#define AJ_SERIO_RX     1 /**< The receive direction (from the wire) of the serial I/O subsystem */
#define AJ_SERIO_TX     2 /**< The transmit direction (to the wire) of the serial I/O subsystem */

/**
 * A type for managing serial port configuration
 */
typedef struct _AJ_SerIOConfig {
    uint32_t bitrate;   /**< bitrate of the port */
    uint8_t  bits;      /**< The number of data bits */
    uint8_t stopBits;  /**< The number of stop bits */
    uint8_t parity;    /**< Zero disables parity checking, one means odd and two means even parity */
    void*    config;    /**< Abstracted context for device/platform specific configuration items */
} AJ_SerIOConfig;

/**
 * @brief Enable or disable the Serial I/O Subsystem to send or receive frames
 *
 * The serial I/O subsystem will not begin sending or receiving data until it
 * is explicitly told to.  This allows for orderly bring-up and shutdown of the
 * system.
 *
 * @param direction  Provide AJ_SERIO_RX for the receiver section, AJ_SERIO_TX for the transmitter.
 * @param enable     Provide true to enable the section, false to disable.
 *
 * @return
 *       - AJ_OK     If the serial operation completed successfully
 */
AJ_Status AJ_SerialIOEnable(uint32_t direction, uint8_t enable);

/**
 * @brief Shutdown the Serial I/O Subsystem
 *
 * Disable the serial I/O subsystem and return all buffers to the client with
 * an appropriated error code.  An implied disable of the transmit and receive
 * side is done during shutdown.
 */
AJ_Status AJ_SerialIOShutdown(void);

/**
 * Function pointer type for an abstracted receive callback function
 *
 * @param buf     The buffer that has a new frame read into it
 * @param len     The number of bytes actually read
 */
typedef void (*AJ_SerIORxCompleteFunc)(uint8_t* buf, uint16_t len);

/**
 * Function pointer type for an abstracted transmit callback function
 *
 * @param buf     The buffer that has had its bytes transmitted out
 * @param len     The number of bytes actually written
 */
typedef void (*AJ_SerIOTxCompleteFunc)(uint8_t* buf, uint16_t len);

/**
 * @brief Initialize the Serial I/O Subsystem
 *
 * Given a serial IO config structure as a guide, initialize the serial I/O
 * subsystem. Not all combinations of bitrate, data bits and stopbits will be
 * legal.  Limitations due to the configurability apply.
 *
 * @param config  The configuration to use.  NULL implies use a default setting.
 * @param rx_cb   The function to call when a complete SLIP frame is received
 * @param tx_cb   The function to call when a frame has been completely transmitted
 *
 * @return
 *          - AJ_OK              if the serial subsystem was successfully initialized
 *          - AJ_ERR_UNEXPECTED  if one of the parameters is not supported
 */
AJ_Status AJ_SerialIOInit(AJ_SerIOConfig* config);

void AJ_SetRxCB(AJ_SerIORxCompleteFunc rx_cb);
void AJ_SetTxCB(AJ_SerIOTxCompleteFunc tx_cb);

void AJ_RX(uint8_t* buf, uint32_t len);
void AJ_PauseRX();
void AJ_ResumeRX();

void AJ_TX(uint8_t* buf, uint32_t len);
void AJ_PauseTX();
void AJ_ResumeTX();

#endif /* _AJ_SERIO_H */
