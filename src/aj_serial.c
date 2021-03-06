/**
 * @file
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
#ifdef AJ_SERIAL_CONNECTION
#include "aj_target.h"
#include "aj_status.h"
#include "aj_serial.h"
#include "aj_serial_rx.h"
#include "aj_serial_tx.h"
#include "aj_timer.h"

#define SLAP_VERSION 0

/**
 * global variable for link parameters
 */
AJ_LinkParameters AJ_SerialLinkParams;

/**
 * controls rate at which we send synch packets when the link is down in milliseconds
 */
#define CONN_TIMEOUT   1000

/**
 * controls how long we will wait for a confirmation packet after we synchronize in milliseconds
 */
#define NEGO_TIMEOUT   1000


#define LINK_PACKET_SIZE    4
#define NEGO_PACKET_SIZE    3

/** link packet types */
typedef enum {
    UNKNOWN_PKT = 0,
    CONN_PKT,
    ACPT_PKT,
    NEGO_PKT,
    NRSP_PKT,
    RESU_PKT
} LINK_PKT_TYPE;


/*
 * link establishment packets
 */
static const char ConnPkt[LINK_PACKET_SIZE] = "CONN";
static const char AcptPkt[LINK_PACKET_SIZE] = "ACPT";
static const char NegoPkt[LINK_PACKET_SIZE] = "NEGO";
static const char NrspPkt[LINK_PACKET_SIZE] = "NRSP";
static const char ResuPkt[LINK_PACKET_SIZE] = "RESU";

/**
 * Time to send the LinkPacket
 */
static AJ_Time sendLinkPacketTime;


/**
 * link configuration information
 */
static uint8_t NegotiationPacket[LINK_PACKET_SIZE + NEGO_PACKET_SIZE];

/**
 * global function pointer for serial transmit funciton
 */
AJ_SerialTxFunc g_AJ_TX;
/************ forward declarations *****************/

static void ScheduleLinkControlPacket(uint32_t timeout);

/********* end of forward declarations *************/

// Converge the remote endpoint's values with my own
static void ProcessNegoPacket(const uint8_t* buffer)
{
    uint16_t max_payload;
    uint8_t proto_version;
    uint8_t window_size;

    max_payload = (((uint16_t) buffer[4]) << 8) | ((uint16_t) buffer[5]);
    AJ_Printf("Read max payload: %u\n", max_payload);
    AJ_SerialLinkParams.packetSize = min(AJ_SerialLinkParams.packetSize, max_payload);

    proto_version = (buffer[6] >> 2) & 0x003F;
    AJ_Printf("Read protocol version: %u\n", proto_version);
    AJ_SerialLinkParams.protoVersion = min(AJ_SerialLinkParams.protoVersion, proto_version);

    // last two bits
    window_size = buffer[6] & 0x03;
    // translate the window size
    switch (window_size) {
    case 0:
        window_size = 1;
        break;

    case 1:
        window_size = 2;
        break;

    case 2:
        window_size = 4;
        break;

    case 4:
        window_size = 8;
        break;

    default:
        AJ_Printf("Invalid window size: %u\n", window_size);
        break;
    }


    AJ_Printf("Read max window size: %u\n", window_size);
    AJ_SerialLinkParams.windowSize = min(AJ_SerialLinkParams.maxWindowSize, window_size);
}

static void SendNegotiationPacket(const char* pkt_type)
{
    uint8_t encoded_window_size = 0;

    // NegoPkt is the packet type
    memset(&NegotiationPacket[0], 0, sizeof(NegotiationPacket));
    // can be NEGO or NRSP
    memcpy(&NegotiationPacket[0], pkt_type, LINK_PACKET_SIZE);

    // max payload length we can support
    NegotiationPacket[4] = (AJ_SerialLinkParams.packetSize & 0xFF00) >> 8;
    NegotiationPacket[5] = (AJ_SerialLinkParams.packetSize & 0x00FF);

    switch (AJ_SerialLinkParams.maxWindowSize) {
    case 1:
        encoded_window_size = 0;
        break;

    case 2:
        encoded_window_size = 1;
        break;

    case 4:
        encoded_window_size = 2;
        break;

    case 8:
        encoded_window_size = 3;
        break;

    default:
        break;
    }

    NegotiationPacket[6] = (AJ_SerialLinkParams.protoVersion << 2) | encoded_window_size;
    AJ_SerialTX_EnqueueCtrl(NegotiationPacket, sizeof(NegotiationPacket), AJ_SERIAL_CTRL);
}


