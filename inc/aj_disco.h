#ifndef _AJ_DISCO_H
#define _AJ_DISCO_H
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
#include "aj_bufio.h"

typedef struct _AJ_Service {
    uint8_t addrTypes;
    uint16_t transportMask;
    uint16_t ipv4port;
    uint16_t ipv6port;
    uint32_t ipv4;
    uint32_t ipv6[4];
} AJ_Service;

/**
 * Discover a remote service
 *
 * @param prefix    The service name prefix
 * @param service   Information about the service that was found
 * @param timeout   How long to wait to discover the service
 *
 */
AJ_Status AJ_Discover(const char* prefix, AJ_Service* service, uint32_t timeout);

#endif
