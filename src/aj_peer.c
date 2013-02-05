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
#include "aj_peer.h"
#include "aj_bus.h"
#include "aj_msg.h"
#include "aj_util.h"
#include "aj_guid.h"
#include "aj_creds.h"
#include "aj_std.h"
#include "aj_auth.h"
#include "aj_crypto.h"
#include "aj_sasl.h"

/*
 * Version number of the key generation algorithm.
 */
#define MIN_KEYGEN_VERSION  0x01
#define MAX_KEYGEN_VERSION  0x01

/*
 * The base authentication version number
 */
#define MIN_AUTH_VERSION  0x0001
#define MAX_AUTH_VERSION  0x0001

#define REQUIRED_AUTH_VERSION  ((MAX_AUTH_VERSION << 16) | MIN_KEYGEN_VERSION)

/*
 * Maximum time to allow an incomplete authentication conversation to proceed
 */
#define MAX_AUTH_TIME  (5 * 60 * 1000)

/*
 * Long timeout for an authentication method calls to allow time for user interaction.
 */
#define AUTH_CALL_TIMEOUT   (2 * 60 * 1000)

/*
 * Timeout for all other method calls
 */
#define CALL_TIMEOUT        (1000 * 5)


#define VERIFIER_LEN  12
#define NONCE_LEN     28
#define KEY_LEN       16

typedef struct _AuthContext {
    AJ_BusAuthPeerCallback callback; /* Callback function to report completion */
    void* cbContext;                 /* Context to pass to the callback function */
    char nonce[2 * NONCE_LEN + 1];   /* Nonce as ascii hex */
    AJ_SASL_Context sasl;            /* The SASL state machine context */
    const AJ_GUID* peerGuid;         /* GUID pointer for the currently authenticating peer */
    AJ_Time timer;                   /* Timer for detecting failed authentication attempts */
} AuthContext;

static AuthContext authContext;

/*
 * Check that we are in an authentication with the peer
 */
static AJ_Status CheckAuthPeer(AJ_Message* msg)
{
    const AJ_GUID* peerGuid = AJ_GUID_Find(msg->sender);
    if (!peerGuid || (authContext.peerGuid != peerGuid)) {
        return AJ_ERR_UNEXPECTED;
    } else {
        return AJ_OK;
    }
}

AJ_Status AJ_PeerHandleExchangeGUIDs(AJ_Message* msg, AJ_Message* reply)
{
    char guidStr[33];
    uint32_t version;
    char* str;
    AJ_GUID remoteGuid;

    AJ_UnmarshalArgs(msg, "su", &str, &version);
    AJ_GUID_FromString(&remoteGuid, str);
    AJ_GUID_AddNameMapping(&remoteGuid, msg->sender);
    /*
     * We are not currently negotiating versions so we tell the peer what version we require.
     */
    version = REQUIRED_AUTH_VERSION;
    AJ_MarshalReplyMsg(msg, reply);
    AJ_GUID_ToString(AJ_GetLocalGUID(), guidStr, sizeof(guidStr));
    return AJ_MarshalArgs(reply, "su", guidStr, version);
}

#define AUTH_BUF_LEN 180

AJ_Status AJ_PeerHandleAuthChallenge(AJ_Message* msg, AJ_Message* reply)
{
    AJ_Status status;
    AJ_Arg arg;
    char* buf = NULL;
    const AJ_GUID* peerGuid = AJ_GUID_Find(msg->sender);

    /*
     * We expect to know the GUID of the sender
     */
    if (!peerGuid) {
        return AJ_MarshalErrorMsg(msg, reply, AJ_ErrRejected);
    }
    /*
     * Check for an authentication conversation in progress with a different peer
     */
    if (authContext.peerGuid && (authContext.peerGuid != peerGuid)) {
        /*
         * Reject the request if the existing conversation has not expired
         */
        if (AJ_GetElapsedTime(&authContext.timer, TRUE) < MAX_AUTH_TIME) {
            return AJ_MarshalErrorMsg(msg, reply, AJ_ErrRejected);
        }
        memset(&authContext, 0, sizeof(AuthContext));
    }
    if (authContext.sasl.state == AJ_SASL_IDLE) {
        /*
         * Remember which peer is being authenticated and initialize the timeout timer
         */
        authContext.peerGuid = peerGuid;
        AJ_InitTimer(&authContext.timer);
        /*
         * Initialize SASL state machine
         */
        AJ_SASL_InitContext(&authContext.sasl, &AJ_AuthPin, AJ_AUTH_CHALLENGER, msg->bus->pwdCallback);
    }
    if (AJ_UnmarshalArg(msg, &arg) != AJ_OK) {
        goto FailAuth;
    }
    /*
     * Need a short-lived buffer to compose the response
     */
    buf = AJ_Malloc(AUTH_BUF_LEN);
    if (!buf) {
        status = AJ_ERR_RESOURCES;
        goto FailAuth;
    }
    status = AJ_SASL_Advance(&authContext.sasl, (char*)arg.val.v_string, buf, AUTH_BUF_LEN);
    if (status != AJ_OK) {
        goto FailAuth;
    }
    AJ_MarshalReplyMsg(msg, reply);
    AJ_MarshalArgs(reply, "s", buf);
    AJ_Free(buf);
    if (authContext.sasl.state == AJ_SASL_AUTHENTICATED) {
        AJ_AuthPin.Final(peerGuid);
        memset(&authContext, 0, sizeof(AuthContext));
    }
    return AJ_OK;

FailAuth:

    AJ_Free(buf);
    /*
     * Clear current authentication context then return an error response
     */
    AJ_AuthPin.Final(peerGuid);
    memset(&authContext, 0, sizeof(AuthContext));
    return AJ_MarshalErrorMsg(msg, reply, AJ_ErrSecurityViolation);
}

