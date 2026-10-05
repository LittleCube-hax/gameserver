#pragma once

#include <errno.h>

#include <common.h>
#include <utils.h>

#define CLIENT_READ(this, out, type) \
	client_recv(this, out, sizeof(type))

#define CLIENT_READ_BLOCK(this, out, type, total_retries) \
	this->bytes_read = 0; \
	for (this->retries = 0; this->retries < total_retries; this->retries += 1) \
	{ \
		this->read = client_recv(this, out + this->bytes_read, sizeof(type) - this->bytes_read); \
		if ((errno != EAGAIN && errno != EWOULDBLOCK) && this->read < 0) \
		{ \
			client_disconnect(this, "read broke"); \
			return; \
		} \
		if (this->read >= 0) \
		{ \
			this->bytes_read += this->read; \
		} \
		if (this->bytes_read == sizeof(type)) \
		{ \
			break; \
		} \
		server_sleep(10); \
	} \
	if (this->retries == total_retries) \
	{ \
		client_disconnect(this, "read timeout"); \
		return; \
	}

#define CLIENT_WRITE(this, in, type) \
	client_send(this, in, sizeof(type))

typedef struct
{
	int retries;
	int read;
	int bytes_read;
	bool disconnected;
	
	int ping;
	
	int fd;
	server_socket_t sock;
	
	bool hello;
} Client;

typedef enum
{
	COMMAND_HELLO = 1,
	COMMAND_WELCOME,
} Command;

int client_recv(Client* this, void* out, size_t size);
int client_send(Client* this, void* in, size_t size);

void client_init(Client* this);
void client_handle(Client* this);
void client_disconnect(Client* this, const char* reason);