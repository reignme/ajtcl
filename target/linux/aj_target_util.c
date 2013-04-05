/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "aj_host.h"
#include "aj_util.h"

AJ_Status AJ_SuspendWifi(uint32_t msec)
{
    return AJ_OK;
}

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

/*
 * get a line of input from the the file pointer (most likely stdin).
 * This will capture the the num-1 characters or till a newline character is
 * entered.
 *
 * @param[out] str a pointer to a character array that will hold the user input
 * @param[in]  num the size of the character array 'str'
 * @param[in]  fp  the file pointer the sting will be read from. (most likely stdin)
 *
 * @return returns the same string as 'str' if there has been a read error a null
 *                 pointer will be returned and 'str' will remain unchanged.
 */
char*AJ_GetLine(char*str, size_t num, void*fp)
{
    char*p = fgets(str, num, fp);

    if (p != NULL) {
        size_t last = strlen(str) - 1;
        if (str[last] == '\n') {
            str[last] = '\0';
        }
    }
    return p;
}

static uint8_t ioThreadRunning = FALSE;
static char cmdline[1024];
static uint8_t consumed = TRUE;
static pthread_t threadId;

void* RunFunc(void* threadArg)
{
    while (ioThreadRunning) {
        if (consumed) {
            AJ_GetLine(cmdline, sizeof(cmdline), stdin);
            consumed = FALSE;
        }
        AJ_Sleep(1000);
    }
    return 0;
}

uint8_t AJ_StartReadFromStdIn()
{
    int ret = 0;
    if (!ioThreadRunning) {
        ret = pthread_create(&threadId, NULL, RunFunc, NULL);
        if (ret != 0) {
            printf("Error: fail to spin a thread for reading from stdin\n");
        }
        ioThreadRunning = TRUE;
        return TRUE;
    }
    return FALSE;
}

char* AJ_GetCmdLine(char* buf, size_t num)
{
    if (!consumed) {
        strncpy(buf, cmdline, num);
        buf[num - 1] = '\0';
        consumed = TRUE;
        return buf;
    }
    return NULL;
}

uint8_t AJ_StopReadFromStdIn()
{
    void* exit_status;
    if (ioThreadRunning) {
        pthread_join(threadId, &exit_status);
        ioThreadRunning = FALSE;
        return TRUE;
    }
    return FALSE;
}