#ifndef _AJ_STATUS_H
#define _AJ_STATUS_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

/**
 * Status codes
 */
typedef enum {

    AJ_OK               = 0,  /**< Success status */
    AJ_ERR_NULL         = 1,  /**< Unexpected NULL pointer */
    AJ_ERR_UNEXPECTED   = 2,  /**< An operation was unexpected at this time */
    AJ_ERR_INVALID      = 3,  /**< A value was invalid */
    AJ_ERR_IO_BUFFER    = 4,  /**< An I/O buffer was invalid or in the wrong state */
    AJ_ERR_READ         = 5,  /**< An error while reading data from the network */
    AJ_ERR_WRITE        = 6,  /**< An error while writing data to the network */
    AJ_ERR_TIMEOUT      = 7,  /**< A timeout occurred */
    AJ_ERR_MARSHAL      = 8,  /**< Marshaling failed due to badly constructed message argument */
    AJ_ERR_UNMARSHAL    = 9,  /**< Unmarshaling failed due to a corrupt or invalid message */
    AJ_ERR_END_OF_DATA  = 10, /**< Not enough data */
    AJ_ERR_RESOURCES    = 11, /**< Insufficient memory to perform the operation */
    AJ_ERR_NO_MORE      = 12, /**< Attempt to unmarshal off the end of an array */
    AJ_ERR_SECURITY     = 13, /**< Authentication or decryption failed */
    AJ_ERR_CONNECT      = 14, /**< Network connect failed */
    AJ_ERR_UNKNOWN      = 15, /**< A unknown value */
    AJ_ERR_NO_MATCH     = 16, /**< Something didn't match */
    AJ_ERR_SIGNATURE    = 17, /**< Signature is not what was expected */
    AJ_ERR_DISALLOWED   = 18, /**< An operation was not allowed */
    AJ_ERR_FAILURE      = 19, /**< A failure has occurred */
    AJ_ERR_RESTART      = 20, /**< The OEM event loop must restart */
    AJ_ERR_LINK_TIMEOUT = 21  /**< The bus link is inactive too long */

} AJ_Status;

#endif
