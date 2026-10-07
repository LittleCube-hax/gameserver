#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <common.h>
#include <heap.h>

#include <list.h>

#define VAL(type, x) *((type*) x)

void list_init(List* l)
{
	l->head = NULL;
	l->tail = NULL;
	
	l->length = 0;
}

void list_bump_front(List* l)
{
	l->length += 1;
	
	Node* n = HALLOC(sizeof(Node));
	
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
	
	Node* n = HALLOC(sizeof(Node));
	n->next = NULL;
	
	if (l->head == NULL)
	{
		l->head = l->tail = n;
		l->length = 1;
		n->prev = NULL;
		return;
	}
	
	n->prev = l->tail;
	l->tail->next = n;
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
	l->head->prev = NULL;
}

void list_remove(List* l, Node* n)
{
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