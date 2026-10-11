#include <common.h>
#include <client.h>
#include <outgoing.h>
#include <heap.h>

#include <lobby.h>

void lobby_init(Lobby* this, size_t max_players)
{
	rwlock_init(&this->lock);
	
	list_init(&this->players, sizeof(ClientPtrNode));
	
	this->leader = NULL;
	this->num_players = 0;
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
		if (this->num_players == this->max_players)
		{
			UNWRITE(this->lock);
			return false;
		}
		
		list_bump_back(&this->players);
		ClientPtrNode* n = this->players.tail;
		n->client = player;
		
		this->num_players += 1;
	});
	
	return true;
}

void lobby_update(Lobby* this)
{
	
}

void lobby_leave(Lobby* this, Client* player)
{
	LOBBY_WRITE(this,
	{
		this->num_players -= 1;
	});
	
	CLIENT_WRITE(player,
	{
		player->current_lobby = NULL;
	});
}