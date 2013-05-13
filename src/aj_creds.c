/**
 * @file
 */
/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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
#include "aj_creds.h"
#include "aj_status.h"
#include "aj_crypto.h"
#include "aj_nvram.h"

#define AJ_LOCAL_GUID_NV_ID 1
#define AJ_REMOTE_CREDS_NV_ID_BEGIN (AJ_LOCAL_GUID_NV_ID + 1)
#define AJ_REMOTE_CREDS_NV_ID_END  (AJ_REMOTE_CREDS_NV_ID_BEGIN + 12)

uint16_t FindCredsEmptySlot()
{
    uint16_t id = AJ_REMOTE_CREDS_NV_ID_BEGIN;
    for (; id < AJ_REMOTE_CREDS_NV_ID_END; id++) {
        if (!AJ_NVRAM_Exist(id)) {
            return id;
        }
    }
    return 0;
}

uint16_t FindCredsByGUID(const AJ_GUID* peerGuid)
{
    uint16_t id = AJ_REMOTE_CREDS_NV_ID_BEGIN;
    AJ_NV_FILE* handle;
    for (; id < AJ_REMOTE_CREDS_NV_ID_END; id++) {
        if (AJ_NVRAM_Exist(id)) {
            handle = AJ_NVRAM_Open(id, "r");
            if (!handle) {
                AJ_Printf("Error: fail to open data set with id = %d\n", id);
            } else {
                AJ_GUID guid;
                if (sizeof(AJ_GUID) != AJ_NVRAM_Read(&guid, sizeof(AJ_GUID), handle)) {
                    AJ_Printf("Error: fail to read %zu bytes from data set with id = %d\n", sizeof(AJ_GUID), id);
                    AJ_NVRAM_Close(handle);
                    continue;
                }
                if (memcmp(peerGuid, &guid, sizeof(AJ_GUID)) == 0) {
                    AJ_NVRAM_Close(handle);
                    return id;
                }
                AJ_NVRAM_Close(handle);
            }
        }
    }
    return 0;
}

AJ_Status UpdatePeerCreds(AJ_PeerCred* peerCred, uint16_t id)
{
    AJ_Status status = AJ_OK;
    AJ_NV_FILE* handle = AJ_NVRAM_Open(id, "w");
    if (!handle) {
        AJ_Printf("Error: fail to open data set with id = %d\n", id);
        status = AJ_ERR_FAILURE;
    } else {
        if (peerCred) {
            if (sizeof(AJ_PeerCred) != AJ_NVRAM_Write(peerCred, sizeof(AJ_PeerCred), handle)) {
                AJ_Printf("Error: fail to read %zu bytes from data set with id = %d\n", sizeof(AJ_PeerCred), id);
                status = AJ_ERR_FAILURE;
            }
        }
        AJ_NVRAM_Close(handle);
    }
    return status;
}

/**
 * Write a credential to a free slot in NVRAM
 */
AJ_Status AJ_StoreCredential(AJ_PeerCred* peerCred)
{
    AJ_Status status = AJ_OK;
    uint16_t id = FindCredsByGUID(&peerCred->guid);
    if (!id) {
        id = FindCredsEmptySlot();
    }

    if (!id) {
        AJ_ClearCredentials();
        status = AJ_ERR_RESOURCES;
    } else {
        status = UpdatePeerCreds(peerCred, id);
    }
    return status;
}

AJ_Status AJ_DeleteCredential(const AJ_GUID* peerGuid)
{
    uint16_t id = FindCredsByGUID(peerGuid);
    if (id > 0) {
        AJ_NV_FILE* handle = AJ_NVRAM_Open(id, "w");
        if (handle) {
            AJ_NVRAM_Close(handle);
        }
    }
    return AJ_OK;
}

AJ_Status AJ_GetLocalGUID(AJ_GUID* localGuid)
{
    AJ_Status status = AJ_ERR_FAILURE;
    AJ_NV_FILE* handle;
    if (AJ_NVRAM_Exist(AJ_LOCAL_GUID_NV_ID)) {
        handle = AJ_NVRAM_Open(AJ_LOCAL_GUID_NV_ID, "r");
        if (handle) {
            AJ_ASSERT(sizeof(AJ_GUID) == AJ_NVRAM_Read(localGuid, sizeof(AJ_GUID), handle));
            AJ_NVRAM_Close(handle);
            status = AJ_OK;
        }
    } else {
        AJ_RandBytes((uint8_t*)localGuid, sizeof(AJ_GUID));
        handle = AJ_NVRAM_Open(AJ_LOCAL_GUID_NV_ID, "w");
        if (handle) {
            AJ_ASSERT(sizeof(AJ_GUID) == AJ_NVRAM_Write(localGuid, sizeof(AJ_GUID), handle));
            AJ_NVRAM_Close(handle);
            status = AJ_OK;
        }
    }
    return status;
}

AJ_Status AJ_GetRemoteCredential(const AJ_GUID* peerGuid, AJ_PeerCred* peerCreds)
{
    AJ_Status status = AJ_ERR_FAILURE;
    uint16_t id = FindCredsByGUID(peerGuid);
    if (id > 0) {
        AJ_NV_FILE* handle = AJ_NVRAM_Open(id, "r");
        if (handle) {
            AJ_ASSERT(sizeof(AJ_PeerCred) == AJ_NVRAM_Read(peerCreds, sizeof(AJ_PeerCred), handle));
            AJ_NVRAM_Close(handle);
            status = AJ_OK;
        }
    }
    return status;
}

void AJ_ClearCredentials(void)
{
    uint16_t id = AJ_REMOTE_CREDS_NV_ID_BEGIN;
    for (; id < AJ_REMOTE_CREDS_NV_ID_END; id++) {
        AJ_NV_FILE* hanle = AJ_NVRAM_Open(id, "w");
        AJ_NVRAM_Close(hanle);
    }
}
