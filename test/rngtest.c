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

#include "alljoyn.h"
#include "aj_debug.h"

#include "efm32_adc.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "dmactrl.h"
#include "efm32_dma.h"

/*
 * Let the application do some work
 */
static void AppDoWork(AJ_BusAttachment* bus)
{
}

extern int GatherBits(uint8_t* buffer, uint32_t len);

int AJ_Main(void)
{
    int i, j, k, value;
    char buffer;

    for (i = 0; i < 1000000; ++i) {
        value = 0;
        for (j = 0; j < 32; ++j) {
            value <<= 1;
            value |= GatherBits(&buffer, sizeof(buffer)) & 1;
        }
        printf("%d\n", value);
    }
    return 0;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
