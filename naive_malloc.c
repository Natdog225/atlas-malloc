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
	size_t effective_header_size;
	size_t total_needed_payload;
	size_t sbrk_alloc_size;
	size_t num_pages;
	size_t *header_ptr;

	if (size == 0)
		return (NULL);

	if (page_size == 0) /* Initialize page_size on first call */
		page_size = (size_t)getpagesize();

	effective_header_size = (sizeof(size_t) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

	total_needed_payload = effective_header_size + size;

	/* Calculate sbrk allocation size (must be multiple of page_size) */
	num_pages = (total_needed_payload + page_size - 1) / page_size;
	sbrk_alloc_size = num_pages * page_size;

	/* Request memory from the OS */
	block_start = sbrk(sbrk_alloc_size);
	if (block_start == (void *)-1) /* sbrk failed */
		return (NULL);

	/*
	 * Store the original requested size in the header.
	 */
	header_ptr = (size_t *)block_start;
	*header_ptr = size;

	/*
	 * The data pointer starts after the effective header space.
	 */
	user_data_ptr = (char *)block_start + effective_header_size;

	return (user_data_ptr);
}
