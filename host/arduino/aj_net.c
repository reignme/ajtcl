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

#include "aj_host.h"
#include "aj_bufio.h"
#include "aj_net.h"
#include "aj_util.h"
#include "aj_debug.h"

#ifdef WIFI_UDP_WORKING
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#endif


static uint8_t rxDataStash[256];
static uint16_t rxLeftover = 0;

/*
 * IANA assigned IPv4 multicast group for AllJoyn.
 */
static const char AJ_IPV4_MULTICAST_GROUP[] = "224.0.0.113";

/*
 * IANA assigned IPv6 multicast group for AllJoyn.
 */
static const char AJ_IPV6_MULTICAST_GROUP[] = "ff02::13a";

/*
 * IANA assigned UDP multicast port for AllJoyn
 */
#define AJ_UDP_PORT 9956

#ifdef WIFI_UDP_WORKING
static WiFiClient g_client;
static WiFiUDP g_clientUDP;
#else
static EthernetClient g_client;
static EthernetUDP g_clientUDP;
#endif

AJ_Status Send(AJ_IOBuffer* buf)
{
    uint32_t ret;
    uint32_t tx = AJ_IO_BUF_AVAIL(buf);

    if (tx > 0) {
        ret = g_client.write(buf->readPtr, tx);
        if (ret == 0) {
            #ifndef NDEBUG
            printf("error sending %d\n", g_client.getWriteError());
            #endif
            return AJ_ERR_WRITE;
        }
        buf->readPtr += ret;
    }
    if (AJ_IO_BUF_AVAIL(buf) == 0) {
        AJ_IO_BUF_RESET(buf);
    }
    return AJ_OK;
}

AJ_Status Recv(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    AJ_Status status = AJ_ERR_READ;
    uint32_t ret;
    uint32_t rx = AJ_IO_BUF_SPACE(buf);
    uint32_t recvd = 0;
    unsigned long Recv_lastCall = millis();

#ifndef NDEBUG
    printf("len: %d rx: %d timeout %d\n", len, rx, timeout);
#endif
    rx = min(rx, len);

    uint32_t M = 0;
    if (rxLeftover != 0) {
        // there was something leftover from before,
        printf("leftover was: %d\n", rxLeftover);
        M = min(rx, rxLeftover);
        memcpy(buf->writePtr, rxDataStash, M);  // copy leftover into buffer.
        buf->writePtr += M;  // move the data pointer over
        memmove(rxDataStash, rxDataStash + M, rxLeftover - M); // shift left-overs toward the start.
        rxLeftover -= M;
        recvd += M;
        if (recvd == rx) {
            status = AJ_OK;
            return status;
        }
    }
    if ((M != 0) && (rxLeftover != 0)) {
        printf("M was: %d, rxLeftover was: %d\n", M, rxLeftover);
    }

    if (g_client.connected()) {
        // Wait until there is enough data or the timeout has happened.
        while ((g_client.available() == 0) && (millis() - Recv_lastCall < timeout)) {
            delay(50); // wait for data or timeout
        }
        printf("millis %d, Last_call %d timeout %d Avail: %d\n", millis(), Recv_lastCall, timeout, g_client.available());

        if ((millis() - Recv_lastCall >= timeout) && (g_client.available() == 0)) {
            return AJ_ERR_TIMEOUT;
        }

        uint32_t askFor = rx;
        askFor -= M;
        printf("ask for: %d\n", askFor);
        ret = g_client.read(buf->writePtr, askFor);
        printf("Recv: ret %d  askfor %d\n", ret, askFor);
        if (ret == -1) {
#ifndef NDEBUG
            printf("Recv failed\n");
#endif
            status = AJ_ERR_READ;
        } else {
            printf("ret now %d\n", ret);
            AJ_DumpBytes("Recv", buf->writePtr, ret);

            if (ret > askFor) {
                printf("new leftover %d\n", ret - askFor);
                // now shove the extra into the stash
                memcpy(rxDataStash + rxLeftover, buf->writePtr + askFor, ret - askFor);
                rxLeftover += (ret - askFor);
                buf->writePtr += rx;
            } else {
                buf->writePtr += ret;
            }
            status = AJ_OK;
        }
    }

    return status;
}


static uint8_t rxData[768];
static uint8_t txData[768];

