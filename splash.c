#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"

static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s [--headless] [--delay ms] [--render-every n] joueur1.so joueur2.so joueur3.so joueur4.so\n", prog);
    printf("\nExemple:\n");
    printf("  %s --delay 1 --render-every 32 ./p1_square.so ./p2_snake.so ./p3_spiral.so ./p4_dash.so\n", prog);
}

static int parse_uint_arg(const char *text, unsigned int *out_value)
{
    char *endptr = NULL;
    unsigned long value = 0;

    if (!text || !out_value)
        return -1;

    value = strtoul(text, &endptr, 10);
    if (endptr == text || *endptr != '\0')
        return -1;

    *out_value = (unsigned int)value;
    return 0;
}

static int paths_are_unique(const char *const *paths, unsigned int count)
{
    unsigned int i = 0;
    unsigned int j = 0;

    for (i = 0; i < count; i++)
    {
        for (j = i + 1u; j < count; j++)
        {
            if (strcmp(paths[i], paths[j]) == 0)
            {
                fprintf(stderr, "Erreur: la meme bibliotheque est passee plusieurs fois: %s\n", paths[i]);
                return 0;
            }
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    int ret = 1;
    int headless = 0;
    unsigned int delay_ms = 0;
    unsigned int render_every = 32u;
    const char *player_paths[GAME_MAX_PLAYERS];
    unsigned int player_count = 0;
    int i = 0;
    game_t game;

    memset(&game, 0, sizeof(game));
    memset(player_paths, 0, sizeof(player_paths));

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--headless") == 0)
        {
            headless = 1;
        }
        else if (strcmp(argv[i], "--delay") == 0)
        {
            if ((i + 1) >= argc || parse_uint_arg(argv[i + 1], &delay_ms) != 0)
            {
                fprintf(stderr, "Erreur: valeur invalide pour --delay\n");
                goto end;
            }
            i++;
        }
        else if (strcmp(argv[i], "--render-every") == 0)
        {
            if ((i + 1) >= argc || parse_uint_arg(argv[i + 1], &render_every) != 0)
            {
                fprintf(stderr, "Erreur: valeur invalide pour --render-every\n");
                goto end;
            }
            i++;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            ret = 0;
            goto end;
        }
        else
        {
            if (player_count >= GAME_MAX_PLAYERS)
            {
                fprintf(stderr, "Erreur: il faut exactement 4 bibliotheques joueurs.\n");
                goto end;
            }
            player_paths[player_count++] = argv[i];
        }
    }

    if (player_count != GAME_MAX_PLAYERS)
    {
        fprintf(stderr, "Erreur: il faut exactement 4 bibliotheques joueurs.\n\n");
        print_usage(argv[0]);
        goto end;
    }

    if (!paths_are_unique(player_paths, player_count))
        goto end;

    if (game_init(&game, player_paths, player_count, headless, render_every, delay_ms) != 0)
        goto end;

    if (game_run(&game) != 0)
        goto end;

    game_print_results(&game);

    ret = 0;
end:
    game_shutdown(&game);
    return ret;
}
