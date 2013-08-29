#ifndef _AJ_TIMER_H
#define _AJ_TIMER_H
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

#include "aj_target.h"

#include "aj_util.h"

/**
 * A timer registered with this value will never expire but can be refreshed.
 */
#define AJ_TIMER_FOREVER  -1



/**
 * Function prototype for a timer callback
 *
 * @param timerId  The identifier for the timer.
 * @param context  A context passed in when the timer was registered.
 */
typedef void (AJ_TimerCallback)(uint32_t timerId, void* context);


typedef struct _AJ_Timer {
    uint32_t id;
    AJ_Time timeNextRaised;
    AJ_TimerCallback* callback;
    void* context;
    struct _AJ_Timer* next;
} AJ_Timer;




/**
 * Register a timer.
 *
 * @param timeout        The timeout for the timer
 * @param timerCallback  Function called when the timeout expires
 * @param context        A context that will be passed to the callback function
 * @param timerId        Returns a timer identifier that can be used to cancel the timer. This is
 *                       guaranteed to never be zero.
 */
AJ_Status AJ_TimerRegister(uint32_t timeout,
                           AJ_TimerCallback timerCallback,
                           void* context,
                           uint32_t* timerId);

/**
 * Refresh a timer by providing a new timeout value.
 *
 * @param timerId        Identifies the timer to reset.
 * @param timeout        The new timeout for the timer
 */
AJ_Status AJ_TimerRefresh(uint32_t timerId,
                          uint32_t timeout);

/**
 * Cancel a timer.
 *
 * @param timerId        Identifies the timer to cancel.
 * @param keep           should the cancelled timer be move to a inactive list
 */
void AJ_TimerCancel(uint32_t timerId, uint8_t keep);


#endif /* _AJ_TIMER_H */
