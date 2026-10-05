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
#include <heap.h>
#include <swap_vector.h>
#include <utils.h>

#define USE_DEFAULT 0

#define HEAP_SIZE 128*1024*1024

int quit = 0;

int sfd;

SwapVector clients;
server_rwlock_t clients_lock;

void sig_q(int sig)
{
	quit = 1;
	close(sfd);
}

void* listen_func(void* context)
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
				svec_bump(&clients);
				Client* top = SVEC_GET_TOP(&clients, Client);
				
				top->fd = next_fd;
				memcpy(&top->sock, &next_client, sizeof(server_socket_t));
				
				client_init(top);
			});
		}
	}
	
	return NULL;
}

int main(int argc, char** argv)
{
	Config config;
	
	server_init_utils();
	heap_init(HEAP_SIZE);
	
	svec_sized_init(&clients, sizeof(Client));
	
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
	
	if (listen(sfd, 1) < 0)
	{
		LOGEQ("couldn't listen\n");
		
		goto release;
	}
	
	signal(SIGINT, sig_q);
	
	pthread_t listen_thread;
	pthread_create(&listen_thread, NULL, listen_func, (void*) &sfd);
	
	while (!quit)
	{
		size_t len;
		
		LOCK_READ(clients_lock,
		{
			len = clients.length;
			
			for (size_t i = 0; i < len; ++i)
			{
				Client* this = SVEC_GET(&clients, Client, i);
				
				if (this->disconnected)
				{
					continue;
				}
				
				client_handle(this);
			}
		});
	}
	
join:
	pthread_join(listen_thread, NULL);
	
release:
	for (size_t i = 0; i < clients.length; ++i)
	{
		Client* this = SVEC_GET(&clients, Client, i);
		close(this->fd);
	}
	
	svec_release(&clients);
	heap_shutdown();
	server_deinit_utils();
	
	return 0;
}