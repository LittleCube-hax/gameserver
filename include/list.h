#pragma once

#include <stdint.h>

#include <client.h>

typedef struct Node Node;

struct Node
{
	Client client;
	int v;
	Node* next;
	Node* prev;
};

typedef struct
{
	Node* head;
	Node* tail;
	
	size_t length;
} List;

void list_init(List* l);
void list_bump_front(List* l);
void list_bump_back(List* l);
void list_pop_front(List* l);
void list_remove(List* l, Node* n);