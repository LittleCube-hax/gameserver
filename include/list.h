#pragma once

#include <stdint.h>

#define HEAD ((Node*) l->head)
#define TAIL ((Node*) l->tail)

typedef struct
{
	void* next;
	void* prev;
} Node;

typedef struct
{
	void* head;
	void* tail;
	
	size_t length;
	size_t struct_size;
} List;

void list_init(List* l, size_t struct_size);
void list_bump_front(List* l);
void list_bump_back(List* l);
void list_pop_front(List* l);
void list_remove(List* l, void* n);
// TODO: list_release