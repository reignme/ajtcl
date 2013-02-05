#ifndef _AJ_PEER_H
#define _AJ_PEER_H
/**
 * @file  Implements the org.alljoyn.Bus.Peer object
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
#include "aj_msg.h"

/**
 * Handle an exchange guids message
 *
 * @param msg  The exchange guids message
 */
AJ_Status AJ_PeerHandleExchangeGUIDs(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an exchange guids reply message
 *
 * @param msg  The exchange guids reply
 */
AJ_Status AJ_PeerHandleExchangeGUIDsReply(AJ_Message* msg);

/**
 * Handle an authentication challenge message
 *
 * @param msg  The authentication challenge message
 */
AJ_Status AJ_PeerHandleAuthChallenge(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an authentication challenge reply message
 *
 * @param msg  The authentication challenge reply message
 */
AJ_Status AJ_PeerHandleAuthChallengeReply(AJ_Message* msg);

/**
 * Handle a gen session key message
 *
 * @param msg  The gen session key message
 */
AJ_Status AJ_PeerHandleGenSessionKey(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle a gen session key reply message
 *
 * @param msg  The gen session key reply message
 */
AJ_Status AJ_PeerHandleGenSessionKeyReply(AJ_Message* msg);

/**
 * Handle an exchange group keys message
 *
 * @param msg  The exchange group keys message
 */
AJ_Status AJ_PeerHandleExchangeGroupKeys(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an exchange group keys reply message
 *
 * @param msg  The exchange group keys reply message
 */
AJ_Status AJ_PeerHandleExchangeGroupKeysReply(AJ_Message* msg);

/**
 * Callback function prototype for the function called when an authentication completes or fails.
 *
 * @param context   The context provided when AJ_PeerAuthenticate() was called.
 * @param status    A status code indicating if the authentication was succesful
 *                  - AJ_OK indicates the authentication succeeded
 *                  - AJ_ERR_SECURITY indicates the authentication failed
 *                  - AJ_ERR_TIMEOUT indciates the authentication timed-out
 */
typedef void (*AJ_PeerAuthenticateCallback)(const void* context, AJ_Status status);

/**
 * Initiate a secure connection to a remote peer authenticating if necessary.
 *
 * @param bus        The bus attachment
 * @param busName    The bus name of the remove peer to secure.
 * @param callback   A function to be called when the authentication completes
 * @Param cbContext  A caller provided context to pass to the callback function
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_PeerAuthenticate(AJ_BusAttachment* bus, const char* peerBusName, AJ_PeerAuthenticateCallback callback, void* cbContext);

#endif
