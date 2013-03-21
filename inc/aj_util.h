#ifndef _AJ_UTIL_H
#define _AJ_UTIL_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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
#include "aj_status.h"

/**
 * Structure for holding a time
 */
typedef struct _AJ_Time {
    uint16_t milliseconds;      /**< The number of milliseconds in the time */
    uint32_t seconds;           /**< The number of seconds in the time */
} AJ_Time;

/**
 * Get the time elapsed in milliseconds since this function was called with the same timer.
 * Call AJ_InitTimer() to initialize the timer before calling this function.
 *
 * @param timer      Tracks relative time.
 * @param cumulative If TRUE the elapsed time returned is cumulative, otheriwise it is relative to
 *                   the the time the function was called.
 *
 * @return  The elapsed milliseconds.
 */
uint32_t AJ_GetElapsedTime(AJ_Time* timer, uint8_t cumulative);

/**
 * Initialize a timer
 *
 * @param timer  The timer to initialize
 */
#define AJ_InitTimer(timer)  (void)AJ_GetElapsedTime(timer, FALSE)

/**
 * Suspend to low-power mode on embedded devices
 *
 * @param msec milliseconds to wait before waking up
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_SuspendWifi(uint32_t msec);


/**
 * Pause the current thread for a number of milliseconds
 */
void AJ_Sleep(uint32_t time);

/**
 * Allocate memory. This function should only be used for allocation of short term buffers that
 * might otherwise be allocated on the stack.
 */
void* AJ_Malloc(size_t size);

/**
 * Free memory previously allocated by AJ_Malloc()
 */
void AJ_Free(void* mem);


/**
 * Macro for getting the size of an array variable
 */
#define ArraySize(a)  (sizeof(a) / sizeof(a[0]))

/**
 * Find position of first instance of any character in a given string in a string.
 *
 * @param str   The string to search
 * @param chars The characters to search for
 *
 * @return  The position of the first instance of the character in the string or -1 if the character
 *          does not appear in the string.
 *
 */
int32_t AJ_StringFindFirstOf(const char* str, char* chars);

/**
 * Convert a raw byte string to NUL terminated ascii hex string. It is permitted for raw and hex to
 * point to the same memory location.
 *
 * @param raw     The bytes to convert
 * @param rawLen  The number of bytes to convert
 * @param hex     The buffer to receive the converted hex data
 * @param hexLen  The length of the hex buffer
 *
 * @return
 *          - AJ_OK if the string was converted
 *          - AJ_ERR_RESOURCES if the hexLen is too small to fit the converted string.
 */
AJ_Status AJ_RawToHex(const uint8_t* raw, size_t rawLen, char* hex, size_t hexLen);

/**
 * Convert a NUL terminated ascii hex string to raw bytes. It is permitted for raw and hex buffer to
 * point to the same memory location.
 *
 * @param hex     The buffer containing the ASCII hex string
 * @param hexLen  Length of the hex data to decode or zero to decode entire string.
 * @param raw     The bytes to convert
 * @param rawLen  The number of bytes to convert
 *
 * @return
 *          - AJ_OK if the string was converted.
 *          - AJ_ERR_RESOURCES if the rawLen is too small to fit the converted string.
 *          - AJ_ERR_UNEXPECTED if the string is not a hexidecimal string.
 */
AJ_Status AJ_HexToRaw(const char* hex, size_t hexLen, uint8_t* raw, size_t rawLen);

/**
 * get a line of input from the the file pointer (most likely stdin).
 * This will capture the the num-1 characters or till a newline character is
 * entered.
 *
 * @param[out] str a pointer to a character array that will hold the user input
 * @param[in]  num the size of the character array 'str'
 * @param[in]  fp  the file pointer the sting will be read from. (most likely stdin)
 *
 * @return returns the same string as 'str' if there has been a read error a null
 *                 pointer will be returned and 'str' will remain unchanged.
 */
char* AJ_GetLine(char*str, size_t num, void*fp);

#endif