static AJ_Status KeyGen(const char* peerName, uint8_t role, const char* nonce1, const char* nonce2, uint8_t* outBuf, uint32_t len)
{
    AJ_Status status;
    const uint8_t* data[4];
    uint8_t lens[4];
    const AJ_GUID* peerGuid = AJ_GUID_Find(peerName);
    const AJ_PeerCred* cred = peerGuid ? AJ_GetRemoteCredential(peerGuid) : NULL;

    if (!cred) {
        return AJ_ERR_NO_MATCH;
    }
    data[0] = cred->secret;
    lens[0] = (uint32_t)sizeof(cred->secret);
    data[1] = nonce1;
    lens[1] = (uint32_t)strlen(nonce1);
    data[2] = nonce2;
    lens[2] = (uint32_t)strlen(nonce2);
    data[3] = "session key";
    lens[3] = (uint32_t)strlen("session key");

    /*
     * We use the outBuf to store both the key and verifier string.
     * Check that there is enough space to do so.
     */
    if (KEY_LEN + VERIFIER_LEN > len) {
        return AJ_ERR_RESOURCES;
    }

    status = AJ_Crypto_PRF(data, lens, ArraySize(data), outBuf, KEY_LEN + VERIFIER_LEN);
    /*
     * Store the session key and compose the verifier string.
     */
    if (status == AJ_OK) {
        status = AJ_SetSessionKey(peerName, outBuf, role);
        memmove(outBuf, outBuf + KEY_LEN, VERIFIER_LEN);
        AJ_RawToHex(outBuf, VERIFIER_LEN, (char*)outBuf, len);
    }
    return status;
}

AJ_Status AJ_PeerHandleGenSessionKey(AJ_Message* msg, AJ_Message* reply)
{
    AJ_Status status;
    char* remGuid;
    char* locGuid;
    char* nonce;
    AJ_GUID guid;
    /*
     * For 12 bytes of verifier, we need at least 12 * 2 characters
     * to store its representation in hex (24 octets + 1 octet for \0).
     * However, the KeyGen function demands a bigger buffer
     * (to store 16 bytes key in addition to the 12 bytes verifier).
     * Hence we allocate, the maximum of (12 * 2 + 1) and (16 + 12).
     */
    char verifier[KEY_LEN + VERIFIER_LEN];

    /*
     * Remote peer GUID, Local peer GUID and Remote peer's nonce
     */
    AJ_UnmarshalArgs(msg, "sss", &remGuid, &locGuid, &nonce);
    /*
     * We expect arg[1] to be the local GUID
     */
    status = AJ_GUID_FromString(&guid, locGuid);
    if ((status != AJ_OK) || (memcmp(&guid, AJ_GetLocalGUID(), sizeof(AJ_GUID)) != 0)) {
        return AJ_MarshalErrorMsg(msg, reply, AJ_ErrRejected);
    }
    AJ_RandHex(authContext.nonce, sizeof(authContext.nonce), NONCE_LEN);
    status = KeyGen(msg->sender, AJ_ROLE_KEY_RESPONDER, nonce, authContext.nonce, (uint8_t*)verifier, sizeof(verifier));
    if (status == AJ_OK) {
        AJ_MarshalReplyMsg(msg, reply);
        status = AJ_MarshalArgs(reply, "ss", authContext.nonce, verifier);
    } else {
        status = AJ_MarshalErrorMsg(msg, reply, AJ_ErrRejected);
    }
    return status;
}

