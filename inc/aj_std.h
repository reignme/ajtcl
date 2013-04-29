#ifndef _AJ_STD_H
#define _AJ_STD_H
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

#include "aj_target.h"
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
#define AJ_METHOD_HELLO                AJ_BUS_MESSAGE_ID(0, 0, 0)    /**< method for hello */
#define AJ_SIGNAL_NAME_OWNER_CHANGED   AJ_BUS_MESSAGE_ID(0, 0, 1)    /**< signal for name owner changed */
#define AJ_SIGNAL_NAME_ACQUIRED        AJ_BUS_MESSAGE_ID(0, 0, 2)    /**< signal for name acquired */
#define AJ_SIGNAL_NAME_LOST            AJ_BUS_MESSAGE_ID(0, 0, 3)    /**< signal for name lost */
#define AJ_SIGNAL_PROPS_CHANGED        AJ_BUS_MESSAGE_ID(0, 0, 4)    /**< signal for props changed */
#define AJ_METHOD_REQUEST_NAME         AJ_BUS_MESSAGE_ID(0, 0, 5)    /**< method for request name */
#define AJ_METHOD_ADD_MATCH            AJ_BUS_MESSAGE_ID(0, 0, 6)    /**< method for add match */
#define AJ_METHOD_REMOVE_MATCH         AJ_BUS_MESSAGE_ID(0, 0, 7)    /**< method for remove match */
#define AJ_METHOD_RELEASE_NAME         AJ_BUS_MESSAGE_ID(0, 0, 8)    /**< method for release name */

/*
 * Members of /org/alljoyn/Bus interface org.alljoyn.Bus
 */
#define AJ_SIGNAL_SESSION_LOST         AJ_BUS_MESSAGE_ID(1, 0, 0)    /**< signal for session lost */
#define AJ_SIGNAL_FOUND_ADV_NAME       AJ_BUS_MESSAGE_ID(1, 0, 1)    /**< signal for found advertising name */
#define AJ_SIGNAL_LOST_ADV_NAME        AJ_BUS_MESSAGE_ID(1, 0, 2)    /**< signal for lost advertising name */
#define AJ_SIGNAL_MP_SESSION_CHANGED   AJ_BUS_MESSAGE_ID(1, 0, 3)    /**< signal for mp session changed */
#define AJ_METHOD_ADVERTISE_NAME       AJ_BUS_MESSAGE_ID(1, 0, 4)    /**< method for advertise name */
#define AJ_METHOD_CANCEL_ADVERTISE     AJ_BUS_MESSAGE_ID(1, 0, 5)    /**< method for cancel advertise */
#define AJ_METHOD_FIND_NAME            AJ_BUS_MESSAGE_ID(1, 0, 6)    /**< method for find name */
#define AJ_METHOD_CANCEL_FIND_NAME     AJ_BUS_MESSAGE_ID(1, 0, 7)    /**< method for cancel find name */
#define AJ_METHOD_BIND_SESSION_PORT    AJ_BUS_MESSAGE_ID(1, 0, 8)    /**< method for bind session port */
#define AJ_METHOD_UNBIND_SESSION       AJ_BUS_MESSAGE_ID(1, 0, 9)    /**< method for unbind session */
#define AJ_METHOD_JOIN_SESSION         AJ_BUS_MESSAGE_ID(1, 0, 10)   /**< method for join session */
#define AJ_METHOD_LEAVE_SESSION        AJ_BUS_MESSAGE_ID(1, 0, 11)   /**< method for leave session */
#define AJ_METHOD_CANCEL_SESSIONLESS   AJ_BUS_MESSAGE_ID(1, 0, 12)   /**< method for cancel sessionless */
#define AJ_METHOD_FIND_NAME_BY_TRANSPORT   AJ_BUS_MESSAGE_ID(1, 0, 13)          /**< method for find name by specific transports */
#define AJ_METHOD_CANCEL_FIND_NAME_BY_TRANSPORT   AJ_BUS_MESSAGE_ID(1, 0, 14)   /**< method for cancel find name by specific transports */

/*
 * Members of /org/alljoyn/Bus/Peer interface org.alljoyn.Bus.Peer.Session
 */
#define AJ_METHOD_ACCEPT_SESSION       AJ_BUS_MESSAGE_ID(2, 0, 0)    /**< method for accept session */
#define AJ_SIGNAL_SESSION_JOINED       AJ_BUS_MESSAGE_ID(2, 0, 1)    /**< signal for session joined */

/*
 * Members of /org/alljoyn/Bus/Peer interface org.alljoyn.Bus.Peer.Authentication
 */
#define AJ_METHOD_EXCHANGE_GUIDS       AJ_BUS_MESSAGE_ID(2, 1, 0)    /**< method for exchange guids */
#define AJ_METHOD_GEN_SESSION_KEY      AJ_BUS_MESSAGE_ID(2, 1, 1)    /**< method for generate session key */
#define AJ_METHOD_EXCHANGE_GROUP_KEYS  AJ_BUS_MESSAGE_ID(2, 1, 2)    /**< method for exchange group keys */
#define AJ_METHOD_AUTH_CHALLENGE       AJ_BUS_MESSAGE_ID(2, 1, 3)    /**< method for auth challenge */

/*
 * Members of interface org.freedesktop.DBus.Introspectable
 */
#define AJ_METHOD_INTROSPECT           AJ_BUS_MESSAGE_ID(3, 0, 0)    /**< method for intprospect */

/*
 * Members of the interface org.freedesktop.DBus.Peer
 */
#define AJ_METHOD_PING                 AJ_BUS_MESSAGE_ID(3, 2, 0)    /**< method for ping */
#define AJ_METHOD_GET_MACHINE_ID       AJ_BUS_MESSAGE_ID(3, 2, 1)    /**< method for get machine id */

/*
 * Members of /org/alljoyn/Daemon interface org.alljoyn.Daemon
 */
#define AJ_SIGNAL_PROBE_REQ            AJ_BUS_MESSAGE_ID(4, 0, 0)    /**< signal for link probe request */
#define AJ_SIGNAL_PROBE_ACK            AJ_BUS_MESSAGE_ID(4, 0, 1)    /**< signal for link probe acknowledgement */

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

/*
 * Error message strings
 */
extern const char AJ_ErrSecurityViolation[35];    /**< Error security violation string */
extern const char AJ_ErrTimeout[25];              /**< Error timeout string */
extern const char AJ_ErrRejected[26];             /**< Error rejected string */
extern const char AJ_ErrServiceUnknown[42];       /**< Error service unknown string */

/**
 * The properties interface. This interface must be included in the property lists of all local and
 * proxy objects that have properties.
 */
extern const char* const AJ_PropertiesIface[5];

/*
 * Constants for the various property method indices in the properties interface
 */
#define AJ_PROP_GET     0        /**< index for property method get */
#define AJ_PROP_SET     1        /**< index for property method set */
#define AJ_PROP_GET_ALL 2        /**< index for property method get_all */

/**
 * The introspection interface.
 */
extern const char* const AJ_IntrospectionIface[3];

/**
 * The standard objects that implement AllJoyn core functionality
 */
extern const AJ_Object AJ_StandardObjects[6];

#endif
