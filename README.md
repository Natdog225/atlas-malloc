# Atlas Malloc

This project is an implementation of custom memory allocation functions (`malloc`, `free`, `realloc`, `calloc`) in C. The goal is to understand the intricacies of dynamic memory management by building these functions from scratch, without relying on the standard library's `malloc` family.

## Project Structure

- `malloc.h`: Header file containing prototypes for all custom allocation functions and any necessary structures or macros.
- `naive_malloc.c`: Implements a simple `naive_malloc` function as a basic starting point.
- `malloc.c`: Contains the implementations for `_malloc` and `_free`.

## Compilation

The C programs and functions are compiled on Ubuntu 20.04 LTS using `gcc 9.4.0` with the following flags:
`-Wall -Werror -Wextra -pedantic -std=gnu89`

Example compilation for a shared library
```bash
gcc -Wall -Werror -Wextra -pedantic -std=gnu89 -fPIC -shared *.c -o libmymalloc.so
