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


#ifndef _AJ_CONFIGURE_H
#define _AJ_CONFIGURE_H

#include <aj_introspect.h>
#include "aj_configureme.h"

/** Note to OEM: Make this the *FIRST* object in your list of AllJoyn objects */
extern const AJ_InterfaceDescription AJ_ConfigInterfaces[];

/**
 *  Attempt to process an internal configuration message
 *
 *  @param msg                  the incoming message
 *  @param identifyFunction     IdentifyFunction
 *
 *  @return
 *          - AJ_OK if we have processed the message
 *          - AJ_ERR_RESTART means that the OEM program should restart its event loop
 *          - AJ_ERR_UNEXPECTED if we dont' know the message; the app should process it!
 **/
AJ_Status AJ_ProcessInternal(AJ_Message* msg, IdentifyFunction identifyFunction);

#endif
