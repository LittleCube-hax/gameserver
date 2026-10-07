#include <outgoing.h>

void cmd_send_welcome(Client* this)
{
	u16 welcome = COMMAND_WELCOME;
	CLIENT_WRITE(this, &welcome, u16);
}

void cmd_send_ping(Client* this)
{
	u16 ping = COMMAND_PING;
	CLIENT_WRITE(this, &ping, u16);
}