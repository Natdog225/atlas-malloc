#include "malloc.h"

/* --- Global Variables --- */
static free_block_t *g_free_list_head; /* Head of the explicit free list */
static void *g_heap_start;             /* Start of the managed heap area */
static size_t g_page_size;             /* System page size, cached */

/* --- Alignment and Size Macros/Utilities --- */

/**
 * ALIGN_SIZE - Rounds up a size to the nearest multiple of ALIGNMENT.
 * @size: The size to round up.
 *
 * Return: The size rounded up to ALIGNMENT.
 */
#define ALIGN_SIZE(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

/* Size of the basic block header, aligned */
#define HEADER_ACTUAL_SIZE (ALIGN_SIZE(sizeof(block_header_t)))

/* Minimum size of any block when it's free (to hold free list pointers) */
#define MIN_FREE_BLOCK_PAYLOAD_SIZE \
(ALIGN_SIZE(sizeof(free_block_t)) - HEADER_ACTUAL_SIZE)
#define MIN_FREE_BLOCK_TOTAL_SIZE (ALIGN_SIZE(sizeof(free_block_t)))


/* Get payload pointer from block pointer (bp) */
#define BLOCK_TO_PAYLOAD(bp) ((void *)((char *)(bp) + HEADER_ACTUAL_SIZE))
/* Get block pointer from payload pointer (ptr) */
#define PAYLOAD_TO_BLOCK(ptr) \
((block_header_t *)((char *)(ptr) - HEADER_ACTUAL_SIZE))

/* Macros to manipulate the size and allocation bit in the header */
#define GET_BLOCK_SIZE(bp) ((bp)->size & ~1UL)
#define IS_ALLOCATED(bp) ((bp)->size & 1UL)
/* Set block as allocated (preserve size, set LSB) */
#define SET_ALLOCATED(bp) ((bp)->size = (GET_BLOCK_SIZE(bp) | 1UL))
/* Set block as free (preserve size, clear LSB) */
#define SET_FREE(bp) ((bp)->size = (GET_BLOCK_SIZE(bp) & ~1UL))
/* Pack size and allocation status into a header value */
#define PACK(size, alloc) ((size) | (alloc))


/* --- Static Helper Function Prototypes --- */
static void initialize_malloc_state(void);
static void add_to_free_list(free_block_t *block_to_add);
static void remove_from_free_list(free_block_t *block_to_remove);
static free_block_t *extend_heap(size_t min_size);
static free_block_t *find_free_block(size_t required_total_size);


/**
 * initialize_malloc_state - Initializes global state for the allocator.
 * Caches page size and sets up initial heap pointers if not already done.
 * This function is designed to be called once.
 */
static void initialize_malloc_state(void)
{
	if (g_page_size == 0)
	{
		g_page_size = (size_t)getpagesize();
	}
	/*
	 * g_heap_start and g_free_list_head are initialized to NULL globally.
	 * They will be set up by the first call to extend_heap or _malloc.
	 */
}

/**
 * add_to_free_list - Adds a block to the explicit free list.
 * @block_to_add: Pointer to the free block (as free_block_t) to add.
 *
 * Description: Inserts the block at the head of the free list (LIFO).
 * Sets the block's header to indicate it's free.
 */
static void add_to_free_list(free_block_t *block_to_add)
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
static void remove_from_free_list(free_block_t *block_to_remove)
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
 * extend_heap - Requests more memory from the OS using sbrk.
 * @min_size: The minimum total size needed for the new block.
 *
 * Description: Calculates an allocation size (multiple of page size)
 * that is at least 'min_size'. Calls sbrk() to extend the program break.
 * Initializes the new memory as a single free block and adds it to the
 * free list.
 *
 * Return: Pointer to the new free block, or NULL on sbrk failure.
 */
