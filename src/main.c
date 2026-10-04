#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <common.h>
#include <config.h>
#include <heap.h>
#include <swap_vector.h>
#include <utils.h>

#define USE_DEFAULT 0

#define HEAP_SIZE 128*1024*1024

typedef enum
{
	COMMAND_NONE = 0,
} Command;

int quit = 0;

int sfd;

SwapVector clients;

void sig_q(int sig)
{
	quit = 1;
	close(sfd);
	exit(EXIT_SUCCESS);
}

void* listen_func(void* context)
{
	struct sockaddr next_client;
	
	unsigned int c_size;
	
	while (true)
	{
		if (accept(sfd, &next_client, &c_size) < 0)
		{
			LOGE("couldn't accept\n");
		}
		
		else
		{
			svec_bump(&clients);
			memcpy(SVEC_GET_TOP(&clients, struct sockaddr), &next_client, sizeof(struct sockaddr));
			printf("accepted\n");
		}
	}
	
	return NULL;
}

int main(int argc, char** argv)
{
	Config config;
	
	server_init_utils();
	heap_init(HEAP_SIZE);
	
	svec_sized_init(&clients, sizeof(struct sockaddr));
	
	if (!config_read("config.toml", &config))
	{
		return EXIT_FAILURE;
	}
	
	sfd = socket(AF_INET, SOCK_STREAM, USE_DEFAULT);
	
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
	
	if (bind(sfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)) < 0)
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
	
join:
	pthread_join(listen_thread, NULL);
	
release:
	svec_release(&clients);
	heap_shutdown();
	server_deinit_utils();
	close(sfd);
	
	return 0;
}