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
       Counter mode the hard way because the SSL CTR-mode API is just wierd.
     */
    while (len) {
        size_t n = min(len, 16);
        uint8_t enc[16];
        uint8_t* p = enc;
        uint16_t counter = (ctr[14] << 8) | ctr[15];
        len -= n;
        AES_encrypt(ctr, enc, &keyState);
        while (n--) {
            *out++ = *p++ ^ *in++;
        }
        ++counter;
        ctr[15] = counter;
        ctr[14] = counter >> 8;
    }
}

void AJ_AES_CBC_128_ENCRYPT(const uint8_t* key, const uint8_t* in, uint8_t* out, uint32_t len, uint8_t* iv)
{
    AES_cbc_encrypt(in, out, len, &keyState, iv, AES_ENCRYPT);
}

void AJ_AES_ECB_128_ENCRYPT(const uint8_t* key, const uint8_t* in, uint8_t* out)
{
    AES_encrypt(in, out, &keyState);
}

void AJ_RandBytes(uint8_t* rand, uint32_t len)
{
    BIGNUM* bn = BN_new();
    BN_rand(bn, len * 8, -1, 0);
    BN_bn2bin(bn, rand);
    BN_free(bn);
}

