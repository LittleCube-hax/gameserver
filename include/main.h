#pragma once

#include <lobby.h>
#include <list.h>
#include <utils.h>

extern List clients;
extern server_rwlock_t clients_lock;

extern Lobby welcome_lobby;
extern List lobbies;
extern server_rwlock_t lobbies_lock;