#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <common.h>
#include <heap.h>
#include <utils.h>

#include <list.h>

#define VAL(type, x) *((type*) x)

void list_init(List* l, size_t struct_size)
{
	l->head = NULL;
	l->tail = NULL;
	
	l->length = 0;
	l->struct_size = struct_size;
}

void list_bump_front(List* l)
{
	l->length += 1;
	
	Node* n = HALLOC(l->struct_size);
	
	if (l->head == NULL)
	{
		l->head = l->tail = n;
		l->length = 1;
		n->next = n->prev = NULL;
		return;
	}
	
	n->next = l->head;
	n->prev = NULL;
	l->head = n;
}

void list_bump_back(List* l)
{
	l->length += 1;
	
	Node* n = HALLOC(l->struct_size);
	n->next = NULL;
	
	if (l->head == NULL)
	{
		l->head = l->tail = n;
		l->length = 1;
		n->prev = NULL;
		return;
	}
	
	n->prev = l->tail;
	TAIL->next = n;
	l->tail = n;
}

void list_pop_front(List* l)
{
	if (UNLIKELY(l->length == 0))
	{
		return;
	}
	
	l->length -= 1;
	Node* front = l->head;
	Node* next = front->next;
	FREE(front);
	
	if (l->length == 0)
	{
		l->head = l->tail = NULL;
	}
	
	l->head = next;
	HEAD->prev = NULL;
}

void list_remove(List* l, void* node)
{
	Node* n = (Node*) node;
	
	Node* next = n->next;
	Node* prev = n->prev;
	
	if (n == l->head)
	{
		l->head = next;
	}
	
	if (n == l->tail)
	{
		l->tail = prev;
	}
	
	if (prev != NULL)
	{
		prev->next = next;
	}
	
	if (next != NULL)
	{
		next->prev = prev;
	}
	
	FREE(n);
	
	l->length -= 1;
	
	if (l->length == 0)
	{
		l->head = l->tail = NULL;
	}
}