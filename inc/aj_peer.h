#ifndef _AJ_PEER_H
#define _AJ_PEER_H
/**
 * @file
 * Implements the org.alljoyn.Bus.Peer object
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
#include "aj_msg.h"

/**
 * Handle an exchange guids message
 *
 * @param msg    The exchange guids message
 * @param reply  The guids reply message
 *
 * @return        Return AJ_Status
 *          - AJ_OK if successfully handled
 *          - AJ_ERR_RESOURCES if the arguments are too big to marshal into the message buffer
 *          - AJ_ERR_WRITE if there was a write failure
 *          - AJ_ERR_UNEXPECTED if any of the argument types in the signature is not a basic type
 */
AJ_Status AJ_PeerHandleExchangeGUIDs(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an exchange guids reply message
 *
 * @param msg    The exchange guids reply
 *
 * @return        Return AJ_Status
 *          - AJ_OK if successfully handled
 *          - AJ_ERR_UNMARSHAL if the message was badly formed
 *          - AJ_ERR_RESOURCES if the message header is too big to unmarshal into the attached buffer
 *          - AJ_ERR_TIMEOUT if there was no message to unmarshal within the timeout period
 *          - AJ_ERR_READ if there was a read failure
 */
AJ_Status AJ_PeerHandleExchangeGUIDsReply(AJ_Message* msg);

/**
 * Handle an authentication challenge message
 *
 * @param msg    The authentication challenge message
 * @param reply  The authentication challenge reply message
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_PeerHandleAuthChallenge(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an authentication challenge reply message
 *
 * @param msg  The authentication challenge reply message
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_PeerHandleAuthChallengeReply(AJ_Message* msg);

/**
 * Handle a gen session key message
 *
 * @param msg    The gen session key message
 * @param reply  The gen session key reply message
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_PeerHandleGenSessionKey(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle a gen session key reply message
 *
 * @param msg  The gen session key reply message
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_PeerHandleGenSessionKeyReply(AJ_Message* msg);

/**
 * Handle an exchange group keys message
 *
 * @param msg    The exchange group keys message
 * @param reply  The group keys reply message
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_PeerHandleExchangeGroupKeys(AJ_Message* msg, AJ_Message* reply);

/**
 * Handle an exchange group keys reply message
 *
 * @param msg  The exchange group keys reply message
 *
 * @return        Return AJ_Status
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
 * @param peerName   The bus name of the remove peer to secure.
 * @param callback   A function to be called when the authentication completes
 * @param cbContext  A caller provided context to pass to the callback function
 *
 * @return   Return AJ_Status
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_PeerAuthenticate(AJ_BusAttachment* bus, const char* peerName, AJ_PeerAuthenticateCallback callback, void* cbContext);

/**
 * @}
 */

/**
 * Generate a compression token from a message.
 *
 * @param msg   The message to compress
 *
 * @return  Returns the compression token.
 */
uint32_t AJ_PeerCompressionToken(AJ_Message* msg);


/**
 * Handle a GetExpansion method call.
 *
 * @param msg     The GetExpansion request message
 * @param reply   The reply to the request
 *
 * @return        Return AJ_Status
 *
 */
AJ_Status AJ_PeerHandleGetExpansion(AJ_Message* msg, AJ_Message* reply);

#endif
