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

#include "alljoyn.h"
#include "aj_debug.h"

static const char ServiceName[] = "org.alljoyn.ajlite";

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)

int AJ_Main()
{
    AJ_Status status;
    AJ_BusAttachment bus;

    AJ_Initialize();

    status = AJ_Connect(&bus, NULL, CONNECT_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_BusRequestName(&bus, ServiceName, AJ_NAME_REQ_DO_NOT_QUEUE);
    }
    while (status == AJ_OK) {
        AJ_Message msg;
        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status == AJ_OK) {
            status = AJ_BusHandleBusMessage(&msg);
        }
        AJ_CloseMsg(&msg);
    }

    return 0;
}

#ifdef AJ_YIELD
extern AJ_MainRoutineType AJ_MainRoutine;

int main()
{
    AJ_MainRoutine = AJ_Main;

    while (1) {
        AJ_Loop();
        if (AJ_GetEventState(AJWAITEVENT_EXIT)) {
            return(0); // got the signal, so exit the app.
        }
    }
}
#else
#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
#endif