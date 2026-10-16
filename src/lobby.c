#include <common.h>
#include <main.h>
#include <client.h>
#include <outgoing.h>
#include <heap.h>

#include <lobby.h>

void lobby_init(Lobby* this, size_t max_players)
{
	rwlock_init(&this->lock);
	
	SVEC_INIT(&this->queued_joins);
	list_init(&this->players, sizeof(ClientPtrNode));
	
	this->leader = NULL;
	this->max_players = max_players;
}

void lobby_set_leader(Lobby* this, Client* leader)
{
	LOCK_WRITE(this->lock,
	{
		this->leader = leader;
	});
}

bool lobby_join(Lobby* this, Client* player)
{
	LOCK_WRITE(player->lock,
	{
		if (player->current_lobby != NULL)
		{
			UNWRITE(player->lock);
			return false;
		}
		
		player->current_lobby = this;
	});
	
	LOCK_WRITE(this->lock,
	{
		if (this->players.length == this->max_players)
		{
			UNWRITE(this->lock);
			return false;
		}
		
		SVEC_PUSH(&this->queued_joins, player);
	});
	
	return true;
}

void lobby_update(Lobby* this)
{
	LOBBY_WRITE(this,
	{
		for (size_t i = 0; i < this->queued_joins.length; ++i)
		{
			Client* player = (Client*) this->queued_joins.data[i];
			
			list_bump_back(&this->players);
			ClientPtrNode* n = this->players.tail;
			n->client = player;
		}
	});
	
	for (ClientPtrNode* n = this->players.head; n != NULL; n = n->node.next)
	{
		Client* player = n->client;
		
		client_handle(player);
		
		if (player->disconnected)
		{
			LOBBY_WRITE(this,
			{
				list_remove(&this->players, n);
			});
			
			if (this != &welcome_lobby)
			{
				lobby_join(&welcome_lobby, player);
			}
			
			else
			{
				client_destroy(player);
			}
		}
	}
}

void lobby_leave(Lobby* this, ClientPtrNode* client)
{
	LOBBY_WRITE(this,
	{
		list_remove(&this->players, client);
	});
	
	Client* player = client->client;
	
	CLIENT_WRITE(player,
	{
		player->current_lobby = NULL;
	});
}