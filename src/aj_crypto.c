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


#include "aj_target.h"
#include "aj_util.h"
#include "aj_crypto.h"
#include "aj_debug.h"

/*
 * Enables fine-grained tracing for debugging new implementations.
 */
#define CCM_TRACE 0

#if CCM_TRACE
#define Trace(tag, data, len) AJ_DumpBytes(tag, data, len)
#else
#define Trace(tag, data, len)
#endif

/*
 * AES-128 processes data 16 bytes at a time
 */
#define BLOCKSZ  16

/*
 * Struct for a single AES data block
 */
typedef struct _AES_Block {
    uint8_t data[BLOCKSZ];
} AES_Block;

#define ZERO(b)  memset((b).data, 0, BLOCKSZ);

/*
 * Struct holding CCM state information
 */
typedef struct _CCM_Context {
    AES_Block T;      /* authentication tag */
    AES_Block ivec0;  /* ivec for CBC MAC */
    AES_Block ivec;   /* ivec for CTR mode encrypt/decrypt */
    union {
        AES_Block A;   /* Working data for CBC MAC */
        AES_Block B_0; /* Initial block for CBC MAC */
    };
} CCM_Context;

/**
 * Compute the CBC MAC over some data
 */
static void CBC_MAC(const uint8_t* key, const uint8_t* in, uint32_t len, CCM_Context* context)
{
    while (len >= BLOCKSZ) {
        AJ_AES_CBC_128_ENCRYPT(key, in, context->T.data, BLOCKSZ, context->ivec0.data);
        Trace("After AES", context->T.data, BLOCKSZ);
        in += BLOCKSZ;
        len -= BLOCKSZ;
    }
    if (len) {
        ZERO(context->A);
        memcpy(context->A.data, in, len);
        AJ_AES_CBC_128_ENCRYPT(key, context->A.data, context->T.data, BLOCKSZ, context->ivec0.data);
        Trace("After AES", context->T.data, BLOCKSZ);
    }
}

/**
 * Compute the AES-CCM authentication tag.
 */
static void Compute_CCM_AuthTag(const uint8_t* key,
                                CCM_Context* context,
                                const uint8_t* msg,
                                uint32_t mLen,
                                uint32_t hdrLen)
{
    /*
     * Initialize CBC-MAC with B_0 initialization vector is 0.
     */
    Trace("CBC IV in", context->B_0.data, BLOCKSZ);
    AJ_AES_CBC_128_ENCRYPT(key, context->B_0.data, context->T.data, BLOCKSZ, context->ivec0.data);
    Trace("CBC IV out", context->T.data, BLOCKSZ);
    /*
     * Compute CBC-MAC for the add data.
     */
    if (hdrLen) {
        uint32_t firstFew;
        /*
         * This encodes the header data length and the first few bytes of the header data
         */
        ZERO(context->A);
        context->A.data[0] = (uint8_t)(hdrLen >> 8);
        context->A.data[1] = (uint8_t)(hdrLen >> 0);
        firstFew = min(hdrLen, 14);
        memcpy(&context->A.data[2], msg, firstFew);
        /*
         * Adjust for the hdr data bytes that were encoded in the length block
         */
        msg += firstFew;
        hdrLen -= firstFew;
        /*
         * Continue the MAC by encrypting the length block
         */
        Trace("Before AES", context->A.data, BLOCKSZ);
        AJ_AES_CBC_128_ENCRYPT(key, context->A.data, context->T.data, BLOCKSZ, context->ivec0.data);
        Trace("After AES", context->T.data, BLOCKSZ);
        /*
         * Continue computing the CBC-MAC
         */
        CBC_MAC(key, msg, hdrLen, context);
        msg += hdrLen;
    }
    /*
     * Continue computing CBC-MAC over the message data.
     */
    if (mLen) {
        CBC_MAC(key, msg, mLen, context);
    }
    Trace("CBC-MAC", context->T.data, context->M);
}

static CCM_Context* InitCCMContext(const uint8_t* nonce, uint32_t nLen, uint32_t hdrLen, uint32_t msgLen, uint8_t M)
{
    int i;
    int l;
    uint8_t L  = 15 - max(nLen, 11);
    uint8_t flags = ((hdrLen) ? 0x40 : 0) | (((M - 2) / 2) << 3) | (L - 1);
    CCM_Context* context;

    AJ_ASSERT(nLen <= 15);

    context = (CCM_Context*)AJ_Malloc(sizeof(CCM_Context));
    if (context) {
        memset(context, 0, sizeof(CCM_Context));
        /*
         * Set ivec and other initial args.
         */
        context->ivec.data[0] = L - 1;
        memcpy(&context->ivec.data[1], nonce, nLen);
        /*
         * Compute the B_0 block. This encodes the flags, the nonce, and the message length.
         */
        context->B_0.data[0] = flags;
        memcpy(&context->B_0.data[1], nonce, nLen);
        for (i = 15, l = msgLen - hdrLen; l != 0; i--) {
            context->B_0.data[i] = (uint8_t)l;
            l >>= 8;
        }
    }
    return context;
}

/*
 * Implements AES-CCM (Counter with CBC-MAC) encryption as described in RFC 3610
 */
