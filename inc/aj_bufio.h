#ifndef _AJ_BUFIO_H
#define _AJ_BUFIO_H
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

/*
 * Forward declaration
 */
struct _AJ_IOBuffer;

/**
 * Function pointer type for an abstracted transmit function
 */
typedef AJ_Status (*AJ_TxFunc)(struct _AJ_IOBuffer* buf);

/**
 * Function pointer type for an abstracted receive function
 *
 * @param buf     The buffer to read into
 * @param len     The requested number of bytes to read. More or fewer bytes may actually be read into
 *                the buffer.
 * @param timeout A timeout in milliseconds after which the read will return with an error status if
 *                there is not data to read.
 *
 * @return
 *         - AJ_OK if some data was read
 *         - AJ_ERR_TIMEOUT if the read timedout
 *         - AJ_ERR_RESOURCES if there isn't enough room in the buffer to read len bytes. The buffer
 *           will contain the bytes actually read so this is not a fatal error.
 *         - AJ_ERR_READ the read failed irrecoverably
 */
typedef AJ_Status (*AJ_RxFunc)(struct _AJ_IOBuffer* buf, uint32_t len, uint32_t timeout);

#define AJ_IO_BUF_RX     1 /**< I/O direction is receive */
#define AJ_IO_BUF_TX     2 /**< I/O direction is send */

/**
 * A type for managing a receive or transmit buffer
 */
typedef struct _AJ_IOBuffer {
    uint8_t direction;  /**< I/O buffer is either a Tx buffer or an Rx buffer */
    uint16_t bufSize;   /**< Size of the data buffer */
    uint8_t* bufStart;  /**< Start for the data buffer */
    uint8_t* readPtr;   /**< Current position in buf for reading data */
    uint8_t* writePtr;  /**< Current position in buf for writing data */
    /*
     * Function pointer to send or recv function
     */
    union {
        AJ_TxFunc send;
        AJ_RxFunc recv;
    };
    void* context;      /**< Abstracted context for managing I/O */

} AJ_IOBuffer;

/**
 * How much data is available to read from the buffer
 */
#define AJ_IO_BUF_AVAIL(iobuf)  (uint32_t)(((iobuf)->writePtr - (iobuf)->readPtr))

/**
 * How much space is available to write to the buffer
 */
#define AJ_IO_BUF_SPACE(iobuf)  ((uint32_t)((iobuf)->bufSize - ((iobuf)->writePtr - (iobuf)->bufStart)))

/**
 * How much data has been consumed from the buffer
 */
#define AJ_IO_BUF_CONSUMED(iobuf)  (uint32_t)(((iobuf)->readPtr - (iobuf)->bufStart))

/**
 * Reset and IO buffer
 */
#define AJ_IO_BUF_RESET(iobuf) \
    do { \
        (iobuf)->readPtr = (iobuf)->bufStart; \
        (iobuf)->writePtr = (iobuf)->bufStart; \
    } while (0)

/**
 * Initialize an I/O Buffer.
 *
 * @param ioBuf     The I/O buffer to initialize
 * @param buffer    The data buffer to use
 * @param bufLen    The size of the data buffer
 * @param direction Indicates if the buffer is being used for sending or receiving data
 * @param context   Abstracted context for managing I/O
 */
void AJ_IOBufInit(AJ_IOBuffer* ioBuf, uint8_t* buffer, uint32_t bufLen, uint8_t direction, void* context);

/**
 * Move any unconsumed data to the start of the buffer.
 *
 * @param ioBuf  An RX I/O buf that may contain unconsumed data
 */
void AJ_IOBufRebase(AJ_IOBuffer* ioBuf);

#endif
