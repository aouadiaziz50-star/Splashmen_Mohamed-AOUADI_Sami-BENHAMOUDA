#ifndef PLAYER_H
#define PLAYER_H

#include <stddef.h>

#define PLAYER_NAME_LEN 128

typedef char (*player_get_action_fn_t)(void);

typedef struct
{
    char path[PLAYER_NAME_LEN];
    char name[PLAYER_NAME_LEN];
    void *handle;
    player_get_action_fn_t get_action;
} player_lib_t;

int player_load(player_lib_t *player, const char *path);
void player_unload(player_lib_t *player);
int player_extract_name(const char *path, char *out_name, size_t out_size);

#endif
