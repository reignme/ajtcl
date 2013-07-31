#ifndef _AJ_CONNECT_H
#define _AJ_CONNECT_H
/**
 * @file aj_connect.h
 * @defgroup aj_connect Bus Connection Management
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
#include "aj_bus.h"

/**
 * Establish an AllJoyn connection.
 *
 * @param  bus          The bus attachment to connect.
 * @param  serviceName  Name of a specific service to connect to, NULL for the default name.
 * @param  timeout      How long to spend attempting to connect
 *
 * @return
 *         - AJ_OK if the connection was succesfully established
 *         - AJ_ERR_TIMEOUT if the connection attempt timed out
 */
AJ_Status AJ_Connect(AJ_BusAttachment* bus, const char* serviceName, uint32_t timeout);

/**
 * Terminate an AllJoyn connection
 *
 * @param  bus  The bus attachment to disconnect.
 */
void AJ_Disconnect(AJ_BusAttachment* bus);

/**
 * Bus authentication password function prototype for requesting a
 * password (to authenticate with the daemon) from the application.
 *
 * @param  buffer  The buffer to receive the password
 * @param  bufLen  The size of the buffer
 *
 * @return  Returns the length of the password. If the length is zero,
 *          this will be treated as a rejected password request.
 */
typedef uint32_t (*BusAuthPwdFunc)(uint8_t* buffer, uint32_t bufLen);

/**
 * Set the callback for the application to provide a password for authentication to the daemon bus
 *
 * @param callback  The callback provided by the application
 */
void SetBusAuthPwdCallback(BusAuthPwdFunc callback);

/**
 * @}
 */
#endif
