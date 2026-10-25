#pragma once

#include <client.h>
#include <swap_vector.h>
#include <list.h>

#define LOBBY_READ(this, code) LOCK_READ(this->lock, code)
#define LOBBY_WRITE(this, code) LOCK_WRITE(this->lock, code)

struct ClientPtrNode
{
	Node node;
	Client* client;
};

typedef struct ClientPtrNode ClientPtrNode;

struct Lobby
{
	server_rwlock_t lock;
	
	SwapVector queued_joins;
	SwapVector queued_disconnects;
	SwapVector temp_players;
	List players;
	Client* leader;
	size_t max_players;
};

typedef struct Lobby Lobby;

typedef struct
{
	Node node;
	Lobby lobby;
} LobbyNode;

typedef enum
{
	CREATE_LOBBY_SUCCESS = 1,
	CREATE_LOBBY_FAILURE_IN_LOBBY,
	CREATE_LOBBY_FAILURE_OWNS_LOBBY,
} CreateLobbyStatus;

typedef struct Client Client;

void lobby_init(Lobby* this, size_t max_players);
void lobby_set_leader(Lobby* this, Client* leader);
bool lobby_join(Lobby* this, Client* player);
void lobby_update(Lobby* this);
void lobby_leave(Lobby* this, Client* player);