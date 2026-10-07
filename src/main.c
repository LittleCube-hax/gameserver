#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <common.h>
#include <config.h>
#include <client.h>
#include <outgoing.h>
#include <heap.h>
#include <list.h>
#include <utils.h>

#define USE_DEFAULT 0

#define HEAP_SIZE 128*1024*1024

int quit = 0;

int sfd;

List clients;
server_rwlock_t clients_lock;

void sig_q(int sig)
{
	quit = 1;
	close(sfd);
}

DECLARE_RUNTIME_THREAD_FUNC(client_func)
{
	Node* n = (Node*) ctx;
	Client* this = &n->client;
	
	while (!quit)
	{
		if (this->disconnected)
		{
			break;
		}
		
		client_handle(this);
		
		server_sleep(0);
	}
	
	LOCK_WRITE(clients_lock,
	{
		close(this->fd);
		list_remove(&clients, n);
	});
	
	thread_exit();
	
	return NULL;
}

DECLARE_RUNTIME_THREAD_FUNC(listen_func)
{
	int next_fd;
	server_socket_t next_client;
	
	unsigned int c_size;
	
	while (!quit)
	{
		next_fd = accept(sfd, &next_client, &c_size);
		
		if (next_fd >= 0)
		{
			LOCK_WRITE(clients_lock,
			{
				list_bump_back(&clients);
				Node* n = clients.tail;
				Client* top = &n->client;
				
				top->fd = next_fd;
				client_init(top, &next_client);
				
				server_thread_t client_thread;
				thread_start(client_func, &client_thread, n);
			});
		}
		
		server_sleep(0);
	}
	
	thread_exit();
	
	return NULL;
}

int main(int argc, char** argv)
{
	Config config;
	
	server_init_utils();
	heap_init(HEAP_SIZE);
	
	list_init(&clients);
	
	if (!config_read("config.toml", &config))
	{
		return EXIT_FAILURE;
	}
	
	sfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, USE_DEFAULT);
	
	if (sfd < 0)
	{
		LOGEQ("couldn't open socket\n");
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	
	if (inet_pton(AF_INET, config.ip, &addr.sin_addr) <= 0)
	{
		LOGEQ("couldn't convert ip\n");
		
		goto release;
	}
	
	addr.sin_port = htons(config.port);
	
	if (bind(sfd, (server_socket_t*) &addr, sizeof(struct sockaddr_in)) < 0)
	{
		LOGEQ("couldn't bind\n");
		
		goto release;
	}
	
	if (listen(sfd, 16) < 0)
	{
		LOGEQ("couldn't listen\n");
		
		goto release;
	}
	
	signal(SIGINT, sig_q);
	
	server_thread_t listen_thread;
	thread_start(listen_func, &listen_thread, NULL);
	
	while (!quit)
	{
		LOCK_WRITE(clients_lock,
		{
			for (Node* n = clients.head; n != NULL; n = n->next)
			{
				printf("pinging %d\n", n->client.fd);
				cmd_send_ping(&n->client);
			}
		});
		
		server_sleep(1000);
	}
	
join:
	thread_join(&listen_thread);
	
release:
	LOCK_WRITE(clients_lock,
	{
		for (Node* n = clients.head; n != NULL; n = n->next)
		{
			Client* this = &n->client;
			printf("closing client %d\n", this->fd);
			thread_join(&this->thread);
			
			close(this->fd);
			n = n->next;
			list_pop_front(&clients);
		}
	});
	
	heap_shutdown();
	server_deinit_utils();
	
	return 0;
}