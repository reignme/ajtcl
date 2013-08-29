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
#define LOCAL_DATA_PACKET_SIZE  100
#define AJ_SERIAL_PACKET_SIZE  LOCAL_DATA_PACKET_SIZE + AJ_SERIAL_HDR_LEN

static uint8_t txBuffer[1600];
static uint8_t rxBuffer[1600];


void TimerCallbackEndProc(uint32_t timerId, void* context)
{
    AJ_Printf("TimerCallbackEndProc %.6d \n", timerId);

#ifdef READTEST
    if (0 == memcmp(txBuffer, rxBuffer, sizeof(rxBuffer))) {
        AJ_Printf("Passed: buffers match.\n");
    } else {
        AJ_Printf("FAILED: buffers mismatch.\n");
        exit(-1);
    }
#endif
    exit(0);
}


#ifdef AJ_MAIN
int main()
{
    AJ_Status status;
    memset(&txBuffer, 'T', sizeof(txBuffer));
    memset(&rxBuffer, 'R', sizeof(rxBuffer));

    int blocks;
    int blocksize = LOCAL_DATA_PACKET_SIZE;
    for (blocks = 0; blocks < 16; blocks++) {
        memset(txBuffer + (blocks * blocksize), 0x41 + (uint8_t)blocks, blocksize);
    }

#ifdef READTEST
    status = AJ_SerialInit("/dev/ttyUSB0", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_ENABLE_CRC, AJ_SERIAL_PACKET_SIZE);
#else
    status = AJ_SerialInit("/dev/ttyUSB1", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_ENABLE_CRC, AJ_SERIAL_PACKET_SIZE);
#endif

    AJ_Printf("serial init was %u\n", status);

    uint32_t timerEndProc = 9999;
    status = AJ_TimerRegister(20000, &TimerCallbackEndProc, NULL, &timerEndProc);
    AJ_Printf("Added id %u\n", timerEndProc);


#ifdef READTEST

    //Read small chunks of a packet at one time.
    for (blocks = 0; blocks < 16 * 4; blocks++) {
        fflush(NULL);
        AJ_SerialRecv(rxBuffer + (blocks * blocksize / 4), blocksize / 4, 2000, NULL);
        AJ_Sleep(200);
    }

    AJ_DumpBytes("Post serial recv", rxBuffer, sizeof(rxBuffer));

#else
    AJ_Sleep(500);
    AJ_SerialSend(txBuffer, sizeof(txBuffer));

//    for (blocks = 0 ; blocks < 16; blocks++) {
//        AJ_SerialSend(txBuffer+(blocks*blocksize), blocksize);
//    }
    AJ_Printf("post serial send\n");
#endif


    while (1) {
        usleep(1000);
    }


    return(0);
}
#endif
