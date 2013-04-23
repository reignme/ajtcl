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

#include <aj_configureme.h>
#include <aj_helper.h>


static AJ_Configuration Config;

#define CONFIG_SENTINEL 0xAABBCCDD

AJ_Configuration* AJ_InitializeConfig()
{
    memset(&Config, 0, sizeof(AJ_Configuration));
    Config.sentinel = CONFIG_SENTINEL;
    return &Config;
}

void AJ_WriteConfiguration(AJ_Configuration* config)
{
    memcpy(&Config, config, sizeof(AJ_Configuration));
}

void AJ_ClearAll()
{
    memset(&Config, 0, sizeof(AJ_Configuration));
}

const AJ_Configuration* AJ_GetConfiguration()
{
    AJ_Configuration* config = &Config;
    return (config->sentinel == CONFIG_SENTINEL ? config : NULL);
}
