#include <common.h>
#include <heap.h>
#include <utils.h>

#include <swap_vector.h>

#define UP(x) VAL(uintptr_t, x)

void svec_init(SwapVector* v)
{
	svec_sized_init(v, sizeof(uintptr_t));
}

void svec_sized_init(SwapVector* v, size_t struct_size)
{
	v->length = 0;
	v->length_bytes = 0;
	v->struct_size = struct_size;
	v->arena_capacity = 64*struct_size;
	v->arena = HALLOC(v->arena_capacity);
}

void svec_push(SwapVector* v, uintptr_t value)
{
	svec_bump(v);
	
	char* this = &v->arena[v->length_bytes - v->struct_size];
	UP(this) = value;
}

void svec_bump(SwapVector* v)
{
	v->length += 1;
	v->length_bytes += v->struct_size;
	ENSURE_SIZE(v->arena, v->length_bytes, v->arena_capacity, 1);
}

void svec_remove(SwapVector* v, size_t index)
{
	v->length -= 1;
	v->length_bytes -= v->struct_size;
	
	if (UNLIKELY(v->length_bytes == 0))
	{
		return;
	}
	
	char* this = &v->arena[v->length_bytes];
	v->data[index] = UP(this);
}

void svec_pop(SwapVector* v)
{
	v->length -= 1;
	v->length_bytes -= v->struct_size;
}

void svec_clear(SwapVector* v)
{
	v->length = 0;
	v->length_bytes = 0;
}

void svec_release(SwapVector* v)
{
	FREE(v->arena);
}