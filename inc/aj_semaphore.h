#ifndef _AJ_SEMAPHORE_H
#define _AJ_SEMAPHORE_H
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

/**
 * This is the maximum number of simultaneous accessors for a semaphore
 */
#define AJ_SEMAPHORE_VALUE_MAX 0x7fff


/**
 * Create a semaphore.
 *
 * @param name               A string name for the semaphore (optional)
 * @param count              the initial count value
 */
AJ_Semaphore* AJ_SemaphoreCreate(char* name,
                                 int32_t count);

/**
 * Destory a semaphore.
 *
 * @param sem                Identifies a semaphore to destroy
 */
void AJ_SemaphoreDestroy(AJ_Semaphore* sem);

/**
 * wait for a semaphore.
 *
 * @param sem                Identifies a semaphore to wait for
 */
AJ_Status AJ_SemaphoreWait(AJ_Semaphore* sem);

/**
 * wait for a semaphore until a timeout expires
 *
 * @param sem                Identifies a semaphore to wait for
 * @param timeout            Identifies the amount of time to wait for the semaphore
 */
AJ_Status AJ_SemaphoreWaitTimed(AJ_Semaphore* sem,
                                uint32_t timeout);

/**
 * unlock a semaphore.
 *
 * @param sem        Identifies a semaphore to unlock
 */
AJ_Status AJ_SemaphoreUnlock(AJ_Semaphore* sem);


#endif /* _AJ_SEMAPHORE_H */

