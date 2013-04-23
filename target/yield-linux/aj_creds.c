/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include <stdio.h>

#include "aj_target.h"
#include "aj_creds.h"
#include "aj_status.h"
#include "aj_crypto.h"

static AJ_Credentials credentials;

static const AJ_GUID InvalidGUID = { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

AJ_Status WriteCreds()
{
    FILE* f = fopen("ajlite.creds", "w");
    if (f) {
        fwrite(&credentials, sizeof(credentials), 1, f);
        fclose(f);
    }
    return AJ_OK;
}

AJ_Status ReadCreds()
{
    size_t num_items_read = 0;
    FILE* f = fopen("ajlite.creds", "r");
    if (f) {
        num_items_read = fread(&credentials, sizeof(credentials), 1, f);
        fclose(f);
        return (1 == num_items_read) ? AJ_OK : AJ_ERR_READ;
    } else {
        return AJ_ERR_READ;
    }
}

AJ_Status AJ_StoreCredential(AJ_PeerCred* peerCred)
{
    const AJ_PeerCred* cred = AJ_GetRemoteCredential(&peerCred->guid);
    if (!cred) {
        /*
         * Find an empty slot
         */
        cred = AJ_GetRemoteCredential(&InvalidGUID);
        if (!cred) {
            return AJ_ERR_RESOURCES;
        }
    }
    memcpy((void*)cred, peerCred, sizeof(AJ_PeerCred));
    return WriteCreds();
}

AJ_Status AJ_DeleteCredential(const AJ_GUID* peerGuid)
{
    const AJ_PeerCred* cred = AJ_GetRemoteCredential(peerGuid);
    if (cred) {
        memset((void*)cred, 0xFF, sizeof(AJ_PeerCred));
        return WriteCreds();
    } else {
        return AJ_OK;
    }
}

const AJ_GUID* AJ_GetLocalGUID(void)
{
    AJ_Status status = ReadCreds();
    if (status == AJ_ERR_READ) {
        AJ_RandBytes((uint8_t*)&credentials.guid, sizeof(AJ_GUID));
        memset(credentials.peers, 0xFF, sizeof(credentials.peers));
        WriteCreds();
    }
    return &credentials.guid;
}

/**
 * Get the credentials for a specific remote peer
 *
 * @return  Returns a pointer to the credentials for a specific remote peer identified by a GUID or
 *          NULL if there are no credentials stored for this peer.
 */
const AJ_PeerCred* AJ_GetRemoteCredential(const AJ_GUID* peerGuid)
{
    AJ_PeerCred* peer = credentials.peers;
    int i;
    for (i = 0; i < AJ_MAX_PEER_GUIDS; ++i, ++peer) {
        if (memcmp(peerGuid, &peer->guid, sizeof(AJ_GUID)) == 0) {
            return peer;
        }
    }
    return NULL;
}

void AJ_ClearCredentials(void)
{
    memset(credentials.peers, 0xFF, sizeof(credentials.peers));
    WriteCreds();
}
