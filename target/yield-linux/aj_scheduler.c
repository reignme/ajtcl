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

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <setjmp.h>

#include "aj_target.h"
#include "aj_status.h"
#include "aj_util.h"



static jmp_buf superEnv;
static jmp_buf waitEnv;

static AJ_Event alwaysSet = { 1, 0 };
static AJ_Event readEvent = { 0, 0 };
static AJ_Event writeEvent = { 0, 0 };
static AJ_Event timeEvent = { 0, 0 };
static AJ_Event exitEvent = { 0, 0 };
static AJ_Event* waitEvents[] = { &readEvent, &writeEvent, &timeEvent, &exitEvent };


/*
 * Call stack for AJ_Main()
 */
static uint64_t CallStack[1024];
AJ_MainRoutineType AJ_MainRoutine = NULL;



static uint32_t bitFlagEventsRequested = 0;
static uint32_t bitFlagEventsRaised = AJWAITEVENT_ALWAYSSET;



/*
 * called to indicate an event has happened.
 *
 * @param[in]  bitFlag indicates which events to signal
 *
 */
void AJ_Schedule(uint32_t bitFlag)
{
    if (bitFlag & AJWAITEVENT_TIMER) {
        timeEvent.isSet = 1;
        bitFlagEventsRaised |= AJWAITEVENT_TIMER;
    }
    if (bitFlag & AJWAITEVENT_READ) {
        readEvent.isSet = 1;
        bitFlagEventsRaised |= AJWAITEVENT_READ;
    }
    if (bitFlag & AJWAITEVENT_WRITE) {
        writeEvent.isSet = 1;
        bitFlagEventsRaised |= AJWAITEVENT_WRITE;
    }
    if (bitFlag & AJWAITEVENT_EXIT) {
        exitEvent.isSet = 1;
        bitFlagEventsRaised |= AJWAITEVENT_EXIT;
    }

    return AJ_OK;
}


/*
 * called by client code to wait until one of the events has happened.
 *
 * @param[in]  bitFlag indicates which events to wait for.
 *
 */
AJ_Status AJ_YieldUntil(uint32_t bitFlag)
{
    AJ_Status status = (AJ_Status)setjmp(waitEnv);
    if (status == AJ_OK) {
        bitFlagEventsRequested = bitFlag;
        longjmp(superEnv, 1);
    }
    return status;
}

/*
 * Reset the state of an event to not-raised.
 *
 * @param[in]  bitFlag indicates which events to reset.
 *
 */
AJ_Status AJ_ClearEvents(uint32_t bitFlag)
{
    if (bitFlag & AJWAITEVENT_TIMER) {
        timeEvent.isSet = 0;
    }
    if (bitFlag & AJWAITEVENT_READ) {
        readEvent.isSet = 0;
    }
    if (bitFlag & AJWAITEVENT_WRITE) {
        writeEvent.isSet = 0;
    }
    if (bitFlag & AJWAITEVENT_EXIT) {
        exitEvent.isSet = 0;
    }

    return AJ_OK;
}

/*
 * Retrive the state of an event
 *
 * @param[in]  bitFlag indicates which single event to check.
 * @return returns the current state of the event (set or not-set)
 */
int AJ_GetEventState(uint32_t bitFlag)
{
    int eventState = 0;
    switch (bitFlag) {
    case AJWAITEVENT_TIMER:
        eventState = timeEvent.isSet;
        break;

    case AJWAITEVENT_READ:
        eventState = readEvent.isSet;
        break;

    case AJWAITEVENT_WRITE:
        eventState = writeEvent.isSet;
        break;

    case AJWAITEVENT_EXIT:
        eventState = exitEvent.isSet;
        break;

    }
    return eventState;
}


/*
 *   Save CallStack address into the jmp_buf.
 *   This will make all of the AJ_Main calls take place inside the CallStack buffer
 */
void CallAllJoyn()
{
    jmp_buf tmpEnv;
    if (!setjmp(tmpEnv)) {

        /*
           The inline assembly is here to adjust the stack pointer to be stored in the same way the runtime library does.
           It probably goes without saying that this is _very_ system specific, based on how glib saves context
         */
        uint64_t mangled = (uint64_t)CallStack + sizeof(CallStack);
        asm (
            "xor  %%fs:0x30,%0\n" /// retrieve PTR_MANGLE value
            "rol  $0x11,%0\n"     /// rol it around
            "mov  %0,%1\n"        /// save it back to mangled.
            : "=r" (mangled)
            : "0" (mangled)
            );
        (tmpEnv[0]).__jmpbuf[6] = mangled;
        // now restore the environment back to the setjmp above, but now the callstack is located in the new CallStack buffer.
        longjmp(tmpEnv, 1);
    } else {
        AJ_MainRoutine();
    }
}

int AJ_Loop()
{
    static uint8_t firstTime = 1;
    uint8_t i;
    uint8_t wasSet = FALSE;

    /*
     * The first time through we need to call into the AllJoyn main program.
     */
    if (firstTime) {
        firstTime = 0;
        if (!setjmp(superEnv)) {
            CallAllJoyn();
        }
        return 0;
    }
    /*
     * Check if any events are set.
     */
    for (i = 0; i < (sizeof(waitEvents) / sizeof(waitEvents[0])); ++i) {
        if (exitEvent.isSet) {
            return(0);
        } else if (waitEvents[i]->isSet) {
            wasSet = TRUE;
            break;
        }
    }
    /*
     * If there are no events set there is nothing to do yet
     */
    if (wasSet) {
        if (setjmp(superEnv) == 0) {
            longjmp(waitEnv, 1);
        }
    }
    return AJ_OK;
}


