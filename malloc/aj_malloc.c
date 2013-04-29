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
#include <assert.h>

#include "aj_target.h"
#include "aj_util.h"


#define MIN_BLOCK_SIZE 16

typedef struct _MemBlock {
    struct _MemBlock* next;
    uint8_t mem[MIN_BLOCK_SIZE - sizeof(void*)];
} MemBlock;

typedef struct _MemPool {
    const uint16_t size;     /* Size of the pool entries in bytes */
    const uint16_t entries;  /* Number of entries in this pool */
    void* endOfPool;         /* Address of end of this pool */
    MemBlock* freeList;      /* Linked free list for this pool */
} MemPool;

static MemPool memPools[] = {
    { 32,   1,  NULL, NULL },
    { 96,   4,  NULL, NULL },
    { 192,  1,  NULL, NULL }
};

#define HEAP_SIZE 720

static uint32_t heap[HEAP_SIZE / 4];


static void InitPools()
{
#ifndef NDEBUG
    size_t totalSz = 0;
#endif
    size_t i;
    size_t n;
    uint8_t* heapPtr = (uint8_t*)heap;

    for (i = 0; i < ArraySize(memPools); ++i) {
        /*
         * Add all blocks to the pool free list
         */
        for (n = memPools[i].entries; n != 0; --n) {
            MemBlock* block = (MemBlock*)heapPtr;
            block->next = memPools[i].freeList;
            memPools[i].freeList = block;
            heapPtr += memPools[i].size;
#ifndef NDEBUG
            totalSz += memPools[i].size;
            assert(totalSz <= sizeof(heap));
#endif
        }
        /*
         * Save end of pool pointer for use by AJ_Free
         */
        memPools[i].endOfPool = (void*)heapPtr;
    }
}

void* AJ_Malloc(size_t sz)
{
    size_t i;

    /*
     * One time initialization
     */
    if (!memPools[0].endOfPool) {
        InitPools();
    }
    /*
     * Find smallest pool that can satisfy the allocation
     */
    for (i = 0; i < ArraySize(memPools); ++i) {
        if ((sz <= memPools[i].size) && memPools[i].freeList) {
            MemBlock* block = memPools[i].freeList;
            //printf("AJ_Malloc pool %d allocated %d\n", memPools[i].size, sz);
            memPools[i].freeList = block->next;
            return (void*)block;
        }
    }
#ifndef NDEBUG
    printf("AJ_Malloc of %d bytes failed\n", sz);
    for (i = 0; i < ArraySize(memPools); ++i) {
        printf("    Pool %d %s\n", memPools[i].size, memPools[i].freeList ? "available" : "depleted");
    }
#endif
    return NULL;
}

void AJ_Free(void* mem)
{
    size_t i;

    if (mem) {
        assert((ptrdiff_t)mem >= (ptrdiff_t)heap);
        /*
         * Locate the pool from which the released memory was allocated
         */
        for (i = 0; i < ArraySize(memPools); ++i) {
            if ((ptrdiff_t)mem < (ptrdiff_t)memPools[i].endOfPool) {
                MemBlock* block = (MemBlock*)mem;
                block->next = memPools[i].freeList;
                memPools[i].freeList = block;
                //printf("AJ_Free pool %d\n", memPools[i].size);
                break;
            }
        }
        assert(i < ArraySize(memPools));
    }
}
