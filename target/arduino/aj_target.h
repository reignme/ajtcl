#ifndef _AJ_TARGET_H
#define _AJ_TARGET_H
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

#include <stdio.h>
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

/*
 * AJ_Reboot() is a NOOP on this platform
 */
#define AJ_Reboot()

#endif
