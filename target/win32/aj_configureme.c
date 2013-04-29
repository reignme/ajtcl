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
