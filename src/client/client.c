#include <arpa/inet.h>
#include <fcntl.h>

#include <common.h>
#include <main.h>
#include <incoming.h>
#include <outgoing.h>
#include <list.h>
#include <utils.h>

#include <client.h>

int client_recv(Client* this, void* out, size_t size)
{
	return recv(this->fd, out, size, 0);
}

int client_send(Client* this, void* in, size_t size)
{
	return send(this->fd, in, size, 0);
}

void client_init(Client* this, ClientNode* node, server_socket_t* sock)
{
	rwlock_init(&this->lock);
	
	this->node = node;
	
	this->current_lobby = NULL;
	
	this->disconnected = false;
	this->hello = false;
	
	int flags = fcntl(this->fd, F_GETFL, 0);
	fcntl(this->fd, F_SETFL, flags | O_NONBLOCK);
	
	struct sockaddr_in* client_sock = (struct sockaddr_in*) sock;
	this->ip = client_sock->sin_addr.s_addr;
}

void client_handle(Client* this)
{
	u16 cmd;
	
	CLIENT_WRITE(this,
	{
		int read = CLIENT_RECV(this, &cmd, u16);
		
		if (read == 0)
		{
			client_disconnect(this, "client connection ended");
			return;
		}
		
		else if (errno != EAGAIN && errno != EWOULDBLOCK && read < 0)
		{
			client_disconnect(this, "client connection closed abruptly");
			return;
		}
		
		else if (read < 0)
		{
			return;
		}
	});
	
	switch (cmd)
	{
		case COMMAND_HELLO: cmd_recv_hello(this); break;
		case COMMAND_PONG: cmd_recv_pong(this); break;
		case COMMAND_CREATE_LOBBY: cmd_recv_create_lobby(this); break;
	}
}

void client_disconnect(Client* this, const char* reason)
{
	fprintf(stderr, "Disconnecting client %d, %s\n", this->fd, reason);
	this->disconnected = true;
}

void client_destroy(Client* this)
{
	LOCK_WRITE(clients_lock,
	{
		list_remove(&clients, this->node);
	});
	
	CLIENT_WRITE(this,
	{
		close(this->fd);
	});
}