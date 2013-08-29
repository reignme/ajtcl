/**
 * @file
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
#include "aj_target.h"

#include "alljoyn.h"
#include "aj_crypto.h"
#include "aj_debug.h"

typedef struct {
    const char* key;     /* AES key */
    const char* nonce;   /* Nonce */
    uint8_t hdrLen;      /* Number of clear text bytes */
    const char* input;   /* Input text) */
    const char* output;  /* Authenticated and encrypted output for verification */
    uint8_t authLen;     /* Length of the authentication field */
} TEST_CASE;

static TEST_CASE const testVector[] = {
    {
        /* =============== RFC 6130 Packet Vector #1 ================== */
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000003020100a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e",
        "0001020304050607588c979a61c663d2f066d0c2c0f989806d5f6b61dac38417e8d12cfdf926e0",
        8
    },
    {
        /* =============== RFC 6130 Packet Vector #2 ================== */
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000004030201a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "000102030405060772c91a36e135f8cf291ca894085c87e3cc15c439c9e43a3ba091d56e10400916",
        8

    },
    {
        /*===============RFC6130PacketVector#3==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000005040302a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20",
        "000102030405060751b1e5f44a197d1da46b0f8e2d282ae871e838bb64da8596574adaa76fbd9fb0c5",
        8
    },
    {
        /*===============RFC6130PacketVector#4==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000006050403a0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e",
        "000102030405060708090a0ba28c6865939a9a79faaa5c4c2a9d4a91cdac8c96c861b9c9e61ef1",
        8
    },
    {
        /*===============RFC6130PacketVector#5==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000007060504a0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "000102030405060708090a0bdcf1fb7b5d9e23fb9d4e131253658ad86ebdca3e51e83f077d9c2d93",
        8
    },
    {
        /*===============RFC6130PacketVector#6==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000008070605a0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20",
        "000102030405060708090a0b6fc1b011f006568b5171a42d953d469b2570a4bd87405a0443ac91cb94",
        8
    },
    {
        /*===============RFC6130PacketVector#7==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000009080706a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e",
        "00010203040506070135d1b2c95f41d5d1d4fec185d166b8094e999dfed96c048c56602c97acbb7490",
        10
    },
    {
        /*===============RFC6130PacketVector#8==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "0000000a090807a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "00010203040506077b75399ac0831dd2f0bbd75879a2fd8f6cae6b6cd9b7db24c17b4433f434963f34b4",
        10
    },
    {
        /*===============RFC6130PacketVector#9==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "0000000b0a0908a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20",
        "000102030405060782531a60cc24945a4b8279181ab5c84df21ce7f9b73f42e197ea9c07e56b5eb17e5f4e",
        10
    },
    {
        /*===============RFC6130PacketVector#10==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "0000000c0b0a09a0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e",
        "000102030405060708090a0b07342594157785152b074098330abb141b947b566aa9406b4d999988dd",
        10
    },
    {
        /*===============RFC6130PacketVector#11==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "0000000d0c0b0aa0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "000102030405060708090a0b676bb20380b0e301e8ab79590a396da78b834934f53aa2e9107a8b6c022c",
        10
    },
    {
        /*===============RFC6130PacketVector#12==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "0000000e0d0c0ba0a1a2a3a4a5",
        12,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20",
        "000102030405060708090a0bc0ffa0d6f05bdb67f24d43a4338d2aa4bed7b20e43cd1aa31662e7ad65d6db",
        10
    },
    {
        /*===============RFC6130PacketVector#13==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00412b4ea9cdbe3c9696766cfa",
        8,
        "0be1a88bace018b108e8cf97d820ea258460e96ad9cf5289054d895ceac47c",
        "0be1a88bace018b14cb97f86a2a4689a877947ab8091ef5386a6ffbdd080f8e78cf7cb0cddd7b3",
        8
    },
    {
        /*===============RFC6130PacketVector#14==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "0033568ef7b2633c9696766cfa",
        8,
        "63018f76dc8a1bcb9020ea6f91bdd85afa0039ba4baff9bfb79c7028949cd0ec",
        "63018f76dc8a1bcb4ccb1e7ca981befaa0726c55d378061298c85c92814abc33c52ee81d7d77c08a",
        8
    },
    {
        /*===============RFC6130PacketVector#15==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00103fe41336713c9696766cfa",
        8,
        "aa6cfa36cae86b40b916e0eacc1c00d7dcec68ec0b3bbb1a02de8a2d1aa346132e",
        "aa6cfa36cae86b40b1d23a2220ddc0ac900d9aa03c61fcf4a559a4417767089708a776796edb723506",
        8
    },
    {
        /*===============RFC6130PacketVector#16==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00764c63b8058e3c9696766cfa",
        12,
        "d0d0735c531e1becf049c24412daac5630efa5396f770ce1a66b21f7b2101c",
        "d0d0735c531e1becf049c24414d253c3967b70609b7cbb7c499160283245269a6f49975bcadeaf",
        8
    },
    {
        /*===============RFC6130PacketVector#17==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00f8b678094e3b3c9696766cfa",
        12,
        "77b60f011c03e1525899bcaee88b6a46c78d63e52eb8c546efb5de6f75e9cc0d",
        "77b60f011c03e1525899bcae5545ff1a085ee2efbf52b2e04bee1e2336c73e3f762c0c7744fe7e3c",
        8
    },
    {
        /*===============RFC6130PacketVector#18==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00d560912d3f703c9696766cfa",
        12,
        "cd9044d2b71fdb8120ea60c06435acbafb11a82e2f071d7ca4a5ebd93a803ba87f",
        "cd9044d2b71fdb8120ea60c0009769ecabdf48625594c59251e6035722675e04c847099e5ae0704551",
        8
    },
    {
        /*===============RFC6130PacketVector#19==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "0042fff8f1951c3c9696766cfa",
        8,
        "d85bc7e69f944fb88a19b950bcf71a018e5e6701c91787659809d67dbedd18",
        "d85bc7e69f944fb8bc218daa947427b6db386a99ac1aef23ade0b52939cb6a637cf9bec2408897c6ba",
        10
    },
    {
        /*===============RFC6130PacketVector#20==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "00920f40e56cdc3c9696766cfa",
        8,
        "74a0ebc9069f5b371761433c37c5a35fc1f39f406302eb907c6163be38c98437",
        "74a0ebc9069f5b375810e6fd25874022e80361a478e3e9cf484ab04f447efff6f0a477cc2fc9bf548944",
        10
    },
    {
        /*===============RFC6130PacketVector#21==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "0027ca0c7120bc3c9696766cfa",
        8,
        "44a3aa3aae6475caa434a8e58500c6e41530538862d686ea9e81301b5ae4226bfa",
        "44a3aa3aae6475caf2beed7bc5098e83feb5b31608f8e29c38819a89c8e776f1544d4151a4ed3a8b87b9ce",
        10
    },
    {
        /*===============RFC6130PacketVector#22==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "005b8ccbcd9af83c9696766cfa",
        12,
        "ec46bb63b02520c33c49fd70b96b49e21d621741632875db7f6c9243d2d7c2",
        "ec46bb63b02520c33c49fd7031d750a09da3ed7fddd49a2032aabf17ec8ebf7d22c8088c666be5c197",
        10
    },
    {
        /*===============RFC6130PacketVector#23==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "003ebe94044b9a3c9696766cfa",
        12,
        "47a65ac78b3d594227e85e71e2fcfbb880442c731bf95167c8ffd7895e337076",
        "47a65ac78b3d594227e85e71e882f1dbd38ce3eda7c23f04dd65071eb41342acdf7e00dccec7ae52987d",
        10
    },
    {
        /*===============RFC6130PacketVector#24==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "008d493b30ae8b3c9696766cfa",
        12,
        "6e37a6ef546d955d34ab6059abf21c0b02feb88f856df4a37381bce3cc128517d4",
        "6e37a6ef546d955d34ab6059f32905b88a641b04b9c9ffb58cc390900f3da12ab16dce9e82efa16da62059",
        10
    },
    {
        /*===============Authenticationonly==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "008d493b30ae8b3c9696766cfa",
        33,
        "6e37a6ef546d955d34ab6059abf21c0b02feb88f856df4a37381bce3cc128517d4",
        "6e37a6ef546d955d34ab6059abf21c0b02feb88f856df4a37381bce3cc128517d4ca35dc8a1ebd6bc7ead7",
        10
    },
    {
        /*===============Noheader==================*/
        "d7828d13b2b0bdc325a76236df93cc6b",
        "008d493b30ae8b3c9696766cfa",
        0,
        "6e37a6ef546d955d34ab6059abf21c0b02feb88f856df4a37381bce3cc128517d4",
        "36ecbf5cdcf736d6080f6b4f54b03078c1d19cb2e0b18a3c3883aa48b3abee5a795300f8778a19bd45bc34",
        10
    },
    {
        /*===============16byteauthenticationfield==================*/
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000003020100a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e",
        "0001020304050607588c979a61c663d2f066d0c2c0f989806d5f6b61dac384509da654e32deac369c2dae7133cb08d",
        16
    },
    {
        /* =============== Small payload ================== */
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000003020100a0a1a2a3a4a5",
        8,
        "000102030405060708090a0b0c0d",
        "0001020304050607588c979a61c6b7c00bb077809cae",
        8
    },
    {
        /* =============== Small payload ================== */
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000003020100a0a1a2a3a4a5",
        1,
        "000102030405060708090a0b0c0d0e0f1011",
        "0051879e9568cd6ad5e97dc9ddd9e29087643eb868cbf8e0e0cc",
        8
    },
    {
        /* =============== Minimal header and payload ================== */
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
        "00000003020100a0a1a2a3a4a5",
        1,
        "0001",
        "0051c0eed548220130d4",
        8
    }
};

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    size_t i;
    char out[128];

    for (i = 0; i < ArraySize(testVector); i++) {

        uint8_t key[16];
        uint8_t msg[64];
        uint8_t nonce[16];
        uint32_t nlen = (uint32_t)strlen(testVector[i].nonce) / 2;
        uint32_t mlen = (uint32_t)strlen(testVector[i].input) / 2;

        AJ_HexToRaw(testVector[i].key, 0, key, sizeof(key));
        AJ_HexToRaw(testVector[i].nonce, 0, nonce, nlen);
        AJ_HexToRaw(testVector[i].input, 0, msg, mlen);

        status = AJ_Encrypt_CCM(key, msg, mlen, testVector[i].hdrLen, testVector[i].authLen, nonce, nlen);
        if (status != AJ_OK) {
            AJ_Printf("Encryption failed (%d) for test #%zu\n", status, i);
            goto ErrorExit;
        }
        AJ_RawToHex(msg, mlen + testVector[i].authLen, out, sizeof(out));
        if (strcmp(out, testVector[i].output) != 0) {
            AJ_Printf("Encrypt verification failure for test #%zu\n%s\n", i, out);
            goto ErrorExit;
        }
        /*
         * Verify decryption.
         */
        status = AJ_Decrypt_CCM(key, msg, mlen, testVector[i].hdrLen, testVector[i].authLen, nonce, nlen);
        if (status != AJ_OK) {
            AJ_Printf("Authentication failure (%d) for test #%zu\n", status, i);
            goto ErrorExit;
        }
        AJ_RawToHex(msg, mlen, out, sizeof(out));
        if (strcmp(out, testVector[i].input) != 0) {
            AJ_Printf("Decrypt verification failure for test #%zu\n%s\n", i, out);
            goto ErrorExit;
        }
        AJ_Printf("Passed and verified test #%zu\n", i);
    }

    AJ_Printf("AES CCM unit test PASSED\n");

    {
        static const char expect[] = "f19787716404918ca20f174cff2e165f21b17a70c472480ae91891b5bb8dd261cbd4273612d41bc6";
        const char secret[] = "1234ABCDE";
        const char seed[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234";
        uint8_t key[40];
        const char* inputs[3];
        uint8_t length[3];

        inputs[0] = secret;
        length[0] = (uint8_t)strlen(secret);
        inputs[1] = seed;
        length[1] = (uint8_t)strlen(seed);
        inputs[2] = "prf test";
        length[2] = 8;

        status = AJ_Crypto_PRF((const uint8_t**)inputs, length, ArraySize(inputs), key, sizeof(key));
        if (status != AJ_OK) {
            AJ_Printf("AJ_Crypto_PRF %d\n", status);
            goto ErrorExit;
        }
        AJ_RawToHex(key, sizeof(key), out, sizeof(out));
        if (strcmp(out, expect) != 0) {
            AJ_Printf("AJ_Crypto_PRF failed: %d\n", status);
            goto ErrorExit;
        }
        AJ_Printf("AJ_Crypto_PRF test PASSED: %d\n", status);
    }

    return 0;

ErrorExit:

    AJ_Printf("AES CCM unit test FAILED\n");
    return 1;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
