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

#include <windows.h>
#include <process.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <assert.h>

#include "aj_target.h"
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
char*AJ_GetLine(char*str, int num, FILE*fp)
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

static boolean ioThreadRunning = FALSE;
static char cmdline[1024];
static const uint32_t stacksize = 80 * 1024;
static uint8_t consumed = TRUE;
static HANDLE handle;
static unsigned int threadId;

unsigned __stdcall RunFunc(void* threadArg)
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
    if (!ioThreadRunning) {
        handle = _beginthreadex(NULL, stacksize, &RunFunc, NULL, 0, &threadId);
        if (handle <= 0) {
            printf("Fail to spin a thread for reading from stdin\n");
            return FALSE;
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
    if (ioThreadRunning) {
        ioThreadRunning = FALSE;
        return TRUE;
    }
    return FALSE;
}
