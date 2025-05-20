#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* --- Constants --- */

/**
 * ALIGNMENT - Memory alignment requirement
 * Pointers returned by malloc must be aligned to this boundary.
 */
#define ALIGNMENT (2 * sizeof(void *))

/* --- Data Structures --- */

/**
 * struct block_header - Metadata for each memory block.
 * @size: Total size of the block (header + data).
 * The least significant bit (LSB) is used as a flag:
 * - 1 if the block is allocated.
 * - 0 if the block is free.
 */
typedef struct block_header
{
	size_t size;
} block_header_t;

/**
 * struct free_block - Structure for a free memory block.
 * @header: The common block header (size and allocation status).
 * @next_free: Pointer to the next free block in the explicit free list.
 * @prev_free: Pointer to the previous free block in the explicit free list.
 */
typedef struct free_block
{
	block_header_t header;
	struct free_block *next_free;
	struct free_block *prev_free;
} free_block_t;


/* --- Function Prototypes --- */

/* Task 0: Naive Malloc */
void *naive_malloc(size_t size);

/* Task 1: Custom Malloc */
void *_malloc(size_t size);

/* Task 2: Custom Free */
void _free(void *ptr);

#endif /* MALLOC_H */
