#pragma once

#include <client.h>

void cmd_send_welcome(Client* this);
void cmd_send_ping(Client* this);
void cmd_send_create_lobby_status(Client* this, int status);
void cmd_send_list_lobbies(Client* this, u16 num_lobbies, u16* lobby_ids);