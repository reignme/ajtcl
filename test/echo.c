/**
 * @file  UART transport Tester
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
#include "alljoyn.h"
#include "aj_util.h"
#include "aj_debug.h"
#include "aj_bufio.h"
#include "aj_serial.h"


#define BITRATE B115200
#define AJ_SERIAL_WINDOW_SIZE   4
#define AJ_SERIAL_ENABLE_CRC    1
#define AJ_SERIAL_PACKET_SIZE  104

static uint8_t txBuffer[32];
static uint8_t rxBuffer[32];


void TimerCallbackEndProc(uint32_t timerId, void* context)
{
    AJ_Printf("TimerCallback %.6d \n", timerId);
    exit(0);
}


#ifdef AJ_MAIN
int main()
{
    AJ_Status status;
    memset(&rxBuffer, 'R', sizeof(rxBuffer));

#ifdef READTEST
    status = AJ_SerialInit("/dev/ttyUSB0", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_ENABLE_CRC, AJ_SERIAL_PACKET_SIZE);
#else
    status = AJ_SerialInit("/dev/ttyUSB1", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_ENABLE_CRC, AJ_SERIAL_PACKET_SIZE);
#endif

    AJ_Printf("serial init was %u\n", status);

    uint32_t timerEndProc = 9999;
    status = AJ_TimerRegister(10000, &TimerCallbackEndProc, NULL, &timerEndProc);
    AJ_Printf("Added id %u\n", timerEndProc);




    uint16_t echocount = 0;
    while (1) {
        snprintf((char*)&txBuffer, sizeof(txBuffer), "echo t %i", ++echocount);

#ifdef READTEST
        uint16_t recv;
        AJ_SerialRecv(rxBuffer, sizeof(rxBuffer), 2000, &recv);
        AJ_DumpBytes("Post serial recv", rxBuffer, sizeof(rxBuffer));

#else
        AJ_Sleep(500);
        AJ_SerialSend(txBuffer, sizeof(txBuffer));
        AJ_Printf("post serial send\n");
#endif

        AJ_Sleep(400);
    }


    return(0);
}
#endif