/**
 * a function that periodically sends sync or conf packets depending on
 * the link state
 */
static void SendLinkPacket()
{
    switch (AJ_SerialLinkParams.linkState) {
    case AJ_LINK_UNINITIALIZED:
        /*
         * Send a sync packet.
         */
        AJ_SerialTX_EnqueueCtrl((uint8_t*) ConnPkt, sizeof(ConnPkt), AJ_SERIAL_CTRL);
        AJ_Printf("Send CONN\n");
        ScheduleLinkControlPacket(CONN_TIMEOUT);
        break;

    case AJ_LINK_INITIALIZED:
        /*
         * Send a conf packet.
         */
        SendNegotiationPacket(NegoPkt);
        AJ_Printf("Send NEGO\n");
        ScheduleLinkControlPacket(NEGO_TIMEOUT);
        break;

    case AJ_LINK_ACTIVE:
        /**
         * Do nothing. No more link control packets to be sent.
         */
        ScheduleLinkControlPacket(AJ_TIMER_FOREVER);
        break;
    }
}

/**
 * This function registers the time to send a link control packet
 * specified by the timeout parameter.
 *
 * @param timeout   time (in milliseconds) when the link control packet
 *                  must be sent
 */
static void ScheduleLinkControlPacket(uint32_t timeout)
{
    AJ_InitTimer(&sendLinkPacketTime);
    AJ_TimeAddOffset(&sendLinkPacketTime, timeout);
}

/**
 * This function returns a link packet type, given a packet as an argument.
 *
 *  @param buffer    pointer to the buffer holding the packet
 *  @param bufLen    size of the buffer
 */
static LINK_PKT_TYPE ClassifyPacket(uint8_t* buffer, uint16_t bufLen)
{
    if (bufLen < LINK_PACKET_SIZE) {
        return UNKNOWN_PKT;
    }

    if (0 == memcmp(buffer, ConnPkt, sizeof(ConnPkt))) {
        return CONN_PKT;
    } else if (0 == memcmp(buffer, AcptPkt, sizeof(AcptPkt))) {
        return ACPT_PKT;
    } else if (0 == memcmp(buffer, NegoPkt, sizeof(NegoPkt))) {
        return NEGO_PKT;
    } else if (0 == memcmp(buffer, NrspPkt, sizeof(NrspPkt))) {
        return NRSP_PKT;
    } else if (0 == memcmp(buffer, ResuPkt, sizeof(ResuPkt))) {
        return RESU_PKT;
    }

    return UNKNOWN_PKT;
}


void AJ_Serial_LinkPacket(uint8_t* buffer, uint16_t len)
{
    LINK_PKT_TYPE pktType = ClassifyPacket(buffer, len);

    if (pktType == UNKNOWN_PKT) {
        AJ_Printf("Unknown link packet type %x\n", (int) pktType);
        return;
    }

    switch (AJ_SerialLinkParams.linkState) {
    case AJ_LINK_UNINITIALIZED:
        /*
         * In the uninitialized state we need to respond to conn packets
         * and acpt packets.
         */
        if (pktType == CONN_PKT) {
            AJ_SerialTX_EnqueueCtrl((uint8_t*) AcptPkt, sizeof(AcptPkt), AJ_SERIAL_CTRL);
        } else if (pktType == ACPT_PKT) {
            AJ_Printf("Received sync response - moving to LINK_INITIALIZED\n");
            AJ_SerialLinkParams.linkState = AJ_LINK_INITIALIZED;
            SendNegotiationPacket(NegoPkt);
        }

        break;

    case AJ_LINK_INITIALIZED:
        /*
         * In the initialized state we need to respond to conn packets, nego
         * packets, and nego-resp packets.
         */
        if (pktType == CONN_PKT) {
            AJ_SerialTX_EnqueueCtrl((uint8_t*) AcptPkt, sizeof(AcptPkt), AJ_SERIAL_CTRL);
        } else if (pktType == NEGO_PKT) {
            ProcessNegoPacket(buffer);
            SendNegotiationPacket(NrspPkt);
        } else if (pktType == NRSP_PKT) {
            AJ_Printf("Received nego response - Moving to LINK_ACTIVE\n");
            AJ_SerialLinkParams.linkState = AJ_LINK_ACTIVE;
        }
        break;

    case AJ_LINK_ACTIVE:
        /*
         * In the initialized state we need to respond to nego-resp packets.
         */
        if (pktType == NEGO_PKT) {
            ProcessNegoPacket(buffer);
            SendNegotiationPacket(NrspPkt);
        }
        break;
    }

    /*
     * Ignore any other packets.
     */
    AJ_Printf("Discarding link packet %d\n", pktType);
}

