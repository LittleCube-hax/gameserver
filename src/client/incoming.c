#include <incoming.h>
#include <outgoing.h>

void cmd_recv_hello(Client* this)
{
	u64 now = get_elapsed_ns();
	
	u32 magic;
	CLIENT_READ_BLOCK(this, &magic, u32, 1000);
	
	if (magic != 0x34769420)
	{
		client_disconnect(this, "bad magic");
		return;
	}
	
	u64 timestamp;
	CLIENT_READ_BLOCK(this, &timestamp, u64, 1000);
	
	this->ping = (int) (now - timestamp);
	
	const u64 ten_seconds_ns = 10*1000*1000*1000ULL;
	
	if (!(timestamp > now - ten_seconds_ns && timestamp < now + ten_seconds_ns))
	{
		client_disconnect(this, "bad timestamp");
		return;
	}
	
	this->hello = true;
	
	cmd_send_welcome(this);
}

void cmd_recv_pong(Client* this)
{
	printf("pong from client %d\n", this->fd);
}

void cmd_recv_create_lobby(Client* this)
{
	
}