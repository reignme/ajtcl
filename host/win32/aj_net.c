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

#include <Winsock2.h>
#include <Mswsock.h>
#include <ws2tcpip.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "aj_host.h"
#include "aj_bufio.h"
#include "aj_net.h"
#include "aj_util.h"

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

static AJ_Status Send(AJ_IOBuffer* buf)
{
    DWORD ret;
    DWORD tx = AJ_IO_BUF_AVAIL(buf);

    assert(buf->direction == AJ_IO_BUF_TX);

    if (tx > 0) {
        ret = send((SOCKET)buf->context, buf->readPtr, tx, 0);
        if (ret == SOCKET_ERROR) {
#ifndef NDEBUG
            fprintf(stderr, "send() failed: %d\n", WSAGetLastError());
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

static AJ_Status Recv(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    AJ_Status status;
    DWORD ret;
    DWORD rx = AJ_IO_BUF_SPACE(buf);
    fd_set fds;
    int rc = 0;
    const struct timeval tv = { timeout / 1000, 1000 * (timeout % 1000) };

    assert(buf->direction == AJ_IO_BUF_RX);

    FD_ZERO(&fds);
    FD_SET((SOCKET)buf->context, &fds);
    rc = select(1, &fds, NULL, NULL, &tv);
    if (rc == 0) {
        return AJ_ERR_TIMEOUT;
    }

    rx = min(rx, len);
    ret = recv((SOCKET)buf->context, buf->writePtr, rx, 0);
    if (ret == SOCKET_ERROR) {
        status = AJ_ERR_READ;
    } else {
        buf->writePtr += ret;
        status = AJ_OK;
    }
    return status;
}

/*
 * Statically sized buffers for I/O
 */
static uint8_t rxData[1024];
static uint8_t txData[1024];

AJ_Status AJ_Net_Connect(AJ_NetSocket* netSock, uint16_t port, uint8_t addrType, const uint32_t* addr)
{
    DWORD ret;
    SOCKADDR_STORAGE addrBuf;
    socklen_t addrSize;
    SOCKET sock;

    memset(&addrBuf, 0, sizeof(addrBuf));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return AJ_ERR_CONNECT;
    }
    if (addrType == AJ_ADDR_IPV4) {
        struct sockaddr_in* sa = (struct sockaddr_in*)&addrBuf;
        sa->sin_family = AF_INET;
        sa->sin_port = htons(port);
        sa->sin_addr.s_addr = *addr;
        addrSize = sizeof(*sa);
    } else {
        struct sockaddr_in6* sa = (struct sockaddr_in6*)&addrBuf;
        sa->sin6_family = AF_INET6;
        sa->sin6_port = htons(port);
        memcpy(sa->sin6_addr.s6_addr, addr, sizeof(sa->sin6_addr.s6_addr));
        addrSize = sizeof(*sa);
    }
    ret = connect(sock, (struct sockaddr*)&addrBuf, addrSize);
    if (ret == SOCKET_ERROR) {
#ifndef NDEBUG
        fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
#endif
        closesocket(sock);
        return AJ_ERR_CONNECT;
    } else {
        AJ_IOBufInit(&netSock->rx, rxData, sizeof(rxData), AJ_IO_BUF_RX, (void*)sock);
        netSock->rx.recv = Recv;
        AJ_IOBufInit(&netSock->tx, txData, sizeof(txData), AJ_IO_BUF_TX, (void*)sock);
        netSock->tx.send = Send;
        return AJ_OK;
    }
}

void AJ_Net_Disconnect(AJ_NetSocket* netSock)
{
    SOCKET sock = (SOCKET)netSock->rx.context;
    if (sock) {
        shutdown(sock, 0);
        closesocket(sock);
        memset(netSock, 0, sizeof(AJ_NetSocket));
    }
}

static AJ_Status SendTo(AJ_IOBuffer* buf)
{
    DWORD ret;
    DWORD tx = AJ_IO_BUF_AVAIL(buf);

    assert(buf->direction == AJ_IO_BUF_TX);

    if (tx > 0) {
        /*
         * Only multicasting over IPv4 for now
         */
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(AJ_UDP_PORT);
        sin.sin_addr.s_addr = inet_addr(AJ_IPV4_MULTICAST_GROUP);
        ret = sendto((SOCKET)buf->context, buf->readPtr, tx, 0, (struct sockaddr*)&sin, sizeof(sin));
        if (ret == SOCKET_ERROR) {
#ifndef NDEBUG
            fprintf(stderr, "sendto() failed: %d\n", WSAGetLastError());
#endif
            return AJ_ERR_WRITE;
        }
        buf->readPtr += ret;
    }
    AJ_IO_BUF_RESET(buf);
    return AJ_OK;
}

static AJ_Status RecvFrom(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    AJ_Status status;
    DWORD ret;
    DWORD rx = AJ_IO_BUF_SPACE(buf);
    fd_set fds;
    int rc = 0;
    const struct timeval tv = { timeout / 1000, 1000 * (timeout % 1000) };

    assert(buf->direction == AJ_IO_BUF_RX);

    FD_ZERO(&fds);
    FD_SET((SOCKET)buf->context, &fds);
    rc = select(1, &fds, NULL, NULL, &tv);
    if (rc == 0) {
        return AJ_ERR_TIMEOUT;
    }

    rx = min(rx, len);
    ret = recvfrom((SOCKET)buf->context, buf->writePtr, rx, 0, NULL, 0);
    if (ret == SOCKET_ERROR) {
        status = AJ_ERR_READ;
    } else {
        buf->writePtr += ret;
        status = AJ_OK;
    }
    return status;
}

static uint8_t rxDataMCast[256];
static uint8_t txDataMCast[256];

AJ_Status AJ_Net_MCastUp(AJ_NetSocket* netSock)
{
    DWORD ret;
    struct ip_mreq mreq;
    SOCKADDR_IN sin;
    WSADATA wsaData;
    WORD version = MAKEWORD(2, 0);
    SOCKET sock;
    int reuse = 1;

    WSAStartup(version, &wsaData);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        return AJ_ERR_READ;
    }

    /*
     * Bind our multicast port
     */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(AJ_UDP_PORT);
    sin.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sock, (struct sockaddr*)&sin, sizeof(sin));
    if (ret == SOCKET_ERROR) {
        closesocket(sock);
        return AJ_ERR_READ;
    }

    /*
     * Join our multicast group
     */
    mreq.imr_multiaddr.s_addr = inet_addr(AJ_IPV4_MULTICAST_GROUP);
    mreq.imr_interface.s_addr = INADDR_ANY;
    ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    if (ret == SOCKET_ERROR) {
        memset(netSock, 0, sizeof(AJ_NetSocket));
        closesocket(sock);
        return AJ_ERR_READ;
    } else {
        AJ_IOBufInit(&netSock->rx, rxDataMCast, sizeof(rxDataMCast), AJ_IO_BUF_RX, (void*)sock);
        netSock->rx.recv = RecvFrom;
        AJ_IOBufInit(&netSock->tx, txDataMCast, sizeof(txDataMCast), AJ_IO_BUF_TX, (void*)sock);
        netSock->tx.send = SendTo;
        return AJ_OK;
    }
}

void AJ_Net_MCastDown(AJ_NetSocket* netSock)
{
    struct ip_mreq mreq;
    SOCKET sock = (SOCKET)netSock->rx.context;
    if (sock) {
        /*
         * Leave our multicast group
         */
        mreq.imr_multiaddr.s_addr = inet_addr(AJ_IPV4_MULTICAST_GROUP);
        mreq.imr_interface.s_addr = INADDR_ANY;
        setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
        shutdown(sock, 0);
        closesocket(sock);
        memset(netSock, 0, sizeof(AJ_NetSocket));
    }
    WSACleanup();
}

AJ_Status AJ_Net_Up(void)
{
    WSADATA wsaData;
    WORD version = MAKEWORD(2, 0);
    WSAStartup(version, &wsaData);
    return AJ_OK;
}

void AJ_Net_Down(void)
{
    WSACleanup();
}
