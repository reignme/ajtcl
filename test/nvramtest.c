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

#include <alljoyn.h>
#include <aj_creds.h>
#include <aj_nvram.h>

AJ_Status TestNVRAM();
AJ_Status TestCreds();
extern void AJ_NVRAM_Layout_Print();

AJ_Status TestCreds()
{
    AJ_Status status = AJ_OK;
    AJ_GUID localGuid;
    AJ_GUID remoteGuid;
    char str[33];
    AJ_PeerCred peerCred;
    AJ_PeerCred peerCredRead;
    int i = 0;
    status = AJ_GetLocalGUID(&localGuid);
    if (AJ_OK != status) {
        goto TEST_CREDS_EXIT;
    }
    AJ_GUID_FromString(&localGuid, str);

    AJ_NVRAM_Layout_Print();
    memset(&peerCred.guid, 1, sizeof(AJ_GUID));
    memcpy(&remoteGuid, &peerCred.guid, sizeof(AJ_GUID)); // backup the GUID
    for (i = 0; i < 24; i++) {
        peerCred.secret[i] = i;
    }
    status = AJ_StoreCredential(&peerCred);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
        goto TEST_CREDS_EXIT;
    }

    status = AJ_GetRemoteCredential(&remoteGuid, &peerCredRead);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
        goto TEST_CREDS_EXIT;
    }

    if (0 != memcmp(&peerCredRead, &peerCred, sizeof(AJ_PeerCred))) {
        AJ_Printf("The retrieved credential does not match\n");
        status = AJ_ERR_FAILURE;
        goto TEST_CREDS_EXIT;
    }

    status = AJ_DeleteCredential(&remoteGuid);
    if (AJ_OK != status) {
        AJ_Printf("AJ_DeleteCredential failed = %d\n", status);
        goto TEST_CREDS_EXIT;
    }

    if (AJ_ERR_FAILURE == AJ_GetRemoteCredential(&remoteGuid, &peerCredRead)) {
        status = AJ_OK;
    } else {
        status = AJ_ERR_FAILURE;
        goto TEST_CREDS_EXIT;
    }
    AJ_NVRAM_Layout_Print();

    status = AJ_StoreCredential(&peerCred);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
        goto TEST_CREDS_EXIT;
    }

    AJ_ClearCredentials();
    if (AJ_ERR_FAILURE == AJ_GetRemoteCredential(&remoteGuid, &peerCredRead)) {
        status = AJ_OK;
    } else {
        status = AJ_ERR_FAILURE;
        goto TEST_CREDS_EXIT;
    }
    AJ_NVRAM_Layout_Print();

TEST_CREDS_EXIT:
    return status;

}

AJ_Status TestNVRAM()
{
    uint16_t id = 16;
    AJ_NV_DATASET* handle = NULL;
    int i = 0;
    size_t bytes = 0;
    AJ_Status status = AJ_OK;
    AJ_NVRAM_Layout_Print();

    {
        handle = AJ_NVRAM_Open(id, "w", 40 + 5);
        AJ_ASSERT(handle);
        for (i = 0; i < 10; i++) {
            bytes = AJ_NVRAM_Write(&i, sizeof(i), handle);
            if (bytes != sizeof(i)) {
                status = AJ_ERR_FAILURE;
                goto _TEST_NVRAM_EXIT;
            }
        }
        {
            uint8_t buf[3] = { 11, 22, 33 };
            uint8_t buf2[2] = { 44, 55 };
            bytes = AJ_NVRAM_Write(buf, sizeof(buf), handle);
            if (bytes != sizeof(buf)) {
                status = AJ_ERR_FAILURE;
                goto _TEST_NVRAM_EXIT;
            }
            bytes = AJ_NVRAM_Write(buf2, sizeof(buf2), handle);
            if (bytes != sizeof(buf2)) {
                status = AJ_ERR_FAILURE;
                goto _TEST_NVRAM_EXIT;
            }

        }
        AJ_NVRAM_Close(handle);
        AJ_NVRAM_Layout_Print();

        handle = AJ_NVRAM_Open(id, "r", 0);
        AJ_ASSERT(handle);
        for (i = 0; i < 10; i++) {
            int data = 0;
            bytes = AJ_NVRAM_Read(&data, sizeof(data), handle);
            if (bytes != sizeof(data) || data != i) {
                status = AJ_ERR_FAILURE;
                goto _TEST_NVRAM_EXIT;
            }
        }
        for (i = 1; i < 6; i++) {
            uint8_t data = 0;
            AJ_NVRAM_Read(&data, 1, handle);
            if (data != i * 11) {
                status = AJ_ERR_FAILURE;
                goto _TEST_NVRAM_EXIT;
            }
        }
        AJ_NVRAM_Close(handle);
    }

    if (AJ_NVRAM_Exist(id + 1)) {
        AJ_ASSERT(AJ_NVRAM_Delete(id + 1) == AJ_OK);
    }

    // Force storage compaction
    for (i = 0; i < 12; i++) {
        if (i == 6) {
            handle = AJ_NVRAM_Open(id + 2, "w", 100);
            AJ_ASSERT(handle);
            status = AJ_NVRAM_Close(handle);
            if (AJ_OK != status) {
                goto _TEST_NVRAM_EXIT;
            }
            continue;
        }
        handle = AJ_NVRAM_Open(id + 1, "w", 200);
        AJ_ASSERT(handle);
        status = AJ_NVRAM_Close(handle);
        if (AJ_OK != status) {
            goto _TEST_NVRAM_EXIT;
        }
        AJ_NVRAM_Layout_Print();
    }

_TEST_NVRAM_EXIT:
    return status;
}

int AJ_Main()
{
    AJ_Status status = AJ_OK;
    AJ_Printf("AJ_Main 1\n");
    AJ_Initialize();
    AJ_Printf("AJ_Main 2\n");
    status = TestNVRAM();
    AJ_Printf("AJ_Main 3\n");
    AJ_ASSERT(status == AJ_OK);
    status = TestCreds();
    AJ_ASSERT(status == AJ_OK);
    return 0;
}

#ifdef AJ_YIELD
extern AJ_MainRoutineType AJ_MainRoutine;

int main()
{
    AJ_MainRoutine = AJ_Main;

    while (1) {
        AJ_Loop();
        if (AJ_GetEventState(AJWAITEVENT_EXIT)) {
            return(0); // got the signal, so exit the app.
        }
    }
}
#else
#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
#endif
