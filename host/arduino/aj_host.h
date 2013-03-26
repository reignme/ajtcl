#ifndef _AJ_HOST_H
#define _AJ_HOST_H
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

#include <stdint.h>
#include <stddef.h>
typedef signed char int8_t;           /** 8-bit signed integer */
typedef unsigned char uint8_t;        /** 8-bit unsigned integer */
typedef signed long long int64_t;     /** 64-bit signed integer */
typedef unsigned long long uint64_t;  /** 64-bit unsigned integer */

typedef uint16_t suint32_t;  /* amount of data sent into a socket */


#include <string.h>
#include <malloc.h>
#include <assert.h>

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

// Begin Memory Diagnostics
static const char* ramstart = (char*)0x20070000;
static const char* ramend = (char*)0x20088000;
extern char _end;

inline int stack_used() {
    register char* stack_ptr asm ("sp");
    return (ramend - stack_ptr);
}

inline int static_used() {
    return (&_end - ramstart);
}

inline int heap_used() {
    struct mallinfo mi = mallinfo();
    return (mi.uordblks);
}

void ram_diag();

// End Memory Diagnostics

#define HOST_IS_LITTLE_ENDIAN  TRUE
#define HOST_IS_BIG_ENDIAN     FALSE

#ifdef WIFI_UDP_WORKING
    #include <WiFi.h>
    #include <WiFiUdp.h>
#else
    #include <Ethernet.h>
    #include <EthernetUdp.h>
#endif

#ifndef NDEBUG
    #define AJ_Printf(fmat, ...) \
    do { printf(fmat, ## __VA_ARGS__); } while (0)
#else
    #define AJ_Printf(fmat, ...)
#endif

#define AJ_ASSERT(x) assert(x)

#endif