AJ_Status AJ_SerialInit(const char* ttyName,
                        uint16_t bitRate,
                        uint8_t windowSize,
                        uint16_t packetSize)
{
    AJ_Status status;
    if ((windowSize < MIN_WINDOW_SIZE) || (windowSize > MAX_WINDOW_SIZE)) {
        return AJ_ERR_INVALID;
    }

    AJ_Printf("Initializing serial transport\n");

    /** Initialize protocol default values */
    AJ_SerialLinkParams.protoVersion = SLAP_VERSION;
    AJ_SerialLinkParams.maxWindowSize = windowSize;
    AJ_SerialLinkParams.windowSize = windowSize;
    AJ_SerialLinkParams.packetSize = packetSize;
    AJ_SerialLinkParams.linkState = AJ_LINK_UNINITIALIZED;

    /** Initialize serial ports */
    status = AJ_SerialTargetInit(ttyName, bitRate);
    if (status != AJ_OK) {
        return status;
    }

    /** Initialize transmit buffers, state */
    status = AJ_SerialTX_Init();
    if (status != AJ_OK) {
        return status;
    }

    /** Initialize receive buffers, state */
    status = AJ_SerialRX_Init();
    if (status != AJ_OK) {
        return status;
    }

    AJ_SerialLinkParams.linkState = AJ_LINK_UNINITIALIZED;
    ScheduleLinkControlPacket(10);
    return AJ_OK;
}


void AJ_SerialShutdown(void)
{
    AJ_SerialTX_Shutdown();
    AJ_SerialRX_Shutdown();
}


extern AJ_Time resendTime;
extern AJ_Time ackTime;
volatile int dataReceived;
volatile int dataSent = 1;

/** This state machine is called from AJ_SerialRecv and AJ_SerialSend.
 * This processes any buffers copied in the Recieve Callback, adds
 * buffers to be written in the Transmit Callback, resends packets
 * after a timeout and sends pure acks in the case of lack of data to send.
 * Also, this sends link control packets periodically depending on the state.
 */
void AJ_StateMachine()
{
    AJ_Time now;
    AJ_InitTimer(&now);

    if (dataReceived) {
        /* Data has been received in the receive callback, process the data,
         * convert it into SLAP packets, validate and process the packets */
        AJ_ProcessRxBufferList();
    }

    if (dataSent) {
        /* There is space in the transmit free list, queue up more buffers to be
         * sent if there are SLAP packets to be sent
         */
        AJ_FillTxBufferList();
    }

    if (AJ_CompareTime(resendTime, now) < 0) {
        /* Resend any data packets that have not been acked */
        ResendPackets();
    }

    if (AJ_CompareTime(ackTime, now) < 0) {
        /* Send an ack for a received packet.(If there is data to send,
         * the ack is sent as a part of the header, but in this case,
         * this end didnt have data to send, so we send an explicit
         * ack packet.)
         */
        SendAck();
    }

    if (AJ_CompareTime(sendLinkPacketTime, now) < 0) {
        /* Time to send a link packet to get the Link to the active state. */
        SendLinkPacket();
    }
}

void ClearSlippedBuffer(volatile AJ_SlippedBuffer* buf)
{
    while (buf != NULL) {
        volatile AJ_SlippedBuffer* prev = buf;
        AJ_Free(buf->buffer);
        buf = buf->next;
        AJ_Free((void*) prev);
    }
}

#endif /* AJ_SERIAL_CONNECTION */
