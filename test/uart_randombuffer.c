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

#define B115200 111520
#define BITRATE B115200
#define AJ_SERIAL_WINDOW_SIZE   4
#define AJ_SERIAL_ENABLE_CRC    1
#define LOCAL_DATA_PACKET_SIZE  100
#define AJ_SERIAL_PACKET_SIZE  LOCAL_DATA_PACKET_SIZE
#define RANDOM_BYTES_MAX 5000

#ifdef ECHO
static uint8_t txBuffer[RANDOM_BYTES_MAX];
#endif
static uint8_t rxBuffer[RANDOM_BYTES_MAX];



int AJ_Main()
{
    AJ_Status status;

    status = AJ_SerialInit("/dev/ttyUSB1", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_PACKET_SIZE);
    AJ_Printf("serial init was %u\n", status);
    uint16_t txlen;
    uint16_t rxlen;
    int i = 0;

#ifdef ECHO
    while (1) {
        AJ_Printf("Iteration %d\n", i++);
        status = AJ_SerialRecv(rxBuffer, RANDOM_BYTES_MAX, 5000, &rxlen);
        if (status == AJ_ERR_TIMEOUT) {
            continue;
        }
        if (status != AJ_OK) {
            AJ_Printf("AJ_SerialRecv returned %d\n", status);
            exit(1);
        }
        AJ_Sleep(rand() % 5000);

        status = AJ_SerialSend(rxBuffer, rxlen);
        if (status != AJ_OK) {
            AJ_Printf("AJ_SerialSend returned %d\n", status);
            exit(1);
        }

        AJ_Sleep(rand() % 5000);
    }
#else
    txlen = 0;
    while (1) {
        AJ_Printf("Iteration %d\n", i++);
        txlen = rand() % 5000;
        for (int i = 0; i < txlen; i++) {
            txBuffer[i] = rand() % 256;
            rxBuffer[i] = 1;
        }
        status = AJ_SerialSend(txBuffer, txlen);
        if (status != AJ_OK) {
            AJ_Printf("AJ_SerialSend returned %d\n", status);
            exit(1);
        }
        AJ_Sleep(rand() % 5000);
        status = AJ_SerialRecv(rxBuffer, txlen, 50000, &rxlen);
        if (status != AJ_OK) {
            AJ_Printf("AJ_SerialRecv returned %d\n", status);
            exit(1);
        }
        if (rxlen != txlen) {
            AJ_Printf("Failed: length match rxlen=%d txlen=%d.\n", rxlen, txlen);
            exit(-1);
        }
        if (0 != memcmp(txBuffer, rxBuffer, rxlen)) {
            AJ_Printf("Failed: buffers match.\n");
            exit(-1);
        }

    }
#endif
    return(0);
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
