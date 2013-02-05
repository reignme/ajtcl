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

#include <stdio.h>

#include "alljoyn.h"
#include "aj_debug.h"

#include "efm32_adc.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"

static const char ServiceName[] = "org.alljoyn.ajlite";
static const uint16_t ServicePort = 24;

static const char* testInterface[] = {
    "org.alljoyn.ajlite_test",
    "!ADC_Update >i",
    "!Gyro_Update >i >i",
    NULL
};


static const AJ_InterfaceDescription testInterfaces[] = {
    testInterface,
    NULL
};

/**
 * Objects implemented by the application
 */
static const AJ_Object AppObjects[] = {
    { "/org/alljoyn/ajlite_test", testInterfaces },
    { NULL }
};

#define APP_MY_ADC_SIGNAL        AJ_APP_MESSAGE_ID(0, 0, 0)
#define APP_MY_GYROSCOPE_SIGNAL  AJ_APP_MESSAGE_ID(0, 0, 1)


/*
 * Let the application do some work
 */
static void AppDoWork(AJ_BusAttachment* bus)
{
    static int savedValue = -1;

    ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
    ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

    int value;
    AJ_Message msg;
    AJ_Arg arg;

    GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1);
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_ADC0, true);

    init.timebase = ADC_TimebaseCalc(0);
    init.prescale = ADC_PrescaleCalc(7000000, 0);

    ADC_Init(ADC0, &init);

    singleInit.reference = adcRef1V25;
    singleInit.input = adcSingleInpCh5; /* According to Maui HW design */
    singleInit.resolution = adcRes8Bit;


    ADC_InitSingle(ADC0, &singleInit);
    /*
     * Setup the next acquisition
     */
    ADC_IntClear(ADC0, ADC_IF_SINGLE);
    ADC_Start(ADC0, adcStartSingle);
    /*
     * Wait for completion
     */
    while (!(ADC_IntGet(ADC0) & ADC_IF_SINGLE)) ;

    value = ADC_DataSingleGet(ADC0);
    GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 0);

    // send the message
    if (value != savedValue) {
        savedValue = value;
        AJ_MarshalSignal(bus, &msg, APP_MY_ADC_SIGNAL, NULL, 0, 0);
        AJ_MarshalArgs(&msg, "u", value);
        AJ_DeliverMsg(&msg);
    }
}

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_Connect(&bus, "org.alljoyn", CONNECT_TIMEOUT);
            if (status != AJ_OK) {
                printf("AllJoyn failed to connect sleeping for 15 seconds\n");
                AJ_Sleep(15 * 1000);
                continue;
            }
            printf("AllJoyn connected\n");
            /*
             * Kick things off by binding a session port
             */
            status = AJ_BusBindSessionPort(&bus, ServicePort, NULL);
            if (status != AJ_OK) {
                printf("Failed to send bind session port message\n");
                break;
            }
            connected = TRUE;
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork(&bus);
            }
            continue;
        }

        switch (msg.msgId) {
        case AJ_REPLY_ID(AJ_METHOD_BIND_SESSION_PORT):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            status = AJ_BusRequestName(&bus, ServiceName, AJ_NAME_REQ_DO_NOT_QUEUE);
            break;

        case AJ_REPLY_ID(AJ_METHOD_REQUEST_NAME):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            status = AJ_BusAdvertiseName(&bus, ServiceName, AJ_TRANSPORT_ANY, AJ_BUS_START_ADVERTISING);
            break;

        case AJ_REPLY_ID(AJ_METHOD_ADVERTISE_NAME):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            break;

        case AJ_METHOD_ACCEPT_SESSION:
            status = AJ_BusReplyAcceptSession(&msg, TRUE);
            break;

        case AJ_SIGNAL_SESSION_LOST:
            /*
             * Force a disconnect
             */
            status = AJ_ERR_READ;
            break;

        default:
            /*
             * Pass to the built-in handlers
             */
            status = AJ_BusHandleBusMessage(&msg);
            break;
        }
        /*
         * Messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            printf("AllJoyn disconnect\n");
            AJ_Disconnect(&bus);
            connected = FALSE;
            /*
             * Sleep a little while before trying to reconnect
             */
            AJ_Sleep(10 * 1000);
        }
    }
    printf("svclite EXIT %d\n", status);

    return status;
}
#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
