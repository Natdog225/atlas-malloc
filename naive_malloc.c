#include "malloc.h"

/* Static global variables to manage the naive heap */
static char *g_naive_heap_current_ptr; /* Points to the next free spot */
static size_t g_naive_remaining_in_chunk; /* Remaining bytes in the current sbrk'd chunk */
static size_t g_naive_page_size; /* Cached system page size */

/* Define an alignment for naive_malloc's internal pointer stepping. */
#define NAIVE_MALLOC_INTERNAL_ALIGNMENT sizeof(size_t)

/**
 * naive_malloc - A naive memory allocator that manages a simple heap.
 * @size: The number of bytes to allocate for the user.
 *
 * Description: This function allocates 'size' bytes of memory.
 * It attempts to serve allocations from a previously sbrk'd memory chunk.
 * If the current chunk is NULL or too small, it requests a new chunk
 *
 * Return: On success, a pointer to the allocated memory.
 * On failure (e.g., if size is 0 or sbrk fails), returns NULL.
 */
void *naive_malloc(size_t size)
{
	void *current_block_start_addr;
	void *user_data_ptr;
	size_t actual_header_size;
	size_t space_for_header_and_data;
	size_t aligned_block_consumption; /* Total space this block will occupy */
	size_t *header_ptr;

	if (size == 0)
		return (NULL);

	/* Initialize page_size on first call */
	if (g_naive_page_size == 0)
		g_naive_page_size = (size_t)getpagesize();

	actual_header_size = sizeof(size_t);
	space_for_header_and_data = actual_header_size + size;

	/* Round up the space consumed by this block to ensure next block is aligned */
	aligned_block_consumption = (space_for_header_and_data +
		NAIVE_MALLOC_INTERNAL_ALIGNMENT - 1) &
		~(NAIVE_MALLOC_INTERNAL_ALIGNMENT - 1);

	/* Check if we need to request more memory from the OS */
	if (g_naive_heap_current_ptr == NULL ||
	    aligned_block_consumption > g_naive_remaining_in_chunk)
	{
		size_t num_pages_to_request;
		size_t sbrk_alloc_amount;
		void *new_chunk_start;

		/* Determine how many pages to request */
		/* Request at least enough for current block, or at least one page */
		if (aligned_block_consumption > g_naive_page_size)
			num_pages_to_request = (aligned_block_consumption +
				g_naive_page_size - 1) / g_naive_page_size;
		else
			num_pages_to_request = 1; /* Default to one page */

		sbrk_alloc_amount = num_pages_to_request * g_naive_page_size;

		/* Ensure sbrk_alloc_amount is at least aligned_block_consumption */
		if (sbrk_alloc_amount < aligned_block_consumption)
			sbrk_alloc_amount = ((aligned_block_consumption + g_naive_page_size -1) / g_naive_page_size) * g_naive_page_size;


		new_chunk_start = sbrk(sbrk_alloc_amount);
		if (new_chunk_start == (void *)-1) /* sbrk failed */
			return (NULL);

		g_naive_heap_current_ptr = (char *)new_chunk_start;
		g_naive_remaining_in_chunk = sbrk_alloc_amount;
	}

	/* Allocate from the current chunk */
	current_block_start_addr = (void *)g_naive_heap_current_ptr;

	/* Store the original requested size in the header */
	header_ptr = (size_t *)current_block_start_addr;
	*header_ptr = size;

	/* User data pointer starts immediately after the header */
	user_data_ptr = (char *)current_block_start_addr + actual_header_size;

	/* Update global heap trackers */
	g_naive_heap_current_ptr += aligned_block_consumption;
	g_naive_remaining_in_chunk -= aligned_block_consumption;

	return (user_data_ptr);
}
