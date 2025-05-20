#include "malloc.h"

/**
 * naive_malloc - A naive memory allocator.
 * @size: The number of bytes to allocate.
 *
 * Description: This function allocates 'size' bytes of memory.
 * It ensures that the memory allocated from the system via sbrk()
 * is a multiple of the page size.
 *
 * Return: On success, a pointer to the allocated memory.
 * On failure (e.g., if size is 0 or sbrk fails), returns NULL.
 */
void *naive_malloc(size_t size)
{
	static size_t page_size; /* Cache page size */
	void *block_start;
	void *user_data_ptr;
	size_t actual_header_size;
	size_t total_payload_plus_header;
	size_t sbrk_alloc_size;
	size_t num_pages;
	size_t *header_ptr;

	if (size == 0)
		return (NULL);

	if (page_size == 0) /* Initialize page_size on first call */
		page_size = (size_t)getpagesize();

	/* The header is exactly sizeof(size_t) */
	actual_header_size = sizeof(size_t);

	total_payload_plus_header = actual_header_size + size;

	/*
	 * Calculate sbrk allocation size.
	 * Must be a multiple of page_size to allocate memory pages only.
	 */
	num_pages = (total_payload_plus_header + page_size - 1) / page_size;
	sbrk_alloc_size = num_pages * page_size;

	/* Request memory from the OS */
	block_start = sbrk(sbrk_alloc_size);
	if (block_start == (void *)-1) /* sbrk failed */
		return (NULL);

	/*
	 * Store the original requested size in the header.
	 */
	header_ptr = (size_t *)block_start;
	*header_ptr = size; /* Store the user's requested size */

	user_data_ptr = (char *)block_start + actual_header_size;

	return (user_data_ptr);
}
