/**
 * @file  Marhal/Unmarshal Tester
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

static uint8_t wireBuffer[16 * 1024];
static size_t wireBytes = 0;

static uint8_t txBuffer[1024];
static uint8_t rxBuffer[1024];

#ifdef AJ_YIELD

#include <assert.h>
#include <pthread.h>

static uint8_t ioThreadRunningWrite = FALSE;
static uint8_t ioThreadRunningRead = FALSE;
static uint8_t ioThreadRunningSleep = FALSE;
static pthread_t threadIdWrite;
static pthread_t threadIdRead;
static pthread_t threadIdSleep;


#if defined _WIN32
#define AJ_SLEEP_MSEC(x) Sleep((x))
#elif defined __linux__
#define AJ_SLEEP_MSEC(x) usleep((x)*1000)
#endif

void* AJ_ReadReady_Mutter(void* threadArg)
{
    AJ_Printf("R ");
    AJ_SLEEP_MSEC(1);  // simulate a delay
    AJ_Schedule(AJWAITEVENT_READ);
    return 0;
}

void* AJ_WriteReady_Mutter(void* threadArg)
{
    AJ_Printf("W ");
    AJ_SLEEP_MSEC(1);  // simulate a delay
    AJ_Schedule(AJWAITEVENT_WRITE);
    return 0;
}

void* AJ_TimeExpired_Mutter(void* threadArg)
{
    AJ_Printf("T %d\n", (uint32_t)threadArg);
    AJ_SLEEP_MSEC((uint32_t)threadArg);
    AJ_Schedule(AJWAITEVENT_TIMER);
    return 0;
}


static AJ_Status AJ_sleepFunc(uint32_t timeout)
{
    /// spin up a thread here to set the timeEvent
    int ret = 0;
    if (!ioThreadRunningSleep) {
        ret = pthread_create(&threadIdSleep, NULL, AJ_TimeExpired_Mutter, (void*)timeout);
        if (ret != 0) {
            printf("Error: fail to spin a thread for sleeping\n");
        }
        ioThreadRunningSleep = TRUE;
    }

    AJ_YieldUntil(AJWAITEVENT_TIMER);

    /// wait for the thread to finish and then cleanup
    void* exit_status;
    if (ioThreadRunningSleep) {
        pthread_join(threadIdSleep, &exit_status);
        ioThreadRunningSleep = FALSE;
    }

    AJ_ClearEvents(AJWAITEVENT_TIMER); //reset the event once we have returned here.
    return AJ_OK;
}
#endif

static AJ_Status TxFunc(AJ_IOBuffer* buf)
{
    size_t tx = AJ_IO_BUF_AVAIL(buf);;

#ifdef AJ_YIELD
    ///  spin a thread here to set the writeEvent
    int ret = 0;
    if (!ioThreadRunningWrite) {
        ret = pthread_create(&threadIdWrite, NULL, AJ_WriteReady_Mutter, NULL);
        if (ret != 0) {
            printf("Error: fail to spin a thread for writing\n");
        }
        ioThreadRunningWrite = TRUE;
    }

    AJ_YieldUntil(AJWAITEVENT_WRITE);

    /// wait for the thread to finish and then cleanup
    void* exit_status;
    if (ioThreadRunningWrite) {
        pthread_join(threadIdWrite, &exit_status);
        ioThreadRunningWrite = FALSE;
    }

    AJ_ClearEvents(AJWAITEVENT_WRITE | AJWAITEVENT_TIMER); //reset the event once we have returned here.
#endif

    if ((wireBytes + tx) > sizeof(wireBuffer)) {
        return AJ_ERR_WRITE;
    } else {
        memcpy(wireBuffer + wireBytes, buf->bufStart, tx);
        AJ_IO_BUF_RESET(buf);
        wireBytes += tx;
        return AJ_OK;
    }
}

AJ_Status RxFunc(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    size_t rx = AJ_IO_BUF_SPACE(buf);

#ifdef AJ_YIELD
    ///  spin a thread here to set the readEvent
    int ret = 0;
    if (!ioThreadRunningRead) {
        ret = pthread_create(&threadIdRead, NULL, AJ_ReadReady_Mutter, (void*)timeout);
        if (ret != 0) {
            printf("Error: fail to spin a thread for reading\n");
        }
        ioThreadRunningRead = TRUE;
    }

    AJ_YieldUntil(AJWAITEVENT_READ | AJWAITEVENT_TIMER);

    /// wait for the thread to finish and then cleanup
    void* exit_status;
    if (ioThreadRunningRead) {
        pthread_join(threadIdRead, &exit_status);
        ioThreadRunningRead = FALSE;
    }

    AJ_ClearEvents(AJWAITEVENT_READ | AJWAITEVENT_TIMER); //reset the event once we have returned here.
#endif

    rx = min(len, rx);
    rx = min(wireBytes, rx);
    if (!rx) {
        return AJ_ERR_READ;
    } else {
        memcpy(buf->writePtr, wireBuffer, rx);
        /*
         * Shuffle the remaining data to the front of the buffer
         */
        memmove(wireBuffer, wireBuffer + rx, wireBytes - rx);
        wireBytes -= rx;
        buf->writePtr += rx;
        return AJ_OK;
    }
}

