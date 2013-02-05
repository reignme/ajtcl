/**
 * @file
 */
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

#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "aj_host.h"
#include "aj_util.h"


void AJ_Sleep(uint32_t time)
{
    usleep(1000 * time);
}

uint32_t AJ_GetElapsedTime(AJ_Time* timer, uint8_t cumulative)
{
    uint32_t elapsed;
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    elapsed = (1000 * (now.tv_sec - timer->seconds)) + ((now.tv_nsec / 1000000) - timer->milliseconds);
    if (!cumulative) {
        timer->seconds = now.tv_sec;
        timer->milliseconds = now.tv_nsec / 1000000;
    }
    return elapsed;
}

void* AJ_Malloc(size_t sz)
{
    return malloc(sz);
}

void AJ_Free(void* mem)
{
    if (mem) {
        free(mem);
    }
}
