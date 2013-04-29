#ifndef _AJ_HELPER_H
#define _AJ_HELPER_H
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

#include "aj_status.h"
#include "aj_bus.h"

#define AJ_JOINSESSION_REPLY_SUCCESS              1   /**< JoinSession reply: Success */
#define AJ_JOINSESSION_REPLY_NO_SESSION           2   /**< JoinSession reply: Session with given name does not exist */
#define AJ_JOINSESSION_REPLY_UNREACHABLE          3   /**< JoinSession reply: Failed to find suitable transport */
#define AJ_JOINSESSION_REPLY_CONNECT_FAILED       4   /**< JoinSession reply: Connect to advertised address */
#define AJ_JOINSESSION_REPLY_REJECTED             5   /**< JoinSession reply: The session creator rejected the join req */
#define AJ_JOINSESSION_REPLY_BAD_SESSION_OPTS     6   /**< JoinSession reply: Failed due to session option incompatibilities */
#define AJ_JOINSESSION_REPLY_ALREADY_JOINED       7   /**< JoinSession reply: Caller has already joined this session */
#define AJ_JOINSESSION_REPLY_FAILED              10   /**< JoinSession reply: Failed for unknown reason */

/**
 * Helper function that connects to a bus initializes an AllJoyn service.
 *
 * @param bus          The bus attachment
 * @param daemonName   Name of a specific daemon service to connect to, NULL for the default name.
 * @param timeout      How long to spend attempting to connect to the bus
 * @param port         The port to bind
 * @param name         The name being requested
 * @param flags        An OR of the name request flags
 * @param opts         The session option setting.
 *
 * @return AJ_OK if service was successfully started.
 */
AJ_Status AJ_StartService(AJ_BusAttachment* bus,
                          const char* daemonName,
                          uint32_t timeout,
                          uint16_t port,
                          const char* name,
                          uint32_t flags,
                          const AJ_SessionOpts* opts);


/**
 * Helper function that connects to a bus initializes an AllJoyn service.
 *
 * @param bus          The bus attachment
 * @param daemonName   Name of a specific daemon service to connect to, NULL for the default name.
 * @param timeout      How long to spend attempting to connect to the bus
 * @param connected    Whether the bus attachment is already connected to the daemon bus
 * @param port         The port to bind
 * @param name         The name being requested
 * @param flags        An OR of the name request flags
 * @param opts         The session option setting.
 *
 * @return AJ_OK if service was successfully started.
 */
AJ_Status AJ_StartService2(AJ_BusAttachment* bus,
                           const char* daemonName,
                           uint32_t timeout,
                           uint8_t connected,
                           uint16_t port,
                           const char* name,
                           uint32_t flags,
                           const AJ_SessionOpts* opts);

/**
 * Initializes an AllJoyn client and connect to a service
 *
 * @param bus            The bus attachment
 * @param daemonName     Name of a specific daemon service to connect to, NULL for the default name.
 * @param timeout        How long to spend attempting to find a remote service to connect to.
 * @param name           The name of the service to connect to.
 * @param port           The service port to connect to.
 * @param[out] sessionId The session id returned if the service connection was succesfully
 * @param opts           The session option setting.
 *
 * @return AJ_OK if connection was successfully established
 */
AJ_Status AJ_StartClient(AJ_BusAttachment* bus,
                         const char* daemonName,
                         uint32_t timeout,
                         const char* name,
                         uint16_t port,
                         uint32_t* sessionId,
                         const AJ_SessionOpts* opts);

/**
 * Initializes an AllJoyn client and connect to a service
 *
 * @param bus            The bus attachment
 * @param daemonName     Name of a specific daemon service to connect to, NULL for the default name.
 * @param timeout        How long to spend attempting to find a remote service to connect to.
 * @param connected      Whether the bus attachment is already connected to the daemon bus
 * @param name           The name of the service to connect to.
 * @param port           The service port to connect to.
 * @param[out] sessionId The session id returned if the service connection was succesfully
 * @param opts           The session option setting.
 *
 * @return AJ_OK if connection was successfully established
 */
AJ_Status AJ_StartClient2(AJ_BusAttachment* bus,
                          const char* daemonName,
                          uint32_t timeout,
                          uint8_t connected,
                          const char* name,
                          uint16_t port,
                          uint32_t* sessionId,
                          const AJ_SessionOpts* opts);



#endif /* _AJ_HELPER_H */
