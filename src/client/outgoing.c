#include <outgoing.h>

void cmd_send_welcome(Client* this)
{
	u16 welcome = COMMAND_WELCOME;
	CLIENT_WRITE(this, &welcome, u16);
}