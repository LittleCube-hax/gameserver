#include <main.h>
#include <outgoing.h>
#include <lobby.h>

#include <incoming.h>

void cmd_recv_hello(Client* this)
{
	CLIENT_WRITE(this,
	{
		if (this->hello)
		{
			UNWRITE(this->lock);
			client_disconnect(this, "already sent hello");
			return;
		}
		
		u64 now = get_elapsed_ns();
		
		u32 magic;
		CLIENT_RECV_BLOCK(this, &magic, u32, 1000);
		
		u64 timestamp;
		CLIENT_RECV_BLOCK(this, &timestamp, u64, 1000);
		
		this->ping = (int) (now - timestamp);
		
		const u64 ten_seconds_ns = 10*1000*1000*1000ULL;
		
		if (magic != 34769420)
		{
			UNWRITE(this->lock);
			client_disconnect(this, "bad magic");
			return;
		}
		
		if (!(timestamp > now - ten_seconds_ns && timestamp < now + ten_seconds_ns))
		{
			UNWRITE(this->lock);
			client_disconnect(this, "bad timestamp");
			return;
		}
		
		this->hello = true;
	});
	
	cmd_send_welcome(this);
}

void cmd_recv_pong(Client* this)
{
	this->last_pong = get_elapsed_ns();
	printf("pong from client %d\n", this->fd);
}

void cmd_recv_create_lobby(Client* this)
{
	if (this->current_lobby != NULL && this->current_lobby != &welcome_lobby)
	{
		cmd_send_create_lobby_status(this, CREATE_LOBBY_FAILURE_IN_LOBBY);
	}
	
	LOCK_WRITE(lobbies_lock,
	{
		for (LobbyNode* n = lobbies.head; n != NULL; n = n->node.next)
		{
			Lobby* l = &n->lobby;
			
			if (l->leader == this)
			{
				cmd_send_create_lobby_status(this, CREATE_LOBBY_FAILURE_OWNS_LOBBY);
				UNWRITE(lobbies_lock);
				return;
			}
		}
		
		list_bump_back(&lobbies);
		
		LobbyNode* n = lobbies.tail;
		Lobby* lobby = &n->lobby;
		lobby_init(lobby, 16);
		
		lobby_join(lobby, this);
		lobby_set_leader(lobby, this);
		
		cmd_send_create_lobby_status(this, CREATE_LOBBY_SUCCESS);
		
		thread_start(lobby_func, &lobby->thread, n);
	});
}

void cmd_recv_list_lobbies(Client* this)
{
	SwapVector lobby_ids;
	SVEC_SIZED_INIT(&lobby_ids, sizeof(u16));
	
	LOCK_READ(lobbies_lock,
	{
		for (LobbyNode* n = lobbies.head; n != NULL; n = n->node.next)
		{
			Lobby* l = &n->lobby;
			
			SVEC_BUMP(&lobby_ids);
			*SVEC_GET_TOP(&lobby_ids, u16) = l->id;
		}
	});
	
	cmd_send_list_lobbies(this, lobby_ids.length, (u16*) lobby_ids.arena);
	
	SVEC_RELEASE(&lobby_ids);
}