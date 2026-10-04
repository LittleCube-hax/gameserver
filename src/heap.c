#include <o1heap.h>
#include <string.h>

#include <heap.h>
#include <utils.h>

O1HeapInstance* heap_instance;
char* heap;
size_t heap_size;
server_rwlock_t heap_lock;

void heap_init(size_t size)
{
	char* h = vmem_reserve(size);
	heap = h;
	heap_size = size;
	heap_instance = o1heapInit(h, size);
	rwlock_init(&heap_lock);
}

void* heap_alloc(size_t size)
{
	void* ret;
	
	LOCK_WRITE(heap_lock,
	{
		ret = o1heapAllocate(heap_instance, size);
	});
	
	if (UNLIKELY(ret == NULL))
	{
		UNREACHABLE("Error allocating memory");
	}
	
	return ret;
}

void* heap_realloc(void* ptr, size_t size)
{
	void* ret;
	
	LOCK_WRITE(heap_lock,
	{
		ret = o1heapReallocate(heap_instance, ptr, size);
	});
	
	if (UNLIKELY(ret == NULL))
	{
		UNREACHABLE("Out of memory, quitting");
	}
	
	return ret;
}

void heap_free(void* ptr)
{
	LOCK_WRITE(heap_lock,
	{
		o1heapFree(heap_instance, ptr);
	});
}

void heap_shutdown()
{
	vmem_release(heap, heap_size);
}