AJ_Status AJ_Net_Connect(AJ_NetSocket* netSock, uint16_t port, uint8_t addrType, const uint32_t* addr)
{
    int ret;

    IPAddress ip(*addr);
    ret = g_client.connect(ip, port);

    Serial.print("Connecting to: ");
    Serial.println(ip);

    if (ret == -1) {
        printf("connect() failed: %d\n", ret);
        return AJ_ERR_CONNECT;
    } else {
        AJ_IOBufInit(&netSock->rx, rxData, sizeof(rxData), AJ_IO_BUF_RX, (void*)&g_client);
        netSock->rx.recv = Recv;
        AJ_IOBufInit(&netSock->tx, txData, sizeof(txData), AJ_IO_BUF_TX, (void*)&g_client);
        netSock->tx.send = Send;
        return AJ_OK;
    }
    return AJ_ERR_CONNECT;
}

void AJ_Net_Disconnect(AJ_NetSocket* netSock)
{
    g_client.stop();
}

AJ_Status SendTo(AJ_IOBuffer* buf)
{
    int ret;
    uint32_t tx = AJ_IO_BUF_AVAIL(buf);

    if (tx > 0) {
#ifdef WIFI_UDP_WORKING
        WiFiUDP* pUDP = (WiFiUDP*)buf->context;
#else
        EthernetUDP* pUDP = (EthernetUDP*)buf->context;
#endif
        ret = pUDP->beginPacket(AJ_IPV4_MULTICAST_GROUP, AJ_UDP_PORT);
        printf("SendTo beginPacket %d\n", ret);
        if (ret == 0) {
            printf("no sender\n");
        }

        ret = pUDP->write(buf->readPtr, tx);
        printf("SendTo write %d\n", ret);
        if (ret == 0) {
            printf("no bytes\n");
            return AJ_ERR_WRITE;
        }

        buf->readPtr += ret;

        ret = pUDP->endPacket();
        if (ret == 0) {
            printf("err endpacket\n");
            return AJ_ERR_WRITE;
        }

    }
    AJ_IO_BUF_RESET(buf);
    return AJ_OK;
}

AJ_Status RecvFrom(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    printf("RecvFrom ");
    AJ_Status status = AJ_OK;
    int ret;
    uint32_t rx = AJ_IO_BUF_SPACE(buf);
    unsigned long Recv_lastCall = millis();

#ifndef NDEBUG
    printf("len: %d rx: %d timeout %d\n", len, rx, timeout);
#endif

    rx = min(rx, len);
#ifdef WIFI_UDP_WORKING
    WiFiUDP* pUDP = (WiFiUDP*)buf->context;
#else
    EthernetUDP* pUDP = (EthernetUDP*)buf->context;
#endif

    while ((pUDP->parsePacket() == 0) && (millis() - Recv_lastCall < timeout)) {
        delay(10); // wait for data or timeout
    }

    printf("millis %d, Last_call %d timeout %d Avail: %d\n", millis(), Recv_lastCall, timeout, pUDP->available());
    ret = pUDP->read(buf->writePtr, rx);
    printf("RecvFrom: ret %d  rx %d\n", ret, rx);

    if (ret == -1) {
        status = AJ_ERR_READ;
    } else {
        if (ret != -1) {
            AJ_DumpBytes("RecvFrom", buf->writePtr, ret);
        }
        buf->writePtr += ret;
        status = AJ_OK;
    }
    return status;
}



AJ_Status AJ_Net_MCastUp(AJ_NetSocket* netSock)
{
    uint8_t ret = 0;
    ret = g_clientUDP.begin(AJ_UDP_PORT);

    if (ret != 1) {
        g_clientUDP.stop();
        return AJ_ERR_READ;
    } else {
        AJ_IOBufInit(&netSock->rx, rxData, sizeof(rxData), AJ_IO_BUF_RX, (void*)&g_clientUDP);
        netSock->rx.recv = RecvFrom;
        AJ_IOBufInit(&netSock->tx, txData, sizeof(txData), AJ_IO_BUF_TX, (void*)&g_clientUDP);
        netSock->tx.send = SendTo;
    }

    return AJ_OK;
}

void AJ_Net_MCastDown(AJ_NetSocket* netSock)
{
    g_clientUDP.flush();
    g_clientUDP.stop();
}


AJ_Status AJ_Net_Up()
{
    return AJ_OK;
}

void AJ_Net_Down()
{
}
