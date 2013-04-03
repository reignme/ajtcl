#ifndef _AJ_HELPER_H
#define _AJ_HELPER_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

#include "aj_status.h"
#include "aj_bus.h"

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
