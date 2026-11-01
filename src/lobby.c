#include <common.h>
#include <main.h>
#include <client.h>
#include <outgoing.h>
#include <heap.h>

#include <lobby.h>

void lobby_init(Lobby* this, size_t max_players)
{
	rwlock_init(&this->lock);
	
	this->stop = false;
	
	SVEC_INIT(&this->queued_joins);
	SVEC_INIT(&this->queued_disconnects);
	list_init(&this->players, sizeof(ClientPtrNode));
	
	this->leader = NULL;
	this->max_players = max_players;
}

void lobby_set_leader(Lobby* this, Client* leader)
{
	LOBBY_WRITE(this,
	{
		this->leader = leader;
	});
}

bool lobby_join(Lobby* this, Client* player)
{
	Lobby* last_lobby = NULL;
	
	CLIENT_READ(player,
	{
		if (player->current_lobby != NULL)
		{
			last_lobby = player->current_lobby;
		}
	});
	
	LOBBY_WRITE(this,
	{
		if (this->players.length == this->max_players)
		{
			UNWRITE(this->lock);
			return false;
		}
		
		if (last_lobby != NULL)
		{
			list_remove(&last_lobby->players, player->lobby_node);
		}
		
		SVEC_PUSH(&this->queued_joins, player);
	});
	
	CLIENT_WRITE(player,
	{
		fprintf(stderr, "setting %p's lobby to %p\n", player, this);
		player->current_lobby = this;
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
			fprintf(stderr, "player %p is joining a lobby\n", player);
			
			list_bump_back(&this->players);
			ClientPtrNode* n = this->players.tail;
			n->client = player;
			player->lobby_node = n;
		}
		
		SVEC_CLEAR(&this->queued_joins);
		SVEC_CLEAR(&this->queued_disconnects);
		
		for (ClientPtrNode* n = this->players.head; n != NULL; n = n->node.next)
		{
			Client* player = n->client;
			fprintf(stderr, "handling player %p (from %p)\n", player, this);
			
			if (player->disconnected)
			{
				fprintf(stderr, "pushing client %p\n", player);
				SVEC_PUSH(&this->queued_disconnects, n);
				
				continue;
			}
			
			client_handle(player);
		}
		
		for (size_t i = 0; i < this->queued_disconnects.length; ++i)
		{
			ClientPtrNode* n = (ClientPtrNode*) this->queued_disconnects.data[i];
			Client* player = n->client;
			
			fprintf(stderr, "dcing %p (from %p)\n", player, this);
			
			if (this->leader == player)
			{
				fprintf(stderr, "leader: %p\n", player);
			}
			
			if (this->leader == player)
			{
				LOGE("dc player owns lobby\n");
				
				if (this->players.length == 1)
				{
					lobby_stop(this);
				}
				
				else
				{
					this->leader = ((ClientPtrNode*) this->players.head)->client;
				}
			}
			
			list_remove(&this->players, n);
			
			fprintf(stderr, "setting %p's lobby to NULL\n", player);
			
			CLIENT_WRITE(player,
			{
				player->current_lobby = NULL;
			});
			
			if (this != &welcome_lobby)
			{
				LOGE("rejoin welcome\n");
				client_reconnect(player);
				lobby_join(&welcome_lobby, player);
			}
			
			else
			{
				client_destroy(player);
			}
		}
	});
}

void lobby_leave(Lobby* this, Client* player)
{
	ClientPtrNode* n = player->lobby_node;
	
	LOBBY_WRITE(this,
	{
		list_remove(&this->players, n);
	});
	
	CLIENT_WRITE(player,
	{
		fprintf(stderr, "setting %p's lobby to NULL\n", player);
		player->current_lobby = NULL;
	});
}

void lobby_stop(Lobby* this)
{
	LOBBY_WRITE(this,
	{
		this->stop = true;
	});
}

void lobby_destroy(Lobby* this)
{
	SVEC_RELEASE(&this->queued_joins);
	SVEC_RELEASE(&this->queued_disconnects);
	
	// TODO: release list
}