AJ_Status AJ_PeerHandleExchangeGroupKeys(AJ_Message* msg, AJ_Message* reply)
{
    AJ_Status status;
    AJ_Arg key;

    AJ_UnmarshalArg(msg, &key);
    /*
     * We expect the key to be 16 bytes
     */
    if (key.len != KEY_LEN) {
        status = AJ_ERR_INVALID;
    } else {
        status = AJ_SetGroupKey(msg->sender, key.val.v_byte);
    }
    if (status == AJ_OK) {
        uint8_t groupKey[KEY_LEN];
        AJ_MarshalReplyMsg(msg, reply);
        AJ_GetGroupKey(NULL, groupKey);
        status = AJ_MarshalArg(reply, AJ_InitArg(&key, AJ_ARG_BYTE, AJ_ARRAY_FLAG, groupKey, sizeof(groupKey)));
    } else {
        status = AJ_MarshalErrorMsg(msg, reply, AJ_ErrRejected);
    }
    return status;
}

static void PeerAuthComplete(AJ_Status status)
{
    if (authContext.callback) {
        /*
         * Report the authentication
         */
        authContext.callback(authContext.cbContext, status);
    }
    memset(&authContext, 0, sizeof(AuthContext));
}

AJ_Status AJ_PeerAuthenticate(AJ_BusAttachment* bus, const char* peerName, AJ_PeerAuthenticateCallback callback, void* cbContext)
{
    AJ_Message msg;
    char guidStr[33];
    uint32_t version = REQUIRED_AUTH_VERSION;

    /*
     * Check there isn't an authentication in progress
     */
    if (authContext.callback || authContext.peerGuid) {
        /*
         * The existing authentication may have timed-out
         */
        if (AJ_GetElapsedTime(&authContext.timer, TRUE) < MAX_AUTH_TIME) {
            return AJ_ERR_RESOURCES;
        }
        /*
         * Report the failed authentication
         */
        PeerAuthComplete(AJ_ERR_TIMEOUT);
    }
    authContext.callback = callback;
    authContext.cbContext = cbContext;
    AJ_InitTimer(&authContext.timer);
    /*
     * Kick off autnetication with an ExchangeGUIDS method call
     */
    AJ_MarshalMethodCall(bus, &msg, AJ_METHOD_EXCHANGE_GUIDS, peerName, 0, AJ_NO_FLAGS, CALL_TIMEOUT);
    AJ_GUID_ToString(AJ_GetLocalGUID(), guidStr, sizeof(guidStr));
    AJ_MarshalArgs(&msg, "su", guidStr, version);
    return AJ_DeliverMsg(&msg);
}

static AJ_Status GenSessionKey(AJ_Message* msg)
{
    AJ_Message call;

    AJ_AuthPin.Final(authContext.peerGuid);
    authContext.sasl.state = AJ_SASL_IDLE;
    AJ_MarshalMethodCall(msg->bus, &call, AJ_METHOD_GEN_SESSION_KEY, msg->sender, 0, AJ_NO_FLAGS, CALL_TIMEOUT);
    /*
     * Marshal local peer GUID, remote peer GUID, and local peer's GUID
     */
    {
        char guidStr[33];
        AJ_GUID_ToString(AJ_GetLocalGUID(), guidStr, sizeof(guidStr));
        AJ_MarshalArgs(&call, "s", guidStr);
        AJ_GUID_ToString(authContext.peerGuid, guidStr, sizeof(guidStr));
        AJ_RandHex(authContext.nonce, sizeof(authContext.nonce), NONCE_LEN);
        AJ_MarshalArgs(&call, "ss", guidStr, authContext.nonce);
    }
    return AJ_DeliverMsg(&call);
}

static AJ_Status AuthResponse(AJ_Message* msg, char* inStr)
{
    AJ_Status status;
    char* buf;

    if (authContext.sasl.state == AJ_SASL_AUTHENTICATED) {
        return GenSessionKey(msg);
    }
    /*
     * Need a short-lived buffer to compose the response
     */
    buf = AJ_Malloc(AUTH_BUF_LEN);
    if (!buf) {
        status = AJ_ERR_RESOURCES;
    } else {
        status = AJ_SASL_Advance(&authContext.sasl, inStr, buf, AUTH_BUF_LEN);
        if (status == AJ_OK) {
            AJ_Message call;
            AJ_MarshalMethodCall(msg->bus, &call, AJ_METHOD_AUTH_CHALLENGE, msg->sender, 0, AJ_NO_FLAGS, AUTH_CALL_TIMEOUT);
            AJ_MarshalArgs(&call, "s", buf);
            status = AJ_DeliverMsg(&call);
        }
        AJ_Free(buf);
    }
    /*
     * If there was an error finalize the auth mechanism
     */
    if (status != AJ_OK) {
        AJ_AuthPin.Final(authContext.peerGuid);
        /*
         * Report authentication failure to application
         */
        if (status != AJ_OK) {
            PeerAuthComplete(status);
        }
    }
    return status;
}

