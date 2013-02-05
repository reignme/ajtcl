#ifndef _AJ_STD_H
#define _AJ_STD_H
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
#include "aj_introspect.h"

/**
 * Identifiers for standard methods and signals. These are the values returned by
 * AJ_IdentifyMessage() for correctly formed method and signal messages.
 *
 * The first value is the index into the object array.
 * The second value is the index into the interfaces array for the object.
 * The third value is the index into the members array of the interface.
 */


/*
 * Members of the /org/freedesktop/DBus interface org.freedesktop.DBus
 */
#define AJ_METHOD_HELLO                AJ_BUS_MESSAGE_ID(0, 0, 0)
#define AJ_SIGNAL_NAME_OWNER_CHANGED   AJ_BUS_MESSAGE_ID(0, 0, 1)
#define AJ_SIGNAL_NAME_ACQUIRED        AJ_BUS_MESSAGE_ID(0, 0, 2)
#define AJ_SIGNAL_NAME_LOST            AJ_BUS_MESSAGE_ID(0, 0, 3)
#define AJ_SIGNAL_PROPS_CHANGED        AJ_BUS_MESSAGE_ID(0, 0, 4)
#define AJ_METHOD_REQUEST_NAME         AJ_BUS_MESSAGE_ID(0, 0, 5)
#define AJ_METHOD_ADD_MATCH            AJ_BUS_MESSAGE_ID(0, 0, 6)
#define AJ_METHOD_REMOVE_MATCH         AJ_BUS_MESSAGE_ID(0, 0, 7)

/*
 * Members of /org/alljoyn/Bus interface org.alljoyn.Bus
 */
#define AJ_SIGNAL_SESSION_LOST         AJ_BUS_MESSAGE_ID(1, 0, 0)
#define AJ_SIGNAL_FOUND_ADV_NAME       AJ_BUS_MESSAGE_ID(1, 0, 1)
#define AJ_SIGNAL_LOST_ADV_NAME        AJ_BUS_MESSAGE_ID(1, 0, 2)
#define AJ_SIGNAL_MP_SESSION_CHANGED   AJ_BUS_MESSAGE_ID(1, 0, 3)
#define AJ_METHOD_ADVERTISE_NAME       AJ_BUS_MESSAGE_ID(1, 0, 4)
#define AJ_METHOD_CANCEL_ADVERTISE     AJ_BUS_MESSAGE_ID(1, 0, 5)
#define AJ_METHOD_FIND_NAME            AJ_BUS_MESSAGE_ID(1, 0, 6)
#define AJ_METHOD_CANCEL_FIND_NAME     AJ_BUS_MESSAGE_ID(1, 0, 7)
#define AJ_METHOD_BIND_SESSION_PORT    AJ_BUS_MESSAGE_ID(1, 0, 8)
#define AJ_METHOD_UNBIND_SESSION       AJ_BUS_MESSAGE_ID(1, 0, 9)
#define AJ_METHOD_JOIN_SESSION         AJ_BUS_MESSAGE_ID(1, 0, 10)
#define AJ_METHOD_LEAVE_SESSION        AJ_BUS_MESSAGE_ID(1, 0, 11)

/*
 * Members of /org/alljoyn/Bus/Peer interface org.alljoyn.Bus.Peer.Session
 */
#define AJ_METHOD_ACCEPT_SESSION       AJ_BUS_MESSAGE_ID(2, 0, 0)
#define AJ_SIGNAL_SESSION_JOINED       AJ_BUS_MESSAGE_ID(2, 0, 1)

/*
 * Members of /org/alljoyn/Bus/Peer interface org.alljoyn.Bus.Peer.Authentication
 */
#define AJ_METHOD_EXCHANGE_GUIDS       AJ_BUS_MESSAGE_ID(2, 1, 0)
#define AJ_METHOD_GEN_SESSION_KEY      AJ_BUS_MESSAGE_ID(2, 1, 1)
#define AJ_METHOD_EXCHANGE_GROUP_KEYS  AJ_BUS_MESSAGE_ID(2, 1, 2)
#define AJ_METHOD_AUTH_CHALLENGE       AJ_BUS_MESSAGE_ID(2, 1, 3)

/*
 * Members of interface org.freedesktop.DBus.Introspectable
 */
#define AJ_METHOD_INTROSPECT           AJ_BUS_MESSAGE_ID(3, 0, 0)

/*
 * Members of the interface org.freedesktop.DBus.Peer
 */
#define AJ_METHOD_PING                 AJ_BUS_MESSAGE_ID(3, 2, 0)
#define AJ_METHOD_GET_MACHINE_ID       AJ_BUS_MESSAGE_ID(3, 2, 1)

/**
 * Message identifier that indicates a message was invalid.
 */
#define AJ_INVALID_MSG_ID              (0xFFFFFFFF)

/**
 * Message identifier that indicates a property was invalid.
 */
#define AJ_INVALID_PROP_ID             (0xFFFFFFFF)

/**
 * DBus well-known bus name
 */
extern const char AJ_DBusDestination[21];

/**
 * AllJoyn well-known bus name
 */
extern const char AJ_BusDestination[16];

/**
 * Error message strings
 */
extern const char AJ_ErrSecurityViolation[34];
extern const char AJ_ErrTimeout[24];
extern const char AJ_ErrRejected[25];
extern const char AJ_ErrServiceUnknown[41];

/**
 * The properties interface. This interface must be included in the property lists of all local and
 * proxy objects that have properties.
 */
extern const char* const AJ_PropertiesIface[5];

/**
 * Constants for the various property method indices in the properties interface
 */
#define AJ_PROP_GET     0
#define AJ_PROP_SET     1
#define AJ_PROP_GET_ALL 2

/**
 * The introspection interface.
 */
extern const char* const AJ_IntrospectionIface[3];

/**
 * The standard objects that implement AllJoyn core functionality
 */
extern const AJ_Object AJ_StandardObjects[5];

#endif