static free_block_t *extend_heap(size_t min_size)
{
	size_t sbrk_req_size;
	size_t num_pages;
	void *new_mem_ptr;
	free_block_t *new_block;

	if (g_page_size == 0) /* Should be initialized by now, but as a fallback */
		initialize_malloc_state();

	/* Determine how much to request: at least min_size, page-aligned */
	num_pages = (min_size + g_page_size - 1) / g_page_size;
	sbrk_req_size = num_pages * g_page_size;

	new_mem_ptr = sbrk(sbrk_req_size);
	if (new_mem_ptr == (void *)-1)
		return (NULL); /* sbrk failed */

	if (g_heap_start == NULL) /* First time extending heap */
		g_heap_start = new_mem_ptr;

	new_block = (free_block_t *)new_mem_ptr;
	new_block->header.size = PACK(sbrk_req_size, 0); /* Size, marked free */

	/* TODO: Implement coalescing with previous block if it was the heap top */
	add_to_free_list(new_block);

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
	free_block_t *current = g_free_list_head;

	while (current)
	{
		if (GET_BLOCK_SIZE(&(current->header)) >= required_total_size)
			return (current);
		current = current->next_free;
	}
	return (NULL); /* No suitable block found */
}


/**
 * _malloc - Allocates memory from the heap. (Task 1)
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

	/* Calculate aligned payload size, ensuring it's at least min free payload */
	aligned_payload_size = ALIGN_SIZE(size);
	if (aligned_payload_size < MIN_FREE_BLOCK_PAYLOAD_SIZE)
		aligned_payload_size = MIN_FREE_BLOCK_PAYLOAD_SIZE;

	required_total_block_size = HEADER_ACTUAL_SIZE + aligned_payload_size;

	found_block = find_free_block(required_total_block_size);

	if (found_block == NULL) /* No suitable block in free list, extend heap */
	{
		found_block = extend_heap(required_total_block_size);
		if (found_block == NULL)
			return (NULL); /* Heap extension failed */
		/* Re-search: extend_heap adds to list, find_free_block gets it */
		found_block = find_free_block(required_total_block_size);
		if (found_block == NULL) /* Should not happen if extend_heap succeeded */
			return (NULL);
	}

	/* A suitable free block is found. Remove it from free list. */
	remove_from_free_list(found_block);
	alloc_block_header = &(found_block->header);
	original_block_size = GET_BLOCK_SIZE(alloc_block_header);

	/* Try to split the block if it's much larger than needed */
	remainder_size = original_block_size - required_total_block_size;

	if (remainder_size >= MIN_FREE_BLOCK_TOTAL_SIZE)
	{ /* Split the block */
		free_block_t *remainder_block;

		/* Setup the allocated portion */
		alloc_block_header->size = PACK(required_total_block_size, 1);

		/* Setup the remainder portion (new free block) */
		remainder_block = (free_block_t *)((char *)alloc_block_header +
			required_total_block_size);
		remainder_block->header.size = PACK(remainder_size, 0); /* Mark free */
		add_to_free_list(remainder_block);
	}
	else
	{ /* Use the whole block, not enough space to split meaningfully,
	* like a bad breakup */
		alloc_block_header->size = PACK(original_block_size, 1);
	}

	return (BLOCK_TO_PAYLOAD(alloc_block_header));
}

/**
 * _free - Frees a previously allocated memory block.
 * @ptr: Pointer to the memory block to free.
 * If ptr is NULL, no action is performed.
 */
void _free(void *ptr)
{
	block_header_t *block_header_to_free;
	free_block_t *cast_to_free_block;

	if (ptr == NULL)
		return;

	initialize_malloc_state(); /* Just in case, though _malloc should have run */

	block_header_to_free = PAYLOAD_TO_BLOCK(ptr);

	/* Basic sanity check: is it already marked free? */
	/* if (!IS_ALLOCATED(block_header_to_free)) return; // Or error */

	cast_to_free_block = (free_block_t *)block_header_to_free;

	/* TODO: Implement coalescing with adjacent (next/prev physical) free blocks */
	add_to_free_list(cast_to_free_block);
}
