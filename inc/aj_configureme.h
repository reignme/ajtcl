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

#ifndef _AJ_CONFIGUREME_H
#define _AJ_CONFIGUREME_H

#include <aj_host.h>
#include <aj_status.h>



#define MAX_PROFILES 1


#define PROFILE_TYPE_WIFI 1
#define SSID_LEN 32
#define PASS_LEN 32

#define PROFILE_TYPE_UNDEFINED 0xFFFFFFFF

typedef struct {
    char ssid[SSID_LEN];
    uint32_t auth;
    uint32_t encryption;
    uint32_t password_len;
    char password[PASS_LEN];
} AJ_WifiProfile;


typedef struct {
    uint32_t type;
    // this union will store other profile types for other types of networking
    union {
        AJ_WifiProfile wifi;
    };
} AJ_ConnectionProfile;


typedef struct {
    uint32_t sentinel;
    uint32_t active;
    char aj_password[32];
    AJ_ConnectionProfile profiles[MAX_PROFILES];
} AJ_Configuration;


typedef void (*IdentifyFunction)(char*, size_t);

/**
 * Run the ConfigureMe service until the user calls Save
 *
 * @return  AJ_OK if configured
 */
AJ_Status AJ_RunConfigureMe();

/**
 * Read the configuration from NVRAM
 *
 * @return       A const pointer to the Configuration; NULL if not initialized
 */
const AJ_Configuration* AJ_GetConfiguration();

/**
 * Set the preferred connection profile
 * @param index the index to set active
 * @return AJ_OK if success
 */
AJ_Status AJ_SetActive(uint32_t index);


uint32_t AJ_GetActive();


AJ_Status AJ_SaveWifiProfile(uint32_t index, char* ssid, char* password, uint32_t auth, uint32_t encryption);


/**
 * Read a profile from NVRAM
 *
 * @param index  The index of the profile to read
 * @return       A const pointer to the profile; NULL if not initialized
 */
const AJ_ConnectionProfile* AJ_ReadProfile(uint32_t index);


/**
 * Clears the configuration specified
 * @param index  the index of the config to clear
 */
void AJ_ClearConfig(uint32_t index);

/**
 * Reset the NVRAM Configuration
 */
void AJ_ClearAll();

#endif /* _AJ_CONFIGUREME_H */
