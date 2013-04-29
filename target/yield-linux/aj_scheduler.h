#ifndef _AJ_SCHEDULER_H
#define _AJ_SCHEDULER_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "aj_status.h"

typedef struct _AJ_Event {
    uint8_t isSet;
    uint32_t data;
} AJ_Event;


#define AJWAITEVENT_READ      0x1
#define AJWAITEVENT_WRITE     0x2
#define AJWAITEVENT_TIMER     0x4
#define AJWAITEVENT_EXIT      0x8
#define AJWAITEVENT_ALWAYSSET 0x10

void AJ_Schedule(uint32_t bitFlag);
AJ_Status AJ_YieldUntil(uint32_t bitFlag);
AJ_Status AJ_ClearEvents(uint32_t bitFlag);
int AJ_GetEventState(uint32_t bitFlag);

typedef int (*AJ_MainRoutineType)(void);

void CallAllJoyn();
int AJ_Loop();

#endif
