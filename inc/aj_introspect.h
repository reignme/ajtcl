#ifndef _AJ_INTROSPECT_H
#define _AJ_INTROSPECT_H
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

#include "aj_host.h"
#include "aj_status.h"
#include "aj_bus.h"
#include "aj_msg.h"

/**
 * Support for introspection
 */

/**
 * An interface description is a NULL terminated array of strings. The first string is the interface
 * name. The subsequent strings are a compact representation of the members of the interface. In
 * this representation special characters encode information about the members, whitespace is
 * significant.
 *
 * The first character of a member string identifies the type of member:
 *
 * A '?' character indicates the member is a METHOD
 * A '!' character indicates the member is a SIGNAL
 * A '@' character indicates the member is a PROPERTY
 *
 * Characters following the member type character up to the end of the string or to the first space
 * character are the member names. If the member is a METHOD or SIGNAL the remaining characters
 * encode the argument names, direction (IN or OUT) and the argument type as a standard AllJoyn
 * signature string. For SIGNALS for correctness the direction should be specified as OUT but it
 * really doensn't matter as the direction is ignored.
 *
 * Arguments are separated by a single space character. Argument names are optional and if present are
 * all characters between the space character and the directions character. All characters after the
 * direction character up to the next space or the end of the string are the argument type. The
 * argument direction is specified as follows:
 *
 * A '>' character indicates the argument is an OUT parameter.
 * A '<' character indicates the argument is an IN parameter.
 *
 * If the member is a PROPERTY the member name is terminated by an access rights character which is
 * immediately followed by the property type signature. The access rights for a property are READ_ONLY, WRITE_ONLY
 * and READ_WRITE. The access rights are specified as follows:
 *
 * A '>' character indicates the argument is READ_ONLY   (i.e. an OUT parameter)
 * A '<' character indicates the argument is WRITE_ONLY (i.e. an IN parameter)
 * A '=' character indicates the argument is READ/WRITE
 *
   @code

   static const char* ExampleInterface[] = {
    "org.alljoyn.example",                  // The interface name
    "?StringPing inStr<s outStr>",          // A method called StringPing with an IN arg and OUT arg of type string
    "?Hello",                               // A method call with no arguments
    "?Add <i <i >i",                        // A method call that takes two integers and returns an integer. The args are not named
    "!ListChanged >a{ys}",                  // A signal that returns a dictionary
    "@TimeNow>(yyy)",                       // A READ_ONLY property that returns a struct with three 8 bit integers
    "@Counter=u",                           // A READ/WRITE property
    "@SecretKey<ay",                        // A WRITE_ONLY property that sets an array of bytes
    NULL                                    // End marker
   };

   @endcode

 *
 * This compact representation is expanded automatically into the very much more verbose XML form required to support introspection
 * requests.
 */

/**
 * Type for an interface description - NULL terminated array of strings.
 */
typedef const char* const* AJ_InterfaceDescription;

/**
 * Type for an AllJoyn object description
 */
typedef struct _AJ_Object {
    const char* path;                               /**< object path */
    const AJ_InterfaceDescription* interfaces;      /**< interface descriptor */
} AJ_Object;


/*
 * Indicates that an identified member belongs to an application object
 */
#define AJ_BUS_ID_FLAG   0x00  /**< Built in bus object messages */
#define AJ_APP_ID_FLAG   0x01  /**< Application object messages */
#define AJ_PRX_ID_FLAG   0x02  /**< Proxy object messages */
#define AJ_REP_ID_FLAG   0x80  /**< Indicates a message is a reply message */

/*
 * Macros to encode a message id from the object path, interface, and member indices.
 */
#define AJ_BUS_MESSAGE_ID(p, i, m)  ((AJ_BUS_ID_FLAG << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m))       /**< Encode a message id from bus object */
#define AJ_APP_MESSAGE_ID(p, i, m)  ((AJ_APP_ID_FLAG << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m))       /**< Encode a message id from application object */
#define AJ_PRX_MESSAGE_ID(p, i, m)  ((AJ_PRX_ID_FLAG << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m))       /**< Encode a message id from proxy object */

/*
 * Macros to encode a property id from the object path, interface, and member indices.
 */
#define AJ_BUS_PROPERTY_ID(p, i, m) AJ_BUS_MESSAGE_ID(p, i, m)      /**< Encode a property id from bus object */
#define AJ_APP_PROPERTY_ID(p, i, m) AJ_APP_MESSAGE_ID(p, i, m)      /**< Encode a property id from application object */
#define AJ_PRX_PROPERTY_ID(p, i, m) AJ_PRX_MESSAGE_ID(p, i, m)      /**< Encode a property id from proxy object */

/**
 * Macro to generate the reply message identifier from method call message. This is the message
 * identifier in the reply context.
 */
#define AJ_REPLY_ID(id)  ((id) | (uint32_t)(AJ_REP_ID_FLAG << 24))

