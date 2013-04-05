#ifndef _AJ_CREDS_H
#define _AJ_CREDS_H
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

#include "aj_target.h"
#include "aj_guid.h"
#include "aj_status.h"

/**
 * Maximum number of different peers for which we can store credentials.
 */
#define AJ_MAX_PEER_GUIDS  12

/**
 * Base address of credential
 */
extern const uint32_t CREDS_BASE_ADDRESS;

/**
 * Credentials for a remote peer
 */
typedef struct _AJ_PeerCred {
    AJ_GUID guid;        /**< GUID for the peer */
    uint8_t secret[24];  /**< secret keying data */
} AJ_PeerCred;

/**
 * This needs to fit in 512 bytes
 */
typedef struct _AJ_Credentials {
    uint32_t sentinel;                     /**< Identifies a valid, initialized credentials block */
    AJ_GUID guid;                          /**< GUID of the local peer */
    AJ_PeerCred peers[AJ_MAX_PEER_GUIDS];  /**< Credentials of remote peers */
} AJ_Credentials;

/**
 * Write a peer credential to NVRAM
 *
 * @param peerCred  The credentials to write.
 *
 * @return
 *          - AJ_OK if the credentials were written.
 *          - AJ_ERR_RESOURCES if there is no space to write the credentials
 */
AJ_Status AJ_StoreCredential(AJ_PeerCred* peerCred);

/**
 * Delete a peer credential from NVRAM
 *
 * @param peerGuid  The guid for the peer that has credentials to delete.
 *
 * @return
 *          - AJ_OK if the credentials were deleted.
 */
AJ_Status AJ_DeleteCredential(const AJ_GUID* peerGuid);

/**
 * Clears all peer credentials.
 */
void AJ_ClearCredentials(void);

/**
 * Get the credentials for a specific remote peer
 *
 * @param peerGuid  The GUID for the remote peer.
 *
 * @return  Returns a pointer to the credentials for a specific remote peer identified by a GUID or
 *          NULL if there are no credentials stored for this peer.
 */
const AJ_PeerCred* AJ_GetRemoteCredential(const AJ_GUID* peerGuid);

/**
 * Get the GUID for this peer. If this is the first time the GUID has been requested this function
 * will generate the GUID and store it in NVRAM
 *
 * @return  Returns a pointer to the GUID for the local peer.
 */
const AJ_GUID* AJ_GetLocalGUID(void);

#endif
