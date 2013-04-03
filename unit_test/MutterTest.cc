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
    "a(uuuu)"
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

TEST_F(MutterTest, ArrayOfStructs)
{
    AJ_Status status = AJ_ERR_FAILURE;

    // Index of "a(uuuu)" in testSignature[] is 9
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
            void* raw;
            size_t sz;
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