/**
 * Register the local objects and the remote objects for this application.  Local objects have
 * methods that remote applications can call, have properties that a remote application can GET or
 * SET or define signals that the local application can emit.  Proxy objects describe the remote
 * objects that have methods that this object can call and signals
 * that remote objects emit that this application can receive.
 *
 * @param localObjects  A NULL terminated array of object info structs.
 * @param proxyObjects  A NULL terminated array of object info structs.
 */
void AJ_RegisterObjects(const AJ_Object* localObjects, const AJ_Object* proxyObjects);

/**
 * This function checks that a message ifrom a remote peer is valid and correct and returns the
 * message id for that message.
 *
 * For method calls this function checks that the object is one of the registered objects, checks
 * that the interface and method are implemented by the object and checks that the signature is
 * correct.
 *
 * For signals this function checks that the interface is a known interface, the signal name is
 * defined for that interface, and the signature is correct.
 *
 * For method replies and error message this function matches the serial number of the response to
 * the serial number in the list of reply contexts. If the reply matches the signature is checked.
 *
 * If everything is correct the the message identifier is set in the message struct
 *
 * @param msg           The message to identify
 *
 * @return              Return AJ_Status
 */
AJ_Status AJ_IdentifyMessage(AJ_Message* msg);

/**
 * This function unmarshals the first two arguments of a property SET or GET message, identifies
 * which property the method is accessing and returns the id for the property.
 *
 * @param msg     The property GET or SET message to identify
 * @param propId  Returns the id for the identified property
 * @param sig     Buffer to fill in with the signature of the identified property
 * @param len     Length of the signature buffer
 *
 * @return   Return AJ_Status
 *         - ER_OK if the property was identified
 *         - AJ_ERR_NO_MATCH if there is no matching property
 *         - AJ_ERR_DISALLOWED if the property exists but has access rights do not permit the requested GET or SET operation.
 */
AJ_Status AJ_UnmarshalPropertyArgs(AJ_Message* msg, uint32_t* propId, char* sig, size_t len);

/**
 * This function marshals the first two arguments of a property SET or GET message.
 *
 * @param msg     The property GET or SET message to be initialized
 * @param propId  The the id for the specified property
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_MarshalPropertyArgs(AJ_Message* msg, uint32_t propId);

/**
 * Handle an introspection request
 *
 * @param msg        The introspection request method call
 * @param reply      The reply to the introspection request
 *
 * @return           Return AJ_Status
 */
AJ_Status AJ_HandleIntrospectRequest(const AJ_Message* msg, AJ_Message* reply);

/**
 * Internal function for initializing a message from information obtained via the message id.
 *
 * @param msg       The message to initialize
 * @param msgId     The message id
 * @param msgType   The type of the message
 *
 * @return          Return AJ_Status
 */
AJ_Status AJ_InitMessageFromMsgId(AJ_Message* msg, uint32_t msgId, uint8_t msgType);

/**
 * Internal function to allocate a reply context for a method call message. Reply contexts are used
 * to associate method replies with method calls. Depending on avaiable system resources the number
 * of reply contexts may be very limited, in some cases only one reply context.
 *
 * @param msg      A method call message that needs a reply context
 * @param timeout  The time to wait for a reply  (0 to use the internal default)
 *
 * @return   Return AJ_Status
 *         - AJ_OK if the reply context was allocated
 *         - AJ_ERR_RESOURCES if the reply context could not be allocated
 */
AJ_Status AJ_AllocReplyContext(AJ_Message* msg, uint32_t timeout);

/**
 * Internal function to release all reply contexts. Called when disconnecting from the bus.
 */
void AJ_ReleaseReplyContexts(void);

/**
 * Internal function to check for timed out method calls. Returns TRUE and sets some information in
 * the message struct to identify the timed-out call if there was one. This function is called by
 * AJ_UnmarshalMessage() when there are no messages to unmarshal.
 *
 * @param msg  A message structure to initialize if there was a timed-out method call.
 *
 * @return  Returns TRUE if there was a timed-out method call, FALSE otherwise.
 */
uint8_t AJ_TimedOutMethodCall(AJ_Message* msg);

/**
 * Internal function called to release a reply context in the case that a message could not be marshaled.
 *
 * @param msg  The message that a reply context might have been allocated for.
 */
void AJ_ReleaseReplyContext(AJ_Message* msg);

/**
 * Debugging aid prints out the XML for an object
 */
#ifdef NDEBUG
#define AJ_PrintXML(obj)
#else
void AJ_PrintXML(const AJ_Object* obj);
#endif

/**
 * Hook for unit testing marshal/unmarshal
 */
#ifndef NDEBUG
typedef AJ_Status (*AJ_MutterHook)(AJ_Message* msg, uint32_t msgId, uint8_t msgType);
#endif

#endif
