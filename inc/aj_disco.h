#ifndef _AJ_DISCO_H
#define _AJ_DISCO_H
/**
 * @file
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
#include "aj_bufio.h"

/**
 * Information about the remote service
 */
typedef struct _AJ_Service {
    uint8_t addrTypes;         /**< address type */
    uint16_t transportMask;    /**< restricts the transports the advertisement */
    uint16_t ipv4port;         /**< port number of ipv4 */
    uint16_t ipv6port;         /**< port number of ipv6 */
    uint32_t ipv4;             /**< ipv4 address */
    uint32_t ipv6[4];          /**< ipv6 address */
} AJ_Service;

/**
 * Discover a remote service
 *
 * @param prefix    The service name prefix
 * @param service   Information about the service that was found
 * @param timeout   How long to wait to discover the service
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_Discover(const char* prefix, AJ_Service* service, uint32_t timeout);

#endif
