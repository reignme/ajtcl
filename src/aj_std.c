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
#include "aj_std.h"

const char AJ_DBusDestination[] = "org.freedesktop.DBus";
const char AJ_BusDestination[] = "org.alljoyn.Bus";

const char AJ_ErrServiceUnknown[] = "org.freedesktop.DBus.Error.ServiceUnknown";
const char AJ_ErrSecurityViolation[] = "org.alljoyn.Bus.SecurityViolation";
const char AJ_ErrTimeout[] = "org.alljoyn.Bus.Timeout";
const char AJ_ErrRejected[] = "org.alljoyn.Bus.Rejected";

static const char DBusObjectPath[] = "/org/freedesktop/DBus";
static const char DBusInterface[] = "org.freedesktop.DBus";
static const char DBusPeerInterface[] = "org.freedesktop.DBus.Peer";
static const char DBusPropsInterface[] = "org.freedesktop.DBus.Properties";
static const char DBusIntrospectableInterface[] = "org.freedesktop.DBus.Introspectable";

static const char BusObjectPath[] = "/org/alljoyn/Bus";
static const char BusInterface[] = "org.alljoyn.Bus";

static const char PeerObjectPath[] = "/org/alljoyn/Bus/Peer";
static const char PeerSessionInterface[] = "org.alljoyn.Bus.Peer.Session";
static const char PeerAuthInterface[] = "org.alljoyn.Bus.Peer.Authentication";



const char* const AJ_PropertiesIface[] = {
    DBusPropsInterface,
    "?Get <s <s >v",
    "?Set <s <s <v",
    "?GetAll <s >a{sv}",
    NULL
};

const char* const AJ_IntrospectionIface[] = {
    DBusIntrospectableInterface,
    "?Introspect >s",
    NULL
};

static const char* const DBusIface[] = {
    DBusInterface,
    "?Hello >s",
    "!NameOwnerChanged >s >s >s",
    "!NameAcquired >s",
    "!NameLost >s",
    "!PropertiesChanged >s >a{sv} >as",
    "?RequestName <s <u >u",
    "?AddMatch <s",
    "?RemoveMatch <s",
    NULL

};

static const char* const DBusPeerIface[] = {
    DBusPeerInterface,
    "?Ping",
    "?GetMachineId >s",
    NULL

};

static const AJ_InterfaceDescription DBusIfaces[] = {
    DBusIface,
    NULL
};

static const char* const BusIface[] = {
    BusInterface,
    "!SessionLost >u",
    "!FoundAdvertisedName >s >q >s",
    "!LostAdvertisedName >s >q >s",
    "!MPSessionChanged >u >s >b",
    "?AdvertiseName <s <q >u",
    "?CancelAdvertiseName <s <q >u",
    "?FindAdvertisedName <s >u",
    "?CancelFindAdvertisedName <s",
    "?BindSessionPort <q <a{sv} >u >q",
    "?UnbindSessionPort <q >u",
    "?JoinSession <s <q <a{sv} >u >u >a{sv}",
    "?LeaveSession <u >u",
    NULL
};

static const char* const PeerSessionIface[] = {
    PeerSessionInterface,
    "?AcceptSession <q <u <s <a{sv} >b",
    "!SessionJoined >q >u >s",
    NULL
};

static const char* const PeerAuthIface[] = {
    PeerAuthInterface,
    "?ExchangeGuids <s <u >s >u",
    "?GenSessionKey <s <s <s >s >s",
    "?ExchangeGroupKeys <ay >ay",
    "?AuthChallenge <s >s",
    "@Mechanisms >s",
    "@Version >u",
    NULL
};

static const AJ_InterfaceDescription PeerIfaces[] = {
    PeerSessionIface,
    PeerAuthIface,
    NULL
};

static const AJ_InterfaceDescription BusIfaces[] = {
    BusIface,
    NULL
};

/*
 * These are interfaces that all objects implement so use the wildcard in the object list
 */
static const AJ_InterfaceDescription CommonIfaces[] = {
    AJ_IntrospectionIface,
    DBusPeerIface,
    NULL
};

const AJ_Object AJ_StandardObjects[] = {
    { DBusObjectPath, DBusIfaces },
    { BusObjectPath,  BusIfaces },
    { PeerObjectPath, PeerIfaces },
    { "*",            CommonIfaces },
    { NULL,           NULL }
};
