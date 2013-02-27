#ifndef _AJ_BUS_H
#define _AJ_BUS_H
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
#include "aj_net.h"
#include "aj_status.h"

/**
 * Forward declarations
 */
typedef struct _AJ_Message AJ_Message;
typedef struct _AJ_Arg AJ_Arg;

/**
 * Callback function prototype for requesting a password or pincode from an application.
 *
 * @param buffer  The buffer to receive the password.
 * @param bufLen  The size of the buffer
 *
 * @return  Returns the length of the password. If the length is zero this will be
 *          treated as a rejected password request.
 */
typedef uint32_t (*AJ_AuthPwdFunc)(uint8_t* buffer, uint32_t bufLen);

/**
 * Type for a bus attachment
 */
typedef struct _AJ_BusAttachment {
    char uniqueName[16];         /**< The unique name returned by the hello message */
    AJ_NetSocket sock;           /**< Abstracts a network socket */
    uint32_t serial;             /**< Next outgoing message serial number */
    AJ_AuthPwdFunc pwdCallback;  /**< Callback for obtaining passwords */
} AJ_BusAttachment;

/**
 * Get the unique name for the bus
 *
 * @return  The unique name or NULL if the bus is not connected.
 */
const char* AJ_GetUniqueName(AJ_BusAttachment* bus);


#define AJ_NAME_REQ_ALLOW_REPLACEMENT 0x01  /**< Allow others to take ownership of this name */
#define AJ_NAME_REQ_REPLACE_EXISTING  0x02  /**< Attempt to take ownership of name if already taken */
#define AJ_NAME_REQ_DO_NOT_QUEUE      0x04  /**< Fail if name cannot be immediately obtained */

/**
 * Make a method call to request a well known name
 *
 * @param bus         The bus attachment
 * @param name        The name being requested
 * @param flags       An XOR of the name request flags
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusRequestName(AJ_BusAttachment* bus, const char* name, uint32_t flags);

#define AJ_TRANSPORT_NONE      0x0000    /**< no transports */
#define AJ_TRANSPORT_ANY       0xFFFF    /**< ANY transport */
#define AJ_TRANSPORT_LOCAL     0x0001    /**< Local (same device) transport */
#define AJ_TRANSPORT_BLUETOOTH 0x0002    /**< Bluetooth transport */
#define AJ_TRANSPORT_WLAN      0x0004    /**< Wireless local-area network transport */
#define AJ_TRANSPORT_WWAN      0x0008    /**< Wireless wide-area network transport */
#define AJ_TRANSPORT_LAN       0x0010    /**< Wired local-area network transport */
#define AJ_TRANSPORT_ICE       0x0020    /**< Transport using ICE protocol */
#define AJ_TRANSPORT_PROXIMITY 0x0040    /**< Transport compatible with WinRT Proximity Framework */

#define AJ_BUS_START_ADVERTISING 0
#define AJ_BUS_STOP_ADVERTISING  1

/**
 * Make a method call to start or stop advertising a name
 *
 * @param bus           The bus attachment
 * @param name          The name to be advertised
 * @param transportMask Restricts the transports the advertisement will be stopped/started on.
 * @param op            Either AJ_BUS_START_ADVERTISING or AJ_BUS_STOP_ADVERTISING
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusAdvertiseName(AJ_BusAttachment* bus, const char* name, uint16_t transportMask, uint8_t op);

#define AJ_BUS_START_FINDING 0
#define AJ_BUS_STOP_FINDING  1

#define AJ_FIND_NAME_STARTED    0x1   /**< Started to find the name as requested */
#define AJ_FIND_NAME_ALREADY    0x2   /**< Was already finding the requested name */
#define AJ_FIND_NAME_FAILURE    0x3   /**< Attempt to find the name failed */

/**
 * Register interest in a well-known name prefix for the purpose of discovery.
 *
 * @param  namePrefix   Well-known name prefix that application is interested in receiving
 *                      FoundAdvertisedName notifications about.
 * @param op            Either AJ_BUS_START_FINDING or AJ_BUS_STOP_FINDING
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusFindAdvertisedName(AJ_BusAttachment* bus, const char* namePrefix, uint8_t op);

#define AJ_SESSION_PROXIMITY_ANY          0xFF   /**< No proximity restrictions */
#define AJ_SESSION_PROXIMITY_PHYSICAL     0x01   /**< Limit to services that are physically proximal */
#define AJ_SESSION_PROXIMITY_NETWORK      0x02   /**< Allow services that are on the same subnet */

#define AJ_SESSION_TRAFFIC_MESSAGES       0x01   /**< Session carries message traffic */
#define AJ_SESSION_TRAFFIC_RAW_UNRELIABLE 0x02   /**< Not supported by this implementation */
#define AJ_SESSION_TRAFFIC_RAW_RELIABLE   0x04   /**< Not supported by this implementation */

#define AJ_SESSION_PORT_ANY                       0x00   /**< Use a daemon assigned ephemeral session port */

/**
 * Type for describing session options
 */
typedef struct _AJ_SessionOpts {
    uint8_t traffic;
    uint8_t proximity;
    uint16_t transports;
    uint32_t isMultipoint;
} AJ_SessionOpts;

/**
 * Make a method call to bind a session port.
 *
 * @param bus          The bus attachment
 * @param port         The port to bind, if AJ_SESSION_PORT_ANY is passed in, the daemon
 *                     will assign an ephemeral session port
 * @param opts         Options for establishing a session, if NULL defaults are used.
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusBindSessionPort(AJ_BusAttachment* bus, uint16_t port, const AJ_SessionOpts* opts);

/**
 * Send a reply to an accept session method call
 *
 * @param bus         The bus attachment
 * @param msg         The AcceptSession method call
 * @parm accept       TRUE to accept the session, FALSE to reject it.
 *
 */
