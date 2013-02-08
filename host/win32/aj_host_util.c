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

#include <windows.h>
#include <sys/timeb.h>
#include <stdio.h>

#include "aj_host.h"
#include "aj_util.h"

AJ_Status AJ_SuspendWifi(uint32_t msec)
{
    return AJ_OK;
}

void AJ_Sleep(uint32_t time)
{
    Sleep(time);
}

uint32_t AJ_GetElapsedTime(AJ_Time* timer, uint8_t cumulative)
{
    uint32_t elapsed;
    struct _timeb now;

    _ftime(&now);

    elapsed = (uint32_t)((1000 * (now.time - timer->seconds)) + (now.millitm - timer->milliseconds));
    if (!cumulative) {
        timer->seconds = (uint32_t)now.time;
        timer->milliseconds = (uint32_t)now.millitm;
    }
    return elapsed;
}

/*
 * Simulate the kind of pool-based allocation implemented on an RTOS
 */
typedef struct _MemPool {
    size_t size;
    size_t free;
} MemPool;

static MemPool memPools[] = {
    { 32,   1  },
    { 96,   4, },
    { 192,  1, }
};

void* AJ_Malloc(size_t sz)
{
    size_t* mem;
    size_t i;
    for (i = 0; i < ArraySize(memPools); ++i) {
        if ((sz <= memPools[i].size) && (memPools[i].free > 0)) {
            //printf("AJ_Malloc pool %d allocated %d\n", memPools[i].size, sz);
            --memPools[i].free;
            mem = (size_t*)malloc(sizeof(size_t) + sz);
            mem[0] = i;
            return &mem[1];
        }
    }
    printf("AJ_Malloc of %d bytes failed\n", sz);
    for (i = 0; i < ArraySize(memPools); ++i) {
        printf("    Pool %d free %d\n", memPools[i].size, memPools[i].free);
    }
    return NULL;
}

void AJ_Free(void* mem)
{
    if (mem) {
        size_t* m = (size_t*)mem - 1;
        //printf("AJ_Free pool %d\n", memPools[*m].size);
        ++memPools[*m].free;
        free(m);
    }
}
