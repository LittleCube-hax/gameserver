#pragma once

#include <stdbool.h>

typedef struct
{
	char ip[64];
	short port;
} Config;

bool config_read(char* path, Config* config_out);