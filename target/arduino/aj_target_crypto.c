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

#include "Arduino.h"    // for digitalRead, digitalWrite, etc

#include "aj_target.h"
#include "aj_crypto.h"

int analogPin = 3;
static uint8_t seed[16];
static uint8_t key[16];
/*
 * The host has various ADC's. We are going to accumulate entropy by repeatedly
 * reading the ADC and accumulating the least significant bit or each reading.
 */
int GatherBits(uint8_t* buffer, uint32_t len)
{
    int i;
    uint32_t val;

    /*
     * Start accumulating entropy one bit at a time
     */
    for (i = 0; i < (8 * len); ++i) {
        val = analogRead(analogPin);
        buffer[i / 8] ^= ((val & 1) << (i & 7));
    }
    return val;
}

void AJ_RandBytes(uint8_t* rand, uint32_t len)
{
    ///*
    // * On the first call we need to accumulate entropy
    // * for the seed and the key.
    // */
    if (seed[0] == 0) {
        GatherBits(seed, sizeof(seed));
        GatherBits(key, sizeof(key));
    }
    AJ_AES_Enable(key);
    /*
     * This follows the NIST guidelines for using AES as a PRF
     */
    while (len) {
        *rand = random(256);
        len -= 1;
        rand += 1;
    }
    AJ_AES_Disable();
}
