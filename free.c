#include "malloc.h"

/* --- Global Variable Definitions --- */
free_block_t *g_free_list_head = NULL; /* Head of the explicit free list */
void *g_heap_start = NULL;             /* Start of the managed heap area */
size_t g_page_size = 0;                /* System page size, cached */

/**
 * initialize_malloc_state - Initializes global state for the allocator.
 * Caches page size. g_heap_start and g_free_list_head are initialized
 * globally and set up by initial heap extension.
 */
void initialize_malloc_state(void)
{
	if (g_page_size == 0)
	{
		g_page_size = (size_t)getpagesize();
	}
}

/**
 * add_to_free_list - Adds a block to the explicit free list.
 * @block_to_add: Pointer to the free block (as free_block_t) to add.
 *
 * Description: Inserts the block at the head of the free list (LIFO).
 * Sets the block's header to indicate it's free.
 */
void add_to_free_list(free_block_t *block_to_add)
{
	if (!block_to_add)
		return;

	SET_FREE(&(block_to_add->header)); /* Mark block as free */

	block_to_add->prev_free = NULL;
	block_to_add->next_free = g_free_list_head;

	if (g_free_list_head)
		g_free_list_head->prev_free = block_to_add;

	g_free_list_head = block_to_add;
}

/**
 * remove_from_free_list - Removes a block from the explicit free list.
 * @block_to_remove: Pointer to the free block to remove.
 */
void remove_from_free_list(free_block_t *block_to_remove)
{
	if (!block_to_remove)
		return;

	if (block_to_remove->prev_free)
		block_to_remove->prev_free->next_free = block_to_remove->next_free;
	else /* It was the head of the list */
		g_free_list_head = block_to_remove->next_free;

	if (block_to_remove->next_free)
		block_to_remove->next_free->prev_free = block_to_remove->prev_free;

	block_to_remove->next_free = NULL;
	block_to_remove->prev_free = NULL;
}

/**
 * _free - Frees a previously allocated memory block and coalesces if possible.
 * @ptr: Pointer to the memory block to free.
 * If ptr is NULL, no action is performed.
 */
void _free(void *ptr)
{
	block_header_t *current_header;
	block_header_t *next_header;
	size_t current_block_total_size;
	void *heap_current_break;

	if (ptr == NULL)
		return;

	initialize_malloc_state(); /* Uses shared helper defined in this file */

	current_header = PAYLOAD_TO_BLOCK(ptr);

	/* Basic sanity check: ensure the block was allocated by us (optional) */
	/* if (!IS_ALLOCATED(current_header)) { handle error or return; } */

	current_block_total_size = GET_BLOCK_SIZE(current_header);
	SET_FREE(current_header); /* Mark current block as free */

	/* Attempt to coalesce with the next physical block */
	next_header = (block_header_t *)((char *)current_header +
		current_block_total_size);

	heap_current_break = sbrk(0); /* Get current program break */

	/* Check if next_header is within heap bounds and is currently free */
	if ((void *)next_header < heap_current_break &&
	    !IS_ALLOCATED(next_header))
	{
		remove_from_free_list((free_block_t *)next_header); /* Helper in this file */
		current_header->size += GET_BLOCK_SIZE(next_header);
		/* current_header is already marked as free, its size is now larger */
	}

	add_to_free_list((free_block_t *)current_header); /* Helper in this file */
}
