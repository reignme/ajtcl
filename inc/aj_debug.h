#ifndef _AJ_DEBUG_H
#define _AJ_DEBUG_H
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
#include "aj_msg.h"

#ifndef NDEBUG

void AJ_DumpMsg(const char* tag, AJ_Message* msg, uint8_t body);

void AJ_DumpBytes(const char* tag, const uint8_t* data, uint32_t len);

#else

#define AJ_DumpMsg(tag, msg, body)
#define AJ_DumpBytes(tag, data, len)

#endif

#endif
