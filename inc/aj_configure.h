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


#ifndef _AJ_CONFIGURE_H
#define _AJ_CONFIGURE_H

#include <aj_introspect.h>

/** Identify Function */
typedef void (*IdentifyFunction)(char*, size_t);


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
