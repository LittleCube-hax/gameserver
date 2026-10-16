#pragma once

#include <errno.h>

#include <common.h>
#include <list.h>
#include <utils.h>

#define CLIENT_READ(this, code) LOCK_READ(this->lock, code)
#define CLIENT_WRITE(this, code) LOCK_WRITE(this->lock, code)

#define CLIENT_RECV(this, out, type) \
	client_recv(this, out, sizeof(type))

#define CLIENT_RECV_BLOCK(this, out, type, total_retries) \
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

#define CLIENT_SEND(this, in, type) \
	client_send(this, in, sizeof(type))

typedef struct ClientNode ClientNode;
typedef struct Lobby Lobby;

struct Client
{
	ClientNode* node;
	
	server_rwlock_t lock;
	server_thread_t thread;
	
	Lobby* current_lobby;
	
	int retries;
	int read;
	int bytes_read;
	bool disconnected;
	
	u32 ip;
	int ping;
	
	int fd;
	
	bool hello;
};

typedef struct Client Client;

struct ClientNode
{
	Node node;
	Client client;
};

typedef enum
{
	COMMAND_HELLO = 1,
	COMMAND_WELCOME,
	COMMAND_PING,
	COMMAND_PONG,
	COMMAND_CREATE_LOBBY,
	COMMAND_CREATE_LOBBY_STATUS,
} Command;

int client_recv(Client* this, void* out, size_t size);
int client_send(Client* this, void* in, size_t size);

void client_init(Client* this, ClientNode* node, server_socket_t* sock);
void client_handle(Client* this);
void client_disconnect(Client* this, const char* reason);
void client_destroy(Client* this);