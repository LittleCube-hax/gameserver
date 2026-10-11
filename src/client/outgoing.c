#include <outgoing.h>

void cmd_send_welcome(Client* this)
{
	u16 cmd = COMMAND_WELCOME;
	CLIENT_SEND(this, &cmd, u16);
}

void cmd_send_ping(Client* this)
{
	u16 cmd = COMMAND_PING;
	CLIENT_SEND(this, &cmd, u16);
}

void cmd_send_create_lobby_status(Client* this, int status)
{
	u16 cmd = COMMAND_CREATE_LOBBY_STATUS;
	CLIENT_SEND(this, &cmd, u16);
	CLIENT_SEND(this, &status, int);
}