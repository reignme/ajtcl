#ifndef _AJ_CONNECT_H
#define _AJ_CONNECT_H
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
#include "aj_bus.h"

/**
 * Establish an AllJoyn connection.
 *
 * @param  bus          The bus attachment to connect.
 * @param  serviceName  Name of a specific service to connect to, NULL for the default name.
 * @param  timeout      How long to spend attempting to connect
 *
 * @return - AJ_OK if the connection was succesfully established
 *         - AJ_ERR_TIMEOUT if the connection attempt timed out
 */
AJ_Status AJ_Connect(AJ_BusAttachment* bus, const char* serviceName, uint32_t timeout);

/**
 * Terminate an AllJoyn connection
 *
 * @param  bus  The bus attachment to disconnect.
 */
void AJ_Disconnect(AJ_BusAttachment* bus);

#endif
