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
#include "aj_init.h"
#include "aj_nvram.h"
#include "aj_creds.h"
#include "aj_guid.h"

static uint8_t initialized = FALSE;

void AJ_Initialize(void)
{
    if (!initialized) {
        AJ_GUID localGuid;
        initialized = TRUE;
        AJ_NVRAM_Init();

        AJ_GetLocalGUID(&localGuid);
    }
}
