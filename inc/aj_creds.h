#ifndef _AJ_CREDS_H
#define _AJ_CREDS_H
/**
 * @file aj_creds.h
 * @defgroup aj_creads Credentials Management
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
#include "aj_guid.h"
#include "aj_status.h"

/**
 * Maximum number of different peers for which we can store credentials.
 */
#define AJ_MAX_PEER_GUIDS  12

/**
 * Credentials for a remote peer
 */
typedef struct _AJ_PeerCred {
    AJ_GUID guid;        /**< GUID for the peer */
    uint8_t secret[24];  /**< secret keying data */
} AJ_PeerCred;

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
 * Get the credentials for a specific remote peer from NVRAM
 *
 * @param peerGuid  The GUID for the remote peer.
 * @param peerCred  Pointer to a bufffer that has enough space to store the credentials for a specific remote peer identified by a GUID
 *
 * @return  AJ_OK if the credentials for the specific remote peer exist and are copied into the buffer
 *          AJ_ERR_FAILURE otherwise.
 */
AJ_Status AJ_GetRemoteCredential(const AJ_GUID* peerGuid, AJ_PeerCred* peerCred);

/**
 * Get the GUID for this peer. If this is the first time the GUID has been requested this function
 * will generate the GUID and store it in NVRAM
 *
 * @param localGuid Pointer to a bufffer that has enough space to store the local GUID
 *
 * @return  AJ_OK if the local GUID is copied into the buffer.
 */
AJ_Status AJ_GetLocalGUID(AJ_GUID* localGuid);

/**
 * @}
 */
#endif
