#include "malloc.h"

/* --- Static Helper Function Prototypes --- */
static free_block_t *extend_heap(size_t min_size);
static free_block_t *find_free_block(size_t required_total_size);

/**
 * extend_heap - Requests more memory from the OS using sbrk.
 * @min_size: The minimum total size needed for the new block.
 *
 * Description: Calculates an allocation size (multiple of page size)
 * that is at least 'min_size'. Calls sbrk() to extend the program break.
 * Initializes the new memory as a single free block and adds it to the
 * free list (via add_to_free_list, defined in free.c).
 *
 * Return: Pointer to the new free block, or NULL on sbrk failure.
 */
static free_block_t *extend_heap(size_t min_size)
{
	size_t sbrk_req_size;
	size_t num_pages;
	void *new_mem_ptr;
	free_block_t *new_block;

	/* g_page_size is global, initialized by initialize_malloc_state() */
	if (g_page_size == 0)
		initialize_malloc_state(); /* Call shared helper if not initialized */

	num_pages = (min_size + g_page_size - 1) / g_page_size;
	sbrk_req_size = num_pages * g_page_size;

	new_mem_ptr = sbrk(sbrk_req_size);
	if (new_mem_ptr == (void *)-1)
		return (NULL); /* sbrk failed */

	if (g_heap_start == NULL) /* g_heap_start is global */
		g_heap_start = new_mem_ptr;

	new_block = (free_block_t *)new_mem_ptr;
	new_block->header.size = PACK(sbrk_req_size, 0); /* Size, marked free */

	/* TODO: Coalesce with previous block if it was the heap top. */
	add_to_free_list(new_block); /* Uses shared helper from free.c */

	return (new_block);
}

/**
 * find_free_block - Searches the free list for a suitable block.
 * @required_total_size: The minimum total block size needed.
 *
 * Description: Implements a first-fit search strategy on the free list.
 *
 * Return: Pointer to a suitable free block, or NULL if none found.
 */
static free_block_t *find_free_block(size_t required_total_size)
{
	free_block_t *current = g_free_list_head; /* Uses global g_free_list_head */

	while (current)
	{
		if (GET_BLOCK_SIZE(&(current->header)) >= required_total_size)
			return (current);
		current = current->next_free;
	}
	return (NULL); /* No suitable block found */
}


/**
 * _malloc - Allocates memory from the heap.
 * @size: Number of bytes to allocate for the user.
 *
 * Return: Pointer to the allocated memory, aligned to ALIGNMENT.
 * NULL if size is 0 or if allocation fails.
 */
void *_malloc(size_t size)
{
	size_t aligned_payload_size;
	size_t required_total_block_size;
	free_block_t *found_block;
	block_header_t *alloc_block_header;
	size_t original_block_size;
	size_t remainder_size;

	if (size == 0)
		return (NULL);

	initialize_malloc_state();

	aligned_payload_size = ALIGN_SIZE(size);
	if (aligned_payload_size < MIN_FREE_BLOCK_PAYLOAD_SIZE)
		aligned_payload_size = MIN_FREE_BLOCK_PAYLOAD_SIZE;

	required_total_block_size = HEADER_ACTUAL_SIZE + aligned_payload_size;

	found_block = find_free_block(required_total_block_size); /* Static helper */

	if (found_block == NULL)
	{
		found_block = extend_heap(required_total_block_size); /* Static helper */
		if (found_block == NULL)
			return (NULL);
		/* Re-search, extend_heap adds to list, find_free_block gets it */
		found_block = find_free_block(required_total_block_size);
		if (found_block == NULL) /* Should not happen if extend_heap succeeded */
			return (NULL);
	}

	remove_from_free_list(found_block);
	alloc_block_header = &(found_block->header);
	original_block_size = GET_BLOCK_SIZE(alloc_block_header);

	remainder_size = original_block_size - required_total_block_size;

	if (remainder_size >= MIN_FREE_BLOCK_TOTAL_SIZE)
	{
		free_block_t *remainder_block;

		alloc_block_header->size = PACK(required_total_block_size, 1);
		remainder_block = (free_block_t *)((char *)alloc_block_header +
			required_total_block_size);
		remainder_block->header.size = PACK(remainder_size, 0);
		add_to_free_list(remainder_block);
	}
	else
	{
		alloc_block_header->size = PACK(original_block_size, 1);
	}

	return (BLOCK_TO_PAYLOAD(alloc_block_header));
}
