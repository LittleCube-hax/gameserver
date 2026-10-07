#pragma once

#include <client.h>

void cmd_recv_hello(Client* this);
void cmd_recv_pong(Client* this);
void cmd_recv_create_lobby(Client* this);