#include <tomlc17.h>

#include <config.h>

#define CONFIG(type, table, out_name, key) \
	toml_datum_t out_name; \
	if (!config_get_top_##type(&table, &out_name, key)) \
	{ \
		return false; \
	}

bool config_get_top_string(toml_result_t* table, toml_datum_t* string, char* key)
{
	*string = toml_seek(table->toptab, key);
	
	if (string->type != TOML_STRING)
	{
		fprintf(stderr, "%s not found, or is not a string field.\n", key);
		return false;
	}
	
	return true;
}

bool config_get_top_int64(toml_result_t* table, toml_datum_t* i64, char* key)
{
	*i64 = toml_seek(table->toptab, key);
	
	if (i64->type != TOML_INT64)
	{
		fprintf(stderr, "%s not found, or is not an int field.\n", key);
		return false;
	}
	
	return true;
}

bool config_get_top_boolean(toml_result_t* table, toml_datum_t* b, char* key)
{
	*b = toml_seek(table->toptab, key);
	
	if (b->type != TOML_BOOLEAN)
	{
		fprintf(stderr, "%s not found, or is not a boolean field.\n", key);
		return false;
	}
	
	return true;
}

bool config_read(char* path, Config* config_out)
{
	toml_result_t table = toml_parse_file_ex(path);
	
	if (!table.ok)
	{
		fprintf(stderr, "Error reading toml.\n%s\n", table.errmsg);
		return false;
	}
	
	CONFIG(string, table, ip, "server.ip");
	snprintf(config_out->ip, 16, "%s", ip.u.s);
	
	CONFIG(int64, table, port, "server.port");
	config_out->port = port.u.int64;
	
	toml_free(table);
	
	return true;
}