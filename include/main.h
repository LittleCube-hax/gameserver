#pragma once

#include <lobby.h>
#include <list.h>
#include <utils.h>

typedef struct
{
	Node node;
	Lobby lobby;
} LobbyNode;

typedef struct
{
	Node node;
	Client client;
} ClientNode;

extern List lobbies;
extern server_rwlock_t lobbies_lock;