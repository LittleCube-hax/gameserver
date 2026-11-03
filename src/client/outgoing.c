#include <outgoing.h>

void cmd_send_welcome(Client* this)
{
	u16 cmd = COMMAND_WELCOME;
	
	CLIENT_WRITE(this,
	{
		CLIENT_SEND(this, &cmd, u16);
	});
}

void cmd_send_ping(Client* this)
{
	u16 cmd = COMMAND_PING;
	
	CLIENT_WRITE(this,
	{
		CLIENT_SEND(this, &cmd, u16);
	});
}

void cmd_send_create_lobby_status(Client* this, int status)
{
	u16 cmd = COMMAND_CREATE_LOBBY_STATUS;
	
	CLIENT_WRITE(this,
	{
		CLIENT_SEND(this, &cmd, u16);
		CLIENT_SEND(this, &status, int);
	});
}

void cmd_send_list_lobbies(Client* this, u16 num_lobbies, u16* lobby_ids)
{
	u16 cmd = COMMAND_LIST_LOBBIES;
	
	CLIENT_WRITE(this,
	{
		CLIENT_SEND(this, &cmd, u16);
		CLIENT_SEND(this, &num_lobbies, u16);
		
		client_send(this, lobby_ids, sizeof(u16)*num_lobbies);
	});
}