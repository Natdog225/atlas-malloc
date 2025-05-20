#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* --- Constants --- */

/**
 * ALIGNMENT - Memory alignment requirement
 */
#define ALIGNMENT (2 * sizeof(void *))

/* --- Function Prototypes --- */

/* Task 0: Naive Malloc */
void *naive_malloc(size_t size);

/* Task 1: Custom Malloc */
void *_malloc(size_t size);

/* Task 2: Custom Free */
void _free(void *ptr);

#endif /* MALLOC_H */
