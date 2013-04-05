#ifndef _AJ_DEBUG_H
#define _AJ_DEBUG_H
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

#include "aj_target.h"
#include "aj_msg.h"

#ifndef NDEBUG

/**
 * Dump message name and content. if body is true, dump raw data
 *
 * @param tag       tag name of message
 * @param msg       message header
 * @param body      if true, dump raw data
 */
void AJ_DumpMsg(const char* tag, AJ_Message* msg, uint8_t body);

/**
 * Dump raw data
 *
 * @param tag       tag name of message
 * @param data      start addres to dump
 * @param len       length to dump
 */
void AJ_DumpBytes(const char* tag, const uint8_t* data, uint32_t len);

/**
 * Debug utility function that converts numerical status to a readable string
 *
 * @param status  A status code
 */
const char* AJ_StatusText(AJ_Status status);

#else

#define AJ_DumpMsg(tag, msg, body)
#define AJ_DumpBytes(tag, data, len)

#endif

#endif