AJ_Status AJ_Encrypt_CCM(const uint8_t* key,
                         uint8_t* msg,
                         uint32_t msgLen,
                         uint32_t hdrLen,
                         uint8_t tagLen,
                         const uint8_t* nonce,
                         uint32_t nLen)
{
    AJ_Status status = AJ_OK;
    CCM_Context* context;

    if (!(context = InitCCMContext(nonce, nLen, hdrLen, msgLen, tagLen))) {
        return AJ_ERR_RESOURCES;
    }
    /*
     * Do any platform specific operations to enable AES
     */
    AJ_AES_Enable(key);
    /*
     * Compute the authentication tag
     */
    Compute_CCM_AuthTag(key, context, msg, msgLen - hdrLen, hdrLen);
    /*
     * Encrypt the authentication tag
     */
    AJ_AES_CTR_128(key, context->T.data, msg + msgLen, tagLen, context->ivec.data);
    Trace("CTR Start", context->ivec.data, BLOCKSZ);
    /*
     * Encrypt the message
     */
    if (msgLen != hdrLen) {
        AJ_AES_CTR_128(key, msg + hdrLen, msg + hdrLen, msgLen - hdrLen, context->ivec.data);
    }
    /*
     * Balance the enable call above
     */
    AJ_AES_Disable();
    /*
     * Done with the context
     */
    AJ_Free(context);
    return status;
}

/*
 * Implements AES-CCM (Counter with CBC-MAC) decryption as described in RFC 3610
 */
AJ_Status AJ_Decrypt_CCM(const uint8_t* key,
                         uint8_t* msg,
                         uint32_t msgLen,
                         uint32_t hdrLen,
                         uint8_t tagLen,
                         const uint8_t* nonce,
                         uint32_t nLen)
{
    AJ_Status status = AJ_OK;
    CCM_Context* context;

    if (!(context = InitCCMContext(nonce, nLen, hdrLen, msgLen, tagLen))) {
        return AJ_ERR_RESOURCES;
    }
    /*
     * Do any platform specific operations to enable AES
     */
    AJ_AES_Enable(key);
    /*
     * Decrypt the authentication field
     */
    AJ_AES_CTR_128(key, msg + msgLen, msg + msgLen, tagLen, context->ivec.data);
    /*
     * Decrypt message.
     */
    if (msgLen != hdrLen) {
        AJ_AES_CTR_128(key, msg + hdrLen, msg + hdrLen, msgLen - hdrLen, context->ivec.data);
    }
    /*
     * Compute and verify the authentication tag T.
     */
    Compute_CCM_AuthTag(key, context, msg, msgLen - hdrLen, hdrLen);
    /*
     * Balance the enable call above
     */
    AJ_AES_Disable();
    if (memcmp(context->T.data, msg + msgLen, tagLen) != 0) {
        /*
         * Authentication failed Clear the decrypted data
         */
        memset(msg, 0, msgLen + tagLen);
        status = AJ_ERR_SECURITY;
    }
    /*
     * Done with the context
     */
    AJ_Free(context);
    return status;
}

AJ_Status AJ_Crypto_PRF(const uint8_t** inputs,
                        const uint8_t* lengths,
                        uint32_t count,
                        uint8_t* out,
                        uint32_t outLen)
{
    AJ_Status status = AJ_OK;
    uint8_t nonce[4];
    uint32_t inLen = 0;
    uint8_t* inBuf;
    uint8_t* key;
    uint8_t* p;
    uint32_t i;

    for (i = 0; i < count; ++i) {
        inLen += lengths[i];
    }
    if (inLen <= 32) {
        return AJ_ERR_INVALID;
    }
    /*
     * Need 16 bytes at the end for the CCM-MAC
     */
    inBuf = (uint8_t*)AJ_Malloc(inLen + 16);
    if (!inBuf) {
        return AJ_ERR_RESOURCES;
    }
    /*
     * Concatenate the inputs
     */
    for (i = 0, p = inBuf; i < count; ++i) {
        memcpy(p, inputs[i], lengths[i]);
        p += lengths[i];
    }
    /*
     * Clear the nonce (it's declared as an array of bytes because of endianess)
     */
    *((uint32_t*)nonce) = 0;
    /*
     * The key first 16 bytes of the input is used as the AES key.
     */
    key = inBuf;
    inLen -= 16;
    inBuf += 16;
    while (outLen) {
        uint32_t len =  min(16, outLen);
        status = AJ_Encrypt_CCM(key, inBuf, inLen, inLen, 16, nonce, sizeof(nonce));
        if (status != AJ_OK) {
            break;
        }
        /*
         * Append CCM-MAC to the output buffer
         */
        memcpy(out, inBuf + inLen, len);
        outLen -= len;
        out += len;
        ++nonce[0];
    }
    inBuf -= 16;
    AJ_Free(inBuf);
    return status;
}

AJ_Status AJ_RandHex(char* rand, uint32_t bufLen, uint32_t len)
{
    AJ_RandBytes((uint8_t*)rand, len);
    return AJ_RawToHex((const uint8_t*) rand, len, rand, bufLen, FALSE);
}