static const char* const testSignature[] = {
    "a{us}",
    "u(usu(ii)qsq)yyy",
    "a(usay)",
    "aas",
    "ivi",
    "v",
    "v",
    "(vvvv)",
    "uqay",
    "a(uuuu)",
    "a(sss)",
    "ya{ss}",
    "yyyyya{ys}"
};

typedef struct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} TestStruct;

static AJ_Status MsgInit(AJ_Message* msg, uint32_t msgId, uint8_t msgType)
{
    msg->objPath = "/test/mutter";
    msg->iface = "test.mutter";
    msg->member = "mumble";
    msg->msgId = msgId;
    msg->signature = testSignature[msgId];
    return AJ_OK;
}

#ifndef NDEBUG
extern AJ_MutterHook MutterHook;
#endif


static const char* const Fruits[] = {
    "apple", "banana", "cherry", "durian", "elderberry", "fig", "grape"
};

static const char* const Colors[] = {
    "azure", "blue", "cyan", "dun", "ecru"
};

static const uint8_t Data8[] = { 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0xA1, 0xB1, 0xC2, 0xD3 };
static const uint16_t Data16[] = { 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06 };

#define CHECK(x) if ((status = (x)) != AJ_OK) { break; }

int AJ_Main()
{
    AJ_Status status;
    AJ_BusAttachment bus;
    AJ_Message txMsg;
    AJ_Message rxMsg;
    AJ_Arg arg;
    AJ_Arg array1;
    AJ_Arg array2;
    AJ_Arg struct1;
    AJ_Arg struct2;
    size_t sz;
    uint32_t i;
    uint32_t j;
    uint32_t k;
    uint32_t key;
    uint32_t len;
    uint32_t u;
    uint32_t v;
    int32_t n;
    int32_t m;
    uint16_t q;
    uint16_t r;
    uint8_t y;
    char* str;
    char* sig;
    void* raw;

    bus.sock.tx.direction = AJ_IO_BUF_TX;
    bus.sock.tx.bufSize = sizeof(txBuffer);
    bus.sock.tx.bufStart = txBuffer;
    bus.sock.tx.readPtr = bus.sock.tx.bufStart;
    bus.sock.tx.writePtr = bus.sock.tx.bufStart;
    bus.sock.tx.send = TxFunc;

    bus.sock.rx.direction = AJ_IO_BUF_RX;
    bus.sock.rx.bufSize = sizeof(rxBuffer);
    bus.sock.rx.bufStart = rxBuffer;
    bus.sock.rx.readPtr = bus.sock.rx.bufStart;
    bus.sock.rx.writePtr = bus.sock.rx.bufStart;
    bus.sock.rx.recv = RxFunc;

    /*
     * Set the hook
     */
#ifndef NDEBUG
    MutterHook = MsgInit;
#else
    AJ_Printf("mutter only works in DEBUG builds\n");
    return -1;
#endif

    for (i = 0; i < ArraySize(testSignature); ++i) {

        status = AJ_MarshalSignal(&bus, &txMsg, i, "mutter.service", 0, 0, 0);
        if (status != AJ_OK) {
            break;
        }

        switch (i) {
        case 0:
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (key = 0; key < ArraySize(Fruits); ++key) {
                AJ_Arg dict;
                CHECK(AJ_MarshalContainer(&txMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_MarshalArgs(&txMsg, "us", key, Fruits[key]));
                CHECK(AJ_MarshalCloseContainer(&txMsg, &dict));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            break;

        case 1:
            CHECK(AJ_MarshalArgs(&txMsg, "u", 11111));
            CHECK(AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_MarshalArgs(&txMsg, "usu", 22222, "hello", 33333));
            CHECK(AJ_MarshalContainer(&txMsg, &struct2, AJ_ARG_STRUCT));
            CHECK(AJ_MarshalArgs(&txMsg, "ii", -100, -200));
            CHECK(AJ_MarshalCloseContainer(&txMsg, &struct2));
            CHECK(AJ_MarshalArgs(&txMsg, "qsq", 4444, "goodbye", 5555));
            CHECK(AJ_MarshalCloseContainer(&txMsg, &struct1));
            CHECK(AJ_MarshalArgs(&txMsg, "yyy", 1, 2, 3));
            break;

        case 2:
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (u = 0; u < ArraySize(Fruits); ++u) {
                CHECK(AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT));
                CHECK(AJ_MarshalArgs(&txMsg, "us", u, Fruits[u]));
                CHECK(AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_BYTE, AJ_ARRAY_FLAG, Data8, u)));
                CHECK(AJ_MarshalCloseContainer(&txMsg, &struct1));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            break;

        case 3:
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (j = 0; j < 3; ++j) {
                CHECK(AJ_MarshalContainer(&txMsg, &array2, AJ_ARG_ARRAY));
                for (k = j; k < ArraySize(Fruits); ++k) {
                    CHECK(AJ_MarshalArgs(&txMsg, "s", Fruits[k]));
                }
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array2));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            break;

        case 4:
            CHECK(AJ_MarshalArgs(&txMsg, "i", 987654321));
            CHECK(AJ_MarshalVariant(&txMsg, "a(ii)"));
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (j = 0; j < 16; ++j) {
                CHECK(AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT));
                CHECK(AJ_MarshalArgs(&txMsg, "ii", j + 1, (j + 1) * 100));
                CHECK(AJ_MarshalCloseContainer(&txMsg, &struct1));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            CHECK(AJ_MarshalArgs(&txMsg, "i", 123456789));
            break;

        case 5:
            CHECK(AJ_MarshalVariant(&txMsg, "(ivi)"));
            CHECK(AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_MarshalArgs(&txMsg, "i", 1212121));
            CHECK(AJ_MarshalVariant(&txMsg, "s"));
            CHECK(AJ_MarshalArgs(&txMsg, "s", "inner variant"));
            CHECK(AJ_MarshalArgs(&txMsg, "i", 3434343));
            CHECK(AJ_MarshalCloseContainer(&txMsg, &struct1));
            break;

        case 6:
            CHECK(AJ_MarshalVariant(&txMsg, "v"));
            CHECK(AJ_MarshalVariant(&txMsg, "v"));
            CHECK(AJ_MarshalVariant(&txMsg, "v"));
            CHECK(AJ_MarshalVariant(&txMsg, "v"));
            CHECK(AJ_MarshalVariant(&txMsg, "s"));
            CHECK(AJ_MarshalArgs(&txMsg, "s", "deep variant"));
            break;

        case 7:
            CHECK(AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_MarshalVariant(&txMsg, "i"));
            CHECK(AJ_MarshalArgs(&txMsg, "i", 1212121));
            CHECK(AJ_MarshalVariant(&txMsg, "s"));
            CHECK(AJ_MarshalArgs(&txMsg, "s", "variant"));
            CHECK(AJ_MarshalVariant(&txMsg, "ay"));
            CHECK(AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_BYTE, AJ_ARRAY_FLAG, Data8, sizeof(Data8))));
            CHECK(AJ_MarshalVariant(&txMsg, "aq"));
            CHECK(AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_UINT16, AJ_ARRAY_FLAG, Data16, sizeof(Data16))));
            CHECK(AJ_MarshalCloseContainer(&txMsg, &struct1));
            break;

        case 8:
            CHECK(AJ_MarshalArgs(&txMsg, "uq", 0xF00F00F0, 0x0707));
            len = 5000;
            CHECK(AJ_DeliverMsgPartial(&txMsg, len + 4));
            CHECK(AJ_MarshalRaw(&txMsg, &len, 4));
            for (j = 0; j < len; ++j) {
                uint8_t n = (uint8_t)j;
                CHECK(AJ_MarshalRaw(&txMsg, &n, 1));
            }
            break;

        case 9:
            len = 500;
            u = len * sizeof(TestStruct);
            CHECK(AJ_DeliverMsgPartial(&txMsg, u + sizeof(u) + 4));
            CHECK(AJ_MarshalRaw(&txMsg, &u, sizeof(u)));
            /*
             * Structs are always 8 byte aligned
             */
            u = 0;
            CHECK(AJ_MarshalRaw(&txMsg, &u, 4));
            for (j = 0; j < len; ++j) {
                TestStruct ts;
                ts.a = j;
                ts.b = j + 1;
                ts.c = j + 2;
                ts.d = j + 3;
                CHECK(AJ_MarshalRaw(&txMsg, &ts, sizeof(ts)));
            }
            break;

        case 10:
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            break;

        case 11:
            CHECK(AJ_MarshalArgs(&txMsg, "y", 127));
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (key = 0; key < ArraySize(Colors); ++key) {
                AJ_Arg dict;
                CHECK(AJ_MarshalContainer(&txMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_MarshalArgs(&txMsg, "ss", Colors[key], Fruits[key]));
                CHECK(AJ_MarshalCloseContainer(&txMsg, &dict));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            break;

        case 12:
            CHECK(AJ_MarshalArgs(&txMsg, "y", 0x11));
            CHECK(AJ_MarshalArgs(&txMsg, "y", 0x22));
            CHECK(AJ_MarshalArgs(&txMsg, "y", 0x33));
            CHECK(AJ_MarshalArgs(&txMsg, "y", 0x44));
            CHECK(AJ_MarshalArgs(&txMsg, "y", 0x55));
            CHECK(AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY));
            for (key = 0; key < ArraySize(Colors); ++key) {
                AJ_Arg dict;
                CHECK(AJ_MarshalContainer(&txMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_MarshalArgs(&txMsg, "ys", (uint8_t)key, Colors[key]));
                CHECK(AJ_MarshalCloseContainer(&txMsg, &dict));
            }
            if (status == AJ_OK) {
                CHECK(AJ_MarshalCloseContainer(&txMsg, &array1));
            }
            break;
        }
        if (status != AJ_OK) {
            AJ_Printf("Failed %d\n", i);
            break;
        }

        AJ_Printf("deliver\n");
        AJ_DeliverMsg(&txMsg);

        status = AJ_UnmarshalMsg(&bus, &rxMsg, 0);
        if (status != AJ_OK) {
            break;
        }

        switch (i) {
        case 0:
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (TRUE) {
                char* fruit;
                AJ_Arg dict;
                CHECK(AJ_UnmarshalContainer(&rxMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_UnmarshalArgs(&rxMsg, "us", &key, &fruit));
                AJ_Printf("Unmarshal[%d] = %s\n", key, fruit);
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &dict));
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;

        case 1:
            CHECK(AJ_UnmarshalArgs(&rxMsg, "u", &u));
            AJ_Printf("Unmarshal %u\n", u);
            CHECK(AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "usu", &u, &str, &v));
            AJ_Printf("Unmarshal %u %s %u\n", u, str, v);
            CHECK(AJ_UnmarshalContainer(&rxMsg, &struct2, AJ_ARG_STRUCT));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "ii", &n, &m));
            AJ_Printf("Unmarshal %d %d\n", n, m);
            CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct2));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "qsq", &q, &str, &r));
            AJ_Printf("Unmarshal %u %s %u\n", q, str, r);
            CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct1));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            AJ_Printf("Unmarshal %d\n", y);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            AJ_Printf("Unmarshal %d\n", y);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            AJ_Printf("Unmarshal %d\n", y);
            break;

        case 2:
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (status == AJ_OK) {
                CHECK(AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT));
                CHECK(AJ_UnmarshalArgs(&rxMsg, "us", &u, &str));
                CHECK(AJ_UnmarshalArg(&rxMsg, &arg));
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct1));
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;

        case 3:
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (status == AJ_OK) {
                CHECK(AJ_UnmarshalContainer(&rxMsg, &array2, AJ_ARG_ARRAY));
                while (status == AJ_OK) {
                    CHECK(AJ_UnmarshalArg(&rxMsg, &arg));
                    AJ_Printf("Unmarshal %s\n", arg.val.v_string);
                }
                /*
                 * We expect AJ_ERR_NO_MORE
                 */
                if (status == AJ_ERR_NO_MORE) {
                    CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array2));
                }
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;

        case 4:
            CHECK(AJ_UnmarshalArgs(&rxMsg, "i", &j));
            AJ_Printf("Unmarshal %d\n", j);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (status == AJ_OK) {
                CHECK(AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT));
                CHECK(AJ_UnmarshalArgs(&rxMsg, "ii", &j, &k));
                AJ_Printf("Unmarshal[%d] %d\n", j, k);
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct1));
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status != AJ_ERR_NO_MORE) {
                break;
            }
            CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "i", &j));
            AJ_Printf("Unmarshal %d\n", j);
            break;

        case 5:
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "i", &j));
            AJ_Printf("Unmarshal %d\n", j);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "s", &str));
            AJ_Printf("Unmarshal %s\n", str);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "i", &j));
            AJ_Printf("Unmarshal %d\n", j);
            CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct1));
            break;

        case 6:
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "s", &str));
            AJ_Printf("Unmarshal %s\n", str);
            break;

        case 7:
            CHECK(AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT));
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "i", &j));
            AJ_Printf("Unmarshal %d\n", j);
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArgs(&rxMsg, "s", &str));
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArg(&rxMsg, &arg));
            CHECK(AJ_UnmarshalVariant(&rxMsg, (const char**)&sig));
            AJ_Printf("Unmarshal variant %s\n", sig);
            CHECK(AJ_UnmarshalArg(&rxMsg, &arg));
            CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &struct1));
            break;

        case 8:
            CHECK(AJ_UnmarshalArgs(&rxMsg, "uq", &j, &q));
            AJ_Printf("Unmarshal %x\n", j);
            AJ_Printf("Unmarshal %x\n", q);
            CHECK(AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, sizeof(len), &sz));
            len = *((uint32_t*)raw);
            AJ_Printf("UnmarshalRaw %d\n", len);
            for (j = 0; j < len; ++j) {
                uint8_t v;
                CHECK(AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 1, &sz));
                v = *((uint8_t*)raw);
                if (v != (uint8_t)j) {
                    status = AJ_ERR_FAILURE;
                    break;
                }
            }
            break;

        case 9:
            CHECK(AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 4, &sz));
            len = *((uint32_t*)raw) / sizeof(TestStruct);
            /*
             * Structs are always 8 byte aligned
             */
            CHECK(AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 4, &sz));
            for (j = 0; j < len; ++j) {
                TestStruct* ts;
                CHECK(AJ_UnmarshalRaw(&rxMsg, (const void**)&ts, sizeof(TestStruct), &sz));
                if ((ts->a != j) || (ts->b != (j + 1)) || (ts->c != (j + 2)) || (ts->d != (j + 3))) {
                    status = AJ_ERR_FAILURE;
                    break;
                }
            }
            break;

        case 10:
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            status = AJ_UnmarshalArg(&rxMsg, &arg);
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;

        case 11:
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (TRUE) {
                AJ_Arg dict;
                char* fruit;
                char* color;
                CHECK(AJ_UnmarshalContainer(&rxMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_UnmarshalArgs(&rxMsg, "ss", &color, &fruit));
                AJ_Printf("Unmarshal[%s] = %s\n", color, fruit);
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &dict));
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;

        case 12:
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalArgs(&rxMsg, "y", &y));
            CHECK(AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY));
            while (TRUE) {
                AJ_Arg dict;
                char* color;
                CHECK(AJ_UnmarshalContainer(&rxMsg, &dict, AJ_ARG_DICT_ENTRY));
                CHECK(AJ_UnmarshalArgs(&rxMsg, "ys", &y, &color));
                AJ_Printf("Unmarshal[%d] = %s\n", y, color);
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &dict));
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                CHECK(AJ_UnmarshalCloseContainer(&rxMsg, &array1));
            }
            break;
        }
        if (status != AJ_OK) {
            AJ_Printf("Failed %d\n", i);
            break;
        }
        AJ_CloseMsg(&rxMsg);
        AJ_Printf("Passed %d\n", i);

    }
    if (status != AJ_OK) {
        AJ_Printf("Marshal/Unmarshal unit test[%d] failed %d\n", i, status);
    }

#ifdef AJ_YIELD
    // test the sleep function.
    AJ_sleepFunc(1);
    AJ_Printf("slept 1\n");
    AJ_sleepFunc(5);
    AJ_Printf("slept 5\n");

    AJ_sleepFunc(1000);
    AJ_Printf("slept 1000\n");

    // instead of returning, trigger an exitEvent and yield out
    AJ_Schedule(AJWAITEVENT_EXIT);
    AJ_YieldUntil(AJWAITEVENT_EXIT);

    while (1) {
        assert("Ran past the yield point. Crash!");
    }
#endif
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

