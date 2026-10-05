#include <fcntl.h>

#include <common.h>
#include <client.h>
#include <incoming.h>
#include <outgoing.h>
#include <utils.h>

int client_recv(Client* this, void* out, size_t size)
{
	return recv(this->fd, out, size, 0);
}

int client_send(Client* this, void* in, size_t size)
{
	return send(this->fd, in, size, 0);
}

void client_init(Client* this)
{
	this->disconnected = false;
	this->hello = false;
	
	int flags = fcntl(this->fd, F_GETFL, 0);
	fcntl(this->fd, F_SETFL, flags | O_NONBLOCK);
}

void client_handle(Client* this)
{
	u16 cmd;
	int read = CLIENT_READ(this, &cmd, u16);
	
	if (read == 0)
	{
		client_disconnect(this, "client connection ended");
		return;
	}
	
	else if (errno != EAGAIN && errno != EWOULDBLOCK && read < 0)
	{
		client_disconnect(this, "client connection broke with an error");
		return;
	}
	
	else if (read < 0)
	{
		return;
	}
	
	switch (cmd)
	{
		case COMMAND_HELLO: cmd_recv_hello(this); break;
	}
}

void client_disconnect(Client* this, const char* reason)
{
	fprintf(stderr, "Disconnecting client %d, %s\n", this->fd, reason);
	this->disconnected = true;
}