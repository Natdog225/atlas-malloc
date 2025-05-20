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
typedef struct block_header {
	size_t size;
} block_header_t;

/**
 * struct free_block - Structure for a free memory block.
 * @header: The common block header (size and allocation status).
 * @next_free: Pointer to the next free block in the explicit free list.
 * @prev_free: Pointer to the previous free block in the explicit free list.
 */
typedef struct free_block {
	block_header_t header;
	struct free_block *next_free;
	struct free_block *prev_free;
} free_block_t;

/* --- Macros for Memory Layout and Manipulation --- */

#define ALIGN_SIZE(s) (((s) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define HEADER_ACTUAL_SIZE (ALIGN_SIZE(sizeof(block_header_t)))
#define MIN_FREE_BLOCK_PAYLOAD_SIZE (ALIGN_SIZE(sizeof(free_block_t)) - HEADER_ACTUAL_SIZE)
#define MIN_FREE_BLOCK_TOTAL_SIZE (ALIGN_SIZE(sizeof(free_block_t)))

#define BLOCK_TO_PAYLOAD(bp) ((void *)((char *)(bp) + HEADER_ACTUAL_SIZE))
#define PAYLOAD_TO_BLOCK(ptr) ((block_header_t *)((char *)(ptr) - HEADER_ACTUAL_SIZE))

#define GET_BLOCK_SIZE(bp) ((bp)->size & ~1UL)
#define IS_ALLOCATED(bp) ((bp)->size & 1UL)
#define SET_ALLOCATED(bp) ((bp)->size = (GET_BLOCK_SIZE(bp) | 1UL))
#define SET_FREE(bp) ((bp)->size = (GET_BLOCK_SIZE(bp) & ~1UL))
#define PACK(s, alloc) ((s) | (alloc))


/* --- Global Variable Declarations --- */
extern free_block_t *g_free_list_head;
extern void *g_heap_start;
extern size_t g_page_size;


/* --- Public Function Prototypes --- */

/* Core allocation functions */
void *naive_malloc(size_t size);
void *_malloc(size_t size);
void _free(void *ptr);

/* Shared helper functions */
void initialize_malloc_state(void);
void add_to_free_list(free_block_t *block_to_add);
void remove_from_free_list(free_block_t *block_to_remove);


#endif /* MALLOC_H */
