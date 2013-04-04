/**
 * @file  Marhal/Unmarshal Unit Test
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

#include <gtest/gtest.h>

extern "C" {
#include "alljoyn.h"
#include "aj_util.h"
#include "aj_debug.h"
#include "aj_bufio.h"

#ifndef NDEBUG
extern AJ_MutterHook MutterHook;
#endif
}

static uint8_t wireBuffer[16 * 1024];
static size_t wireBytes = 0;

static uint8_t txBuffer[1024];
static uint8_t rxBuffer[1024];

static AJ_Status TxFunc(AJ_IOBuffer* buf)
{
    size_t tx = AJ_IO_BUF_AVAIL(buf);;

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

// Array of test signatures
// Each test case will use a particular index into this array
// to get the message signature.
static const char* testSignature[] = {
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
    "a(sss)"
};

static AJ_Status MsgInit(AJ_Message* msg, uint32_t msgId, uint8_t msgType)
{
    msg->objPath = "/test/mutter";
    msg->iface = "test.mutter";
    msg->member = "mumble";
    msg->msgId = msgId;
    msg->signature = testSignature[msgId];
    return AJ_OK;
}

static const char* Fruits[] = {
    "apple", "banana", "cherry", "durian", "elderberry", "fig", "grape"
};

static const uint8_t Data8[] = { 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0xA1, 0xB1, 0xC2, 0xD3 };
static const uint16_t Data16[] = { 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06 };

// Variables common to all tests
static AJ_BusAttachment testBus;
static AJ_Message txMsg;
static AJ_Message rxMsg;
static AJ_Arg arg;
static AJ_Arg array1;
static AJ_Arg array2;
static AJ_Arg struct1;
static AJ_Arg struct2;

class MutterTest : public testing::Test {
  public:
    virtual void SetUp() {
        testBus.sock.tx.direction = AJ_IO_BUF_TX;
        testBus.sock.tx.bufSize = sizeof(txBuffer);
        testBus.sock.tx.bufStart = txBuffer;
        testBus.sock.tx.readPtr = txBuffer;
        testBus.sock.tx.writePtr = txBuffer;
        testBus.sock.tx.send = TxFunc;

        testBus.sock.rx.direction = AJ_IO_BUF_RX;
        testBus.sock.rx.bufSize = sizeof(rxBuffer);
        testBus.sock.rx.bufStart = rxBuffer;
        testBus.sock.rx.readPtr = rxBuffer;
        testBus.sock.rx.writePtr = rxBuffer;
        testBus.sock.rx.recv = RxFunc;

        MutterHook = MsgInit;
    }

    virtual void TearDown() {

    }
};

typedef struct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} MutterTestStruct;

TEST_F(MutterTest, ArrayofDict) {

    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "a{us}" in testSignature[] is 0
    status = AJ_MarshalSignal(&testBus, &txMsg, 0, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
    if (AJ_OK == status) {
        status = AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        for (uint32_t key = 0; key < ArraySize(Fruits); ++key) {
            AJ_Arg dict;
            status = AJ_MarshalContainer(&txMsg, &dict, AJ_ARG_DICT_ENTRY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalArgs(&txMsg, "us", key, Fruits[key]);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalCloseContainer(&txMsg, &dict);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        if (AJ_OK == status) {
            status = AJ_MarshalCloseContainer(&txMsg, &array1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            uint32_t key;
            status = AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            for (uint32_t i = 0; i < ArraySize(Fruits); ++i) {
                char* fruit;
                AJ_Arg dict;
                status = AJ_UnmarshalContainer(&rxMsg, &dict, AJ_ARG_DICT_ENTRY);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalArgs(&rxMsg, "us", &key, &fruit);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                printf("Unmarshal[%d] = %s\n", key, fruit);
                status = AJ_UnmarshalCloseContainer(&rxMsg, &dict);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }
            AJ_Arg dict;
            status = AJ_UnmarshalContainer(&rxMsg, &dict, AJ_ARG_DICT_ENTRY);
            EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);
            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (status == AJ_ERR_NO_MORE) {
                status = AJ_UnmarshalCloseContainer(&rxMsg, &array1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, BasicTypesAndNestedStruct) {
    uint32_t u;
    uint32_t v;
    int32_t n;
    int32_t m;
    uint16_t q;
    uint16_t r;
    uint8_t y;
    char* str;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "u(usu(ii)qsq)yyy" in testSignature[] is 1
    status = AJ_MarshalSignal(&testBus, &txMsg, 1, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
    if (AJ_OK == status) {
        status = AJ_MarshalArgs(&txMsg, "u", 11111);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "usu", 22222, "hello", 33333);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalContainer(&txMsg, &struct2, AJ_ARG_STRUCT);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "ii", -100, -200);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalCloseContainer(&txMsg, &struct2);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "qsq", 4444, "goodbye", 5555);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalCloseContainer(&txMsg, &struct1);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "yyy", 1, 2, 3);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);


        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        if (AJ_OK == status) {

            status = AJ_UnmarshalArgs(&rxMsg, "u", &u);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "usu", &u, &str, &v);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalContainer(&rxMsg, &struct2, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "ii", &n, &m);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalCloseContainer(&rxMsg, &struct2);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "qsq", &q, &str, &r);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalCloseContainer(&rxMsg, &struct1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "y", &y);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "y", &y);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "y", &y);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, ArrayOfStructofBasicTypeStringandByteArray)
{
    char* str;
    AJ_Status status = AJ_ERR_FAILURE;

    //Index of "a(usay)" in testSignature[] is 2
    status = AJ_MarshalSignal(&testBus, &txMsg, 2, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
    if (AJ_OK == status) {

        status = AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        for (uint32_t u = 0; u < ArraySize(Fruits); ++u) {

            status = AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalArgs(&txMsg, "us", u, Fruits[u]);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_BYTE, AJ_ARRAY_FLAG, Data8, u));
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalCloseContainer(&txMsg, &struct1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        if (AJ_OK == status) {
            status = AJ_MarshalCloseContainer(&txMsg, &array1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        if (AJ_OK == status) {
            uint32_t u;
            status = AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY);

            for (uint32_t i = 0; i < ArraySize(Fruits); ++i) {

                status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalArgs(&rxMsg, "us", &u, &str);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalArg(&rxMsg, &arg);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalCloseContainer(&rxMsg, &struct1);

            }
            /*
             * We expect AJ_ERR_NO_MORE
             */
            status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);

            if (AJ_ERR_NO_MORE == status) {
                status = AJ_UnmarshalCloseContainer(&rxMsg, &array1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, ArrayOfArrayofString)
{
    uint32_t count = 3;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "aas" in testSignature[] is 3
    status = AJ_MarshalSignal(&testBus, &txMsg, 3, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {

        status = AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        for (uint32_t j = 0; j < count; ++j) {

            status = AJ_MarshalContainer(&txMsg, &array2, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

            for (uint32_t k = j; k < ArraySize(Fruits); ++k) {

                status = AJ_MarshalArgs(&txMsg, "s", Fruits[k]);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }

            status = AJ_MarshalCloseContainer(&txMsg, &array2);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
        if (AJ_OK == status) {

            status = AJ_MarshalCloseContainer(&txMsg, &array1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        if (AJ_OK == status) {

            status = AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

            for (uint32_t j = 0; j < count; ++j) {

                status = AJ_UnmarshalContainer(&rxMsg, &array2, AJ_ARG_ARRAY);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

                for (uint32_t k = j; k < ArraySize(Fruits); ++k) {

                    status = AJ_UnmarshalArg(&rxMsg, &arg);
                    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                    AJ_Printf("Unmarshal %s\n", arg.val.v_string);
                }

                /*
                 * We expect AJ_ERR_NO_MORE
                 */

                status = AJ_UnmarshalArg(&rxMsg, &arg);
                EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);

                if (AJ_ERR_NO_MORE == status) {
                    status = AJ_UnmarshalCloseContainer(&rxMsg, &array2);
                    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                }
            }
            /*
             * We expect AJ_ERR_NO_MORE
             */

            status = AJ_UnmarshalContainer(&rxMsg, &array2, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);

            if (AJ_ERR_NO_MORE == status) {
                status = AJ_UnmarshalCloseContainer(&rxMsg, &array1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, IntegerandVariant)
{
    char* sig;
    uint32_t count = 16;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "ivi" in testSignature[] is 4
    status = AJ_MarshalSignal(&testBus, &txMsg, 4, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {

        status = AJ_MarshalArgs(&txMsg, "i", 987654321);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "a(ii)");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        for (uint32_t i = 0; i < count; ++i) {
            status = AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalArgs(&txMsg, "ii", i + 1, (i + 1) * 100);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_MarshalCloseContainer(&txMsg, &struct1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
        if (AJ_OK == status) {

            status = AJ_MarshalCloseContainer(&txMsg, &array1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_MarshalArgs(&txMsg, "i", 123456789);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            uint32_t k;
            uint32_t l;
            status = AJ_UnmarshalArgs(&rxMsg, "i", &k);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %d\n", k);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            for (uint32_t i = 0; i < count; ++i) {

                status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalArgs(&rxMsg, "ii", &k, &l);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                AJ_Printf("Unmarshal[%d] %d\n", k, l);
                status = AJ_UnmarshalCloseContainer(&rxMsg, &struct1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }

            /*
             * We expect AJ_ERR_NO_MORE
             */
            status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);
            if (AJ_ERR_NO_MORE == status) {
                status = AJ_UnmarshalCloseContainer(&rxMsg, &array1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                status = AJ_UnmarshalArgs(&rxMsg, "i", &k);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                AJ_Printf("Unmarshal %d\n", k);
            }
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}


TEST_F(MutterTest, StructofInteger_VariantandInteger)
{
    char* str;
    char* sig;
    uint32_t j;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "v" in testSignature[] is 5
    status = AJ_MarshalSignal(&testBus, &txMsg, 5, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
    if (AJ_OK == status) {
        status = AJ_MarshalVariant(&txMsg, "(ivi)");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "i", 1212121);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "s");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "s", "inner variant");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "i", 3434343);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalCloseContainer(&txMsg, &struct1);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);
            status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArgs(&rxMsg, "i", &j);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %d\n", j);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArgs(&rxMsg, "s", &str);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %s\n", str);

            status = AJ_UnmarshalArgs(&rxMsg, "i", &j);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %d\n", j);

            status = AJ_UnmarshalCloseContainer(&rxMsg, &struct1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}


TEST_F(MutterTest, DeepVariant)
{
    char* str;
    char* sig;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "v" in testSignature[] is 6
    status = AJ_MarshalSignal(&testBus, &txMsg, 6, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
    if (AJ_OK == status) {

        status = AJ_MarshalVariant(&txMsg, "v");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "v");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "v");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "v");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "s");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "s", "deep variant");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArgs(&rxMsg, "s", &str);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %s\n", str);

            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}


TEST_F(MutterTest, StructofVariants)
{
    char* str;
    char* sig;
    uint32_t j;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "(vvvv)" in testSignature[] is 7
    status = AJ_MarshalSignal(&testBus, &txMsg, 7, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {
        status = AJ_MarshalContainer(&txMsg, &struct1, AJ_ARG_STRUCT);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "i");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "i", 1212121);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "s");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArgs(&txMsg, "s", "variant");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "ay");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_BYTE, AJ_ARRAY_FLAG, Data8, sizeof(Data8)));
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalVariant(&txMsg, "aq");
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalArg(&txMsg, AJ_InitArg(&arg, AJ_ARG_UINT16, AJ_ARRAY_FLAG, Data16, sizeof(Data16)));
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalCloseContainer(&txMsg, &struct1);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            status = AJ_UnmarshalContainer(&rxMsg, &struct1, AJ_ARG_STRUCT);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArgs(&rxMsg, "i", &j);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal %d\n", j);

            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArgs(&rxMsg, "s", &str);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArg(&rxMsg, &arg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalVariant(&rxMsg, (const char**)&sig);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            AJ_Printf("Unmarshal variant %s\n", sig);

            status = AJ_UnmarshalArg(&rxMsg, &arg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalCloseContainer(&rxMsg, &struct1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, IntegerandArrayofInteger)
{
    uint32_t len;
    uint32_t j;
    uint16_t q;
    void* raw;
    size_t sz;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "uqay" in testSignature[] is 8
    status = AJ_MarshalSignal(&testBus, &txMsg, 8, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {

        status = AJ_MarshalArgs(&txMsg, "uq", 0xF00F00F00, 0x070707);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        len = 5000;
        status = AJ_DeliverMsgPartial(&txMsg, len + 4);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalRaw(&txMsg, &len, 4);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        for (j = 0; j < len; ++j) {
            uint8_t n = (uint8_t)j;
            status = AJ_MarshalRaw(&txMsg, &n, 1);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            status = AJ_UnmarshalArgs(&rxMsg, "uq", &j, &q);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, sizeof(len), &sz);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            len = *((uint32_t*)raw);
            for (j = 0; j < len; ++j) {
                uint8_t v;
                status = AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 1, &sz);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
                v = *((uint8_t*)raw);
                EXPECT_EQ(v, (uint8_t)j);
            }
            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}

TEST_F(MutterTest, ArrayOfStructs)
{
    void* raw;
    size_t sz;
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "a(uuuu)" in testSignature[] is 9
    status = AJ_MarshalSignal(&testBus, &txMsg, 9, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {
        size_t len = 500;
        uint32_t u = len * sizeof(MutterTestStruct);
        status = AJ_DeliverMsgPartial(&txMsg, u + sizeof(u) + 4);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalRaw(&txMsg, &u, sizeof(u));
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        /*
         * Structs are always 8 byte aligned
         */
        u = 0;
        status = AJ_MarshalRaw(&txMsg, &u, 4);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        for (size_t j = 0; j < len; ++j) {
            MutterTestStruct ts;
            ts.a = j;
            ts.b = j + 1;
            ts.c = j + 2;
            ts.d = j + 3;
            status = AJ_MarshalRaw(&txMsg, &ts, sizeof(ts));
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }

        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {
            status = AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 4, &sz);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            len = *((uint32_t*)raw) / sizeof(MutterTestStruct);
            /*
             * Structs are always 8 byte aligned
             */
            status = AJ_UnmarshalRaw(&rxMsg, (const void**)&raw, 4, &sz);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

            for (size_t j = 0; j < len; ++j) {
                MutterTestStruct* ts;
                status = AJ_UnmarshalRaw(&rxMsg, (const void**)&ts, sizeof(MutterTestStruct), &sz);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

                // Check the contents of the struct
                EXPECT_EQ(j, ts->a);
                EXPECT_EQ((j + 1), ts->b);
                EXPECT_EQ((j + 2), ts->c);
                EXPECT_EQ((j + 3), ts->d);
            }

            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}


TEST_F(MutterTest, ArrayOfStructofStrings)
{
    AJ_Status status = AJ_ERR_FAILURE;
    //Index of "a(sss)" in testSignature[] is 10
    status = AJ_MarshalSignal(&testBus, &txMsg, 10, "mutter.service", 0, 0, 0);
    EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

    if (AJ_OK == status) {

        status = AJ_MarshalContainer(&txMsg, &array1, AJ_ARG_ARRAY);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        status = AJ_MarshalCloseContainer(&txMsg, &array1);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);


        status = AJ_DeliverMsg(&txMsg);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        status = AJ_UnmarshalMsg(&testBus, &rxMsg, 0);
        EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);

        if (AJ_OK == status) {

            status = AJ_UnmarshalContainer(&rxMsg, &array1, AJ_ARG_ARRAY);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            status = AJ_UnmarshalArg(&rxMsg, &arg);
            EXPECT_EQ(AJ_ERR_NO_MORE, status) << "  Actual Status: " << AJ_StatusText(status);

            /*
             * We expect AJ_ERR_NO_MORE
             */
            if (AJ_ERR_NO_MORE == status) {

                status = AJ_UnmarshalCloseContainer(&rxMsg, &array1);
                EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
            }

            status = AJ_CloseMsg(&rxMsg);
            EXPECT_EQ(AJ_OK, status) << "  Actual Status: " << AJ_StatusText(status);
        }
    }
}
