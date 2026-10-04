#pragma once

#define HALLOC(s) heap_alloc(s)
#define HREALLOC(p, s) heap_realloc(p, s)
#define FREE(p) heap_free(p)

/**
 * Memory Heap Manager
 *
 * Wrapper around o1heap allocator providing multi-heap support.
 */

/**
 * Initialize the heap system
 *
 * @param size Heap size in bytes
 */
void heap_init(size_t size);

/**
 * Allocate memory from the heap
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* heap_alloc(size_t size);

/**
 * Allocate memory from the heap, copying memory from an old ptr and freeing it
 *
 * @param ptr Old ptr to copy and free
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* heap_realloc(void* ptr, size_t size);

/**
 * Free memory allocated by heap_alloc() or heap_calloc()
 *
 * Pointer must have been returned by heap_alloc()
 *
 * @param ptr Pointer to memory to free
 */
void heap_free(void* ptr);

/**
 * Shutdown the heap system
 *
 * Frees all heap arenas. Should be called at program exit.
 *
 */
void heap_shutdown();