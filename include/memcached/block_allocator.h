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
#ifndef MEMCACHED_ENGINE_BLOCK_ALLOCATOR_H
#define MEMCACHED_ENGINE_BLOCK_ALLOCATOR_H

#include "memcached/types.h"

#define EITEMS_PER_BLOCK 1023
#define BLOCK_ALLOCATOR_DEFAULT_SIZE 500

typedef struct _mem_block_t {
  eitem* items[EITEMS_PER_BLOCK];
  struct _mem_block_t *next;
} mem_block_t;

typedef struct _block_result_t {
  struct _mem_block_t *root_blk; /* block at the start point */
  struct _mem_block_t *curr_blk; /* current access block */
  uint32_t totelem;              /* total number of elements in the block */
  uint32_t readelem;             /* current access elements number */
} block_result_t;

/* block allocator API */
int block_allocator_init(size_t);
void block_allocator_destroy(void);
void *allocate_single_block(void);
uint32_t free_block_list(void *start, const int numblk);

#endif