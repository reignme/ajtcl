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

void TestNVRAM();
void TestCreds();
extern void AJ_NVRAM_Layout_Print();
int main()
{
    AJ_Initialize();
    TestNVRAM();
    TestCreds();
}

void TestCreds()
{
    AJ_Status status = AJ_OK;
    AJ_GUID localGuid;
    AJ_GUID remoteGuid;
    char str[33];
    AJ_PeerCred peerCred;
    AJ_PeerCred peerCred2;
    int i = 0;
    AJ_GetLocalGUID(&localGuid);
    AJ_GUID_FromString(&localGuid, str);

    AJ_NVRAM_Layout_Print();

    memset(&peerCred.guid, 1, sizeof(AJ_GUID));
    memcpy(&remoteGuid, &peerCred.guid, sizeof(AJ_GUID));
    for (i = 0; i < 24; i++) {
        peerCred.secret[i] = i;
    }
    status = AJ_StoreCredential(&peerCred);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
    }

    status = AJ_GetRemoteCredential(&remoteGuid, &peerCred2);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
    }

    if (0 != memcmp(&peerCred2, &peerCred, sizeof(AJ_PeerCred))) {
        AJ_Printf("The retrieved credential does not match\n");
    }

    AJ_DeleteCredential(&remoteGuid);

    status = AJ_GetRemoteCredential(&remoteGuid, &peerCred2);
    assert(status == AJ_ERR_FAILURE);
    AJ_NVRAM_Layout_Print();

    status = AJ_StoreCredential(&peerCred);
    if (AJ_OK != status) {
        AJ_Printf("AJ_StoreCredential failed = %d\n", status);
    }
    AJ_ClearCredentials();
    status = AJ_GetRemoteCredential(&remoteGuid, &peerCred2);
    assert(status == AJ_ERR_FAILURE);
    AJ_NVRAM_Layout_Print();

}

void TestNVRAM()
{
    uint16_t id = 16;
    AJ_NV_FILE* handle = NULL;
    int i = 0;
    AJ_NVRAM_Layout_Print();
    for (i = 0; i < 3; i++) {
        AJ_Printf("ID = %d exist = %d\n", (id + i), AJ_NVRAM_Exist(id + i));
    }

    handle = AJ_NVRAM_Open(id, "w");
    assert(handle);
    for (i = 1; i <= 20; i++) {
        size_t written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written\n", written);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id, "r");
    assert(handle);
    printf("total bytes = %d\n", AJ_NVRAM_Size(handle));
    for (i = 0; i < 10; i++) {
        int data;
        size_t read = AJ_NVRAM_Read(&data, sizeof(data), handle);
        printf(" %d bytes read =  %d \n", read, data);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id, "w");
    assert(handle);
    for (i = 0; i < 10; i++) {
        size_t written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written\n", written);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id + 1, "a+");
    assert(handle);
    for (i = 1; i <= 2; i++) {
        size_t written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written\n", written);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id, "a");
    assert(handle);
    for (i = 0; i < 10; i++) {
        size_t written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written =  %d \n", written, i);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id + 2, "w");
    assert(handle);
    {
        uint32_t data = 9;
        size_t written = AJ_NVRAM_Write(&data, 7, handle);
        printf(" %d bytes written\n", written);
    }
    printf("The data set size = %d\n", AJ_NVRAM_Size(handle));
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    for (i = 0; i < 10; i++) {
        size_t written = 0;
        handle = AJ_NVRAM_Open(id, "a");
        assert(handle);
        written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written =  %d \n", written, i);
        AJ_NVRAM_Close(handle);
        AJ_NVRAM_Layout_Print();
    }

    handle = AJ_NVRAM_Open(id, "r+");
    for (i = 0; i < 20; i++) {
        int data;
        size_t read = AJ_NVRAM_Read(&data, sizeof(data), handle);
        printf(" %d bytes read =  %d \n", read, data);
    }
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();

    handle = AJ_NVRAM_Open(id, "w+");
    assert(handle);
    for (i = 0; i < 10; i++) {
        size_t written = AJ_NVRAM_Write(&i, sizeof(i), handle);
        printf(" %d bytes written\n", written);
    }
    printf("The data set size = %d\n", AJ_NVRAM_Size(handle));
    AJ_NVRAM_Close(handle);
    AJ_NVRAM_Layout_Print();
}