AJ_Status AJ_BusReplyAcceptSession(AJ_Message* msg, uint32_t accept);

/**
 * Make a method call join a session.
 *
 * @param bus          The bus attachment
 * @param sessionHost  Bus name of attachment that is hosting the session to be joined.
 * @param port         The session port to join
 * @param opts         Options for establishing a session, if NULL defaults are used.
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusJoinSession(AJ_BusAttachment* bus, const char* sessionHost, uint16_t port, const AJ_SessionOpts* opts);

#define AJ_BUS_SIGNAL_ALLOW  0
#define AJ_BUS_SIGNAL_DENY   1

/**
 * Add a SIGNAL match rule. A rule must be added for every non-session signal that the application
 * is interested in receiving.
 *
 * @param bus           The bus attachment
 * @param signalName    The name of the signal.
 * @param ifaceName     The name of the interface.
 * @param rule          Either AJ_BUS_SIGNAL_ALLOW or AJ_BUS_SIGNAL_DENY
 *
 * @return
 *         - AJ_OK if the request was sent
 *         - An error status otherwise
 */
AJ_Status AJ_BusSetSignalRule(AJ_BusAttachment* bus, const char* signalName, const char* interfaceName, uint8_t rule);

/**
 * Invoke a built-in handler for standard bus messages. Signals passed to this function that are
 * not bus messages are silently ignored. Method calls passed to this function that are not
 * recognized bus messages are rejected with an error response.
 *
 * Method calls that currently have built-in handlers are:
 *
 *  - AJ_BUS_METHOD_PING
 *  - AJ_BUS_METHOD_GET_MACHINE_ID
 *  - AJ_BUS_METHOD_INTROSPECT
 *  - AJ_BUS_METHOD_EXCHANGE_GUIDS
 *  - AJ_BUS_METHOD_GEN_SESSION_KEY
 *  - AJ_BUS_METHOD_EXCHANGE_GROUP_KEYS
 *  - AJ_BUS_METHOD_AUTH_CHALLENGE
 *
 * @param msg     The message to handle
 *
 * @return
 *         - AJ_OK if the message was handled or ingored
 */
AJ_Status AJ_BusHandleBusMessage(AJ_Message* msg);

/**
 * Set a callback for returning passwords for peer authentication. Authentication is not enabled
 * until a password callback function has been set.
 *
 * @param bus          The bus attachment struct
 * @parma pwdCallback  The password callback function.
 */
void AJ_BusSetPasswordCallback(AJ_BusAttachment* bus, AJ_AuthPwdFunc pwdCallback);

/**
 * Callback function prototype for the function called when an authentication completes or fails.
 *
 * @param context   The context provided when AJ_PeerAuthenticate() was called.
 * @param status    A status code indicating if the authentication was succesful
 *                  - AJ_OK indicates the authentication succeeded
 *                  - AJ_ERR_SECURITY indicates the authentication failed
 *                  - AJ_ERR_TIMEOUT indciates the authentication timed-out
 */
typedef void (*AJ_BusAuthPeerCallback)(const void* context, AJ_Status status);

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
AJ_Status AJ_BusAuthenticatePeer(AJ_BusAttachment* bus, const char* peerBusName, AJ_BusAuthPeerCallback callback, void* cbContext);

/**
 * Callback function prototype for a callback function to GET a property. All this function has to
 * do is marshal the property value.
 *
 * @param replyMsg  The GET_PROPERTY reply message
 * @param propId    The property identifier
 * @apram context   The caller provided context that was passed into AJ_BusPropGet()
 *
 * @return  - AJ_OK if the property was read and marshaled
 *          - An error status if the property could not be returned for any reason.
 */
typedef AJ_Status (*AJ_BusPropGetCallback)(AJ_Message* replyMsg, uint32_t propId, void* context);

/**
 * Helper function that provides all the boilerplate for responding to a GET_PROPERTY. All the
 * application has to do is marshal the property value.
 *
 * @param msg       An unmarshalled GET_PROPERTY message
 * @param callback  The function called to request the application to marshal the property value.
 * @apram context   A caller provided context that is passed into the callback function
 */
AJ_Status AJ_BusPropGet(AJ_Message* msg, AJ_BusPropGetCallback callback, void* context);

/**
 * Callback function prototype for a callback function to SET an application property. All this
 * function has to do is unmarshal the property value.
 *
 * @param replyMsg  The SET_PROPERTY reply message
 * @param propId    The property identifier
 * @apram context   The caller provided context that was passed into AJ_BusPropSet()

 * @return  - AJ_OK if the property was unmarshaled
 *          - An error status if the property could not be set for any reason.
 */
typedef AJ_Status (*AJ_BusPropSetCallback)(AJ_Message* replyMsg, uint32_t propId, void* context);

/**
 * Helper function that provides all the boilerplate for responding to a SET_PROPERTY. All the
 * application has to do is unmarshal the property value.
 *
 * @param msg       An unmarshalled SET_PROPERTY message
 * @param callback  The function called to request the application to marshal the property value.
 * @apram context   A caller provided context that is passed into the callback function
 */
AJ_Status AJ_BusPropSet(AJ_Message* msg, AJ_BusPropSetCallback callback, void* context);

#endif