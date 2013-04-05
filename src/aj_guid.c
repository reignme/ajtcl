/**
 * @file
 */
/******************************************************************************
 * Copyright 2012, 2013, Qualcomm Innovation Center, Inc.
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
#include "aj_util.h"
#include "aj_crypto.h"

#define NAME_MAP_GUID_SIZE   2
#define MAX_NAME_SIZE       14

typedef struct _NameToGUID {
    uint8_t keyRole;
    char uniqueName[MAX_NAME_SIZE + 1];
    const char* serviceName;
    AJ_GUID guid;
    uint8_t sessionKey[16];
    uint8_t groupKey[16];
} NameToGUID;

static uint8_t localGroupKey[16];

static NameToGUID nameMap[NAME_MAP_GUID_SIZE];

AJ_Status AJ_GUID_ToString(const AJ_GUID* guid, char* buffer, uint32_t bufLen)
{
    return AJ_RawToHex(guid->val, 16, buffer, bufLen);
}

AJ_Status AJ_GUID_FromString(AJ_GUID* guid, const char* str)
{
    return AJ_HexToRaw(str, 32, guid->val, 16);
}

static NameToGUID* LookupName(const char* name)
{
    uint32_t i;
    for (i = 0; i < NAME_MAP_GUID_SIZE; ++i) {
        if (strcmp(nameMap[i].uniqueName, name) == 0) {
            return &nameMap[i];
        }
        if (nameMap[i].serviceName && (strcmp(nameMap[i].serviceName, name)) == 0) {
            return &nameMap[i];
        }
    }
    return NULL;
}

AJ_Status AJ_GUID_AddNameMapping(const AJ_GUID* guid, const char* uniqueName, const char* serviceName)
{
    size_t len = strlen(uniqueName);

    NameToGUID* mapping = LookupName(uniqueName);
    if (!mapping) {
        mapping = LookupName("");
    }
    if (mapping && (len <= MAX_NAME_SIZE)) {
        memcpy(&mapping->guid, guid, sizeof(AJ_GUID));
        memcpy(&mapping->uniqueName, uniqueName, len + 1);
        mapping->serviceName = serviceName;
        return AJ_OK;
    } else {
        return AJ_ERR_RESOURCES;
    }
}

void AJ_GUID_DeleteNameMapping(const char* uniqueName)
{
    NameToGUID* mapping = LookupName(uniqueName);
    if (mapping) {
        memset(mapping, 0, sizeof(NameToGUID));
    }
}

const AJ_GUID* AJ_GUID_Find(const char* name)
{
    NameToGUID* mapping = LookupName(name);
    return mapping ? &mapping->guid : NULL;
}


void AJ_GUID_ClearNameMap(void)
{
    memset(nameMap, 0, sizeof(nameMap));
}

AJ_Status AJ_SetGroupKey(const char* uniqueName, const uint8_t* key)
{
    NameToGUID* mapping = LookupName(uniqueName);
    if (mapping) {
        memcpy(mapping->groupKey, key, 16);
        return AJ_OK;
    } else {
        return AJ_ERR_NO_MATCH;
    }
}

AJ_Status AJ_SetSessionKey(const char* uniqueName, const uint8_t* key, uint8_t role)
{
    NameToGUID* mapping = LookupName(uniqueName);
    if (mapping) {
        mapping->keyRole = role;
        memcpy(mapping->sessionKey, key, 16);
        return AJ_OK;
    } else {
        return AJ_ERR_NO_MATCH;
    }
}

AJ_Status AJ_GetSessionKey(const char* name, uint8_t* key, uint8_t* role)
{
    NameToGUID* mapping = LookupName(name);
    if (mapping) {
        *role = mapping->keyRole;
        memcpy(key, mapping->sessionKey, 16);
        return AJ_OK;
    } else {
        return AJ_ERR_NO_MATCH;
    }
}

AJ_Status AJ_GetGroupKey(const char* name, uint8_t* key)
{
    if (name) {
        NameToGUID* mapping = LookupName(name);
        if (!mapping) {
            return AJ_ERR_NO_MATCH;
        }
        memcpy(key, mapping->groupKey, 16);
    } else {
        /*
         * Check if the group key needs to be initialized
         */
        memset(key, 0, 16);
        if (memcmp(localGroupKey, key, 16) == 0) {
            AJ_RandBytes(localGroupKey, 16);
        }
        memcpy(key, localGroupKey, 16);
    }
    return AJ_OK;
}
