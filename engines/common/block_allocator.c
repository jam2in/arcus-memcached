/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * arcus-memcached - Arcus memory cache server
 * Copyright 2017 JaM2in Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <memcached/block_allocator.h>

static mem_block_t *pool_head = NULL;
static mem_block_t *pool_tail = NULL;
static uint32_t initial_blocks;
static uint32_t total_blocks;
static uint32_t free_blocks;

static void do_block_allocator_free_all();

//static pthread_mutex_t pool_mutex;

static void do_block_allocator_free_all() {
    mem_block_t *helper;
    while (pool_head != NULL) {
        helper = pool_head;
        pool_head = pool_head->next;
        free(helper);
    }
}

int block_allocator_init(size_t nblocks) {
    mem_block_t *helper = NULL;
    int i;

    for (i = 0; i < nblocks; i++) {
        helper = (mem_block_t *)malloc(sizeof(mem_block_t));
        if (helper == NULL) break;

        helper->next = NULL;
        if (pool_tail) pool_tail->next = helper;
        else           pool_head = helper;
        pool_tail = helper;
    }
    if (i < nblocks) { /* incompleted state */
        do_block_allocator_free_all();
        return -1;
    }

    //pthread_mutex_init(&pool_mutex, NULL);
    initial_blocks = nblocks;
    total_blocks= nblocks;
    free_blocks = nblocks;

    return 0;
}

void block_allocator_destroy() {
    //pthread_mutex_lock(&pool_mutex);
    do_block_allocator_free_all();

    initial_blocks = 0;
    total_blocks = 0;
    free_blocks = 0;
    //pthread_mutex_unlock(&pool_mutex);
    //pthread_mutex_destroy(&pool_mutex);
}

void *allocate_single_block() {
    mem_block_t *ret;

    //pthread_mutex_lock(&pool_mutex);

    if ((ret = pool_head) != NULL) {
        pool_head = pool_head->next;
        if (pool_head == NULL)
            pool_tail= NULL;
        ret->next = NULL;
        free_blocks--;
    } else {
        // TODO :
        // This malloc() inside mutex may raise some performance issue,
        // Is there any way to execute malloc and counter adjustment
        // outside the mutex lock?
        ret = (mem_block_t *)malloc(sizeof(mem_block_t));
        if (ret != NULL) {
            total_blocks++;
            ret->next = NULL;
        }
    }

    //pthread_mutex_unlock(&pool_mutex);

    return (void *)ret;
}

uint32_t free_block_list(void *start, const int numblk) {
    //mem_block_t *bye = NULL;
    //mem_block_t *bye_helper = NULL;
    int i;
    int freed_blocks = 0;

    if (start == NULL || numblk == 0) {
        return 0;
    }

    //pthread_mutex_lock(&pool_mutex);

    assert(pool_tail == NULL || pool_tail->next == NULL);

    mem_block_t *helper = (mem_block_t*)start;
    freed_blocks++;

    if (numblk < 0) {
        while (helper->next != NULL) {
            helper = helper->next;
            freed_blocks++;
        }
    } else {
        for (i = 1; i < numblk && helper->next != NULL; i++) {
            helper = helper->next;
            freed_blocks++;
        }
    }

    if (free_blocks == 0) { /* first_block */
        pool_head = (mem_block_t *)start;
        helper->next = NULL;
        pool_tail = helper;
    } else {
        helper->next = NULL;
        pool_tail->next = (mem_block_t *)start;
        pool_tail = helper;
    }

    free_blocks += freed_blocks;

    // TODO : implement intelligent resize logic
    /*
    if (total_blocks > initial_blocks
            && free_blocks > initial_blocks / 2) {
        bye = pool_head;
        while(total_blocks > initial_blocks) {
            bye_helper = pool_head;
            pool_head = pool_head->next;
            free_blocks--;
            total_blocks--;
        }
        bye_helper->next = NULL;
    }*/

    //pthread_mutex_unlock(&pool_mutex);

    // rest of resize logic
    /*while (bye != NULL) {
        bye_helper = bye;
        bye = bye->next;
        free(bye_helper);
    }*/

    return freed_blocks;
}
