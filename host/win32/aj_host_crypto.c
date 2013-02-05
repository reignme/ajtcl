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

#include "aj_host.h"
#include "aj_crypto.h"
#include <openssl/aes.h>
#include <openssl/bn.h>

static AES_KEY keyState;

void AJ_AES_Enable(const uint8_t* key)
{
    AES_set_encrypt_key(key, 16 * 8, &keyState);
}

void AJ_AES_Disable(void)
{
}

void AJ_AES_CTR_128(const uint8_t* key, const uint8_t* in, uint8_t* out, uint32_t len, uint8_t* ctr)
{
    /*
       Counter mode the hard way because the SSL CTR-mode API is just weird.
     */
    while (len) {
        uint32_t n = min(len, 16);
        uint8_t enc[16];
        uint8_t* p = enc;
        uint16_t counter = (ctr[14] << 8) | ctr[15];
        len -= n;
        AES_encrypt(ctr, enc, &keyState);
        while (n--) {
            *out++ = *p++ ^ *in++;
        }
        ++counter;
        ctr[15] = (uint8_t)counter;
        ctr[14] = (uint8_t)(counter >> 8);
    }
}

void AJ_AES_CBC_128_ENCRYPT(const uint8_t* key, const uint8_t* in, uint8_t* out, uint32_t len, uint8_t* iv)
{
    AES_cbc_encrypt(in, out, len, &keyState, iv, AES_ENCRYPT);
}

void AJ_RandBytes(uint8_t* rand, uint32_t len)
{
    BIGNUM* bn = BN_new();
    BN_rand(bn, len * 8, -1, 0);
    BN_bn2bin(bn, rand);
    BN_free(bn);
}