AJ_Status AJ_PeerHandleAuthChallengeReply(AJ_Message* msg)
{
    AJ_Status status;
    AJ_Arg arg;

    /*
     * Check we are in an auth conversation with the sender
     */
    status = CheckAuthPeer(msg);
    if (status != AJ_OK) {
        return status;
    }
    status = AJ_UnmarshalArg(msg, &arg);
    if (status == AJ_OK) {
        status = AuthResponse(msg, (char*)arg.val.v_string);
    }
    return status;
}

AJ_Status AJ_PeerHandleExchangeGUIDsReply(AJ_Message* msg)
{
    AJ_Status status;
    const char* guidStr;
    AJ_GUID remoteGuid;
    uint32_t version;

    status = AJ_UnmarshalArgs(msg, "su", &guidStr, &version);
    if (status != AJ_OK) {
        return status;
    }
    if (version != REQUIRED_AUTH_VERSION) {
        PeerAuthComplete(AJ_ERR_SECURITY);
        return AJ_OK;
    }
    AJ_GUID_FromString(&remoteGuid, guidStr);
    AJ_GUID_AddNameMapping(&remoteGuid, msg->sender);
    /*
     * Remember which peer is being authenticated
     */
    authContext.peerGuid = AJ_GUID_Find(msg->sender);
    /*
     * Initialize SASL state machine
     */
    AJ_SASL_InitContext(&authContext.sasl, &AJ_AuthPin, AJ_AUTH_RESPONDER, msg->bus->pwdCallback);
    /*
     * Start the authentication conversation
     */
    status = AuthResponse(msg, NULL);
    if (status != AJ_OK) {
        PeerAuthComplete(status);
    }
    return status;
}

AJ_Status AJ_PeerHandleGenSessionKeyReply(AJ_Message* msg)
{
    AJ_Status status;
    /*
     * For 12 bytes of verifier, we need at least 12 * 2 characters
     * to store its representation in hex (24 octets + 1 octet for \0).
     * However, the KeyGen function demands a bigger buffer
     * (to store 16 bytes key in addition to the 12 bytes verifier).
     * Hence we allocate, the maximum of (12 * 2 + 1) and (16 + 12).
     */
    char verifier[KEY_LEN + VERIFIER_LEN];
    char* nonce;
    char* remVerifier;

    /*
     * Check we are in an auth conversation with the sender
     */
    status = CheckAuthPeer(msg);
    if (status != AJ_OK) {
        return status;
    }
    if (msg->hdr->msgType == AJ_MSG_ERROR) {
        status = AJ_ERR_SECURITY;
    } else {
        AJ_UnmarshalArgs(msg, "ss", &nonce, &remVerifier);
        status = KeyGen(msg->sender, AJ_ROLE_KEY_INITIATOR, authContext.nonce, nonce, (uint8_t*)verifier, sizeof(verifier));
        if (status == AJ_OK) {
            /*
             * Check verifier strings match as expected
             */
            if (strcmp(remVerifier, verifier) != 0) {
                status = AJ_ERR_SECURITY;
            }
        }
        if (status == AJ_OK) {
            AJ_Arg key;
            AJ_Message call;
            uint8_t groupKey[KEY_LEN];
            /*
             * Group keys are exchanged via an encrypted message
             */
            AJ_MarshalMethodCall(msg->bus, &call, AJ_METHOD_EXCHANGE_GROUP_KEYS, msg->sender, 0, AJ_FLAG_ENCRYPTED, CALL_TIMEOUT);
            AJ_GetGroupKey(NULL, groupKey);
            AJ_MarshalArg(&call, AJ_InitArg(&key, AJ_ARG_BYTE, AJ_ARRAY_FLAG, groupKey, sizeof(groupKey)));
            status = AJ_DeliverMsg(&call);
        }
    }
    if (status != AJ_OK) {
        PeerAuthComplete(status);
    }
    return AJ_OK;
}

AJ_Status AJ_PeerHandleExchangeGroupKeysReply(AJ_Message* msg)
{
    AJ_Status status;
    AJ_Arg arg;

    /*
     * Check we are in an auth conversation with the sender
     */
    status = CheckAuthPeer(msg);
    if (status != AJ_OK) {
        return status;
    }
    AJ_UnmarshalArg(msg, &arg);
    /*
     * We expect the key to be 16 bytes
     */
    if (arg.len != KEY_LEN) {
        status = AJ_ERR_INVALID;
    } else {
        status = AJ_SetGroupKey(msg->sender, arg.val.v_byte);
    }
    PeerAuthComplete(status);
    return AJ_OK;
}
