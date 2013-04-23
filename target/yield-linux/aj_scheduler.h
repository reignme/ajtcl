#ifndef _AJ_SCHEDULER_H
#define _AJ_SCHEDULER_H
/**
 * @file
 */
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
