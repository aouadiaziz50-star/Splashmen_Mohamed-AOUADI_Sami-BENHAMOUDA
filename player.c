#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "player.h"

static const char *find_basename_local(const char *path)
{
    const char *base = path;
    const char *p = path;

    while (*p != '\0')
    {
        if (*p == '/')
        {
            base = p + 1;
        }
        p++;
    }

    return base;
}

int player_extract_name(const char *path, char *out_name, size_t out_size)
{
    int ret = -1;
    const char *base = NULL;
    size_t len = 0;

    if (!path || !out_name || out_size == 0u)
        goto end;

    base = find_basename_local(path);
    len = strlen(base);

    if (len >= 3u && strcmp(base + len - 3u, ".so") == 0)
    {
        len -= 3u;
    }

    if (len >= out_size)
    {
        len = out_size - 1u;
    }

    memcpy(out_name, base, len);
    out_name[len] = '\0';

    ret = 0;
end:
    return ret;
}

int player_load(player_lib_t *player, const char *path)
{
    int ret = -1;
    dlerror();

    if (!player || !path)
        goto end;

    memset(player, 0, sizeof(*player));

    if (snprintf(player->path, sizeof(player->path), "%s", path) >= (int)sizeof(player->path))
    {
        fprintf(stderr, "Erreur: chemin joueur trop long: %s\n", path);
        goto end;
    }

    if (player_extract_name(path, player->name, sizeof(player->name)) != 0)
    {
        fprintf(stderr, "Erreur: nom joueur invalide: %s\n", path);
        goto end;
    }

    player->handle = dlopen(path, RTLD_NOW);
    if (!player->handle)
    {
        fprintf(stderr, "Erreur dlopen(%s): %s\n", path, dlerror());
        goto end;
    }

    dlerror();
    player->get_action = (player_get_action_fn_t)dlsym(player->handle, "get_action");
    if (!player->get_action)
    {
        fprintf(stderr, "Erreur dlsym(get_action) pour %s: %s\n", path, dlerror());
        goto end;
    }

    ret = 0;
end:
    if (ret != 0)
    {
        player_unload(player);
    }
    return ret;
}

void player_unload(player_lib_t *player)
{
    if (!player)
        return;

    if (player->handle)
    {
        dlclose(player->handle);
    }

    memset(player, 0, sizeof(*player));
}
