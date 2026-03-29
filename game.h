#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"
#include "renderer.h"

#define GAME_MAX_PLAYERS 4
#define GAME_START_CREDITS 9000

typedef struct
{
    player_lib_t lib;
    int x;
    int y;
    int credits;
    int active;
} game_player_t;

typedef struct
{
    board_t board;
    game_player_t players[GAME_MAX_PLAYERS];
    unsigned int player_count;
    unsigned long action_counter;
    unsigned int render_every;
    unsigned int delay_ms;
    int headless;
    int stop_requested;
    renderer_t renderer;
} game_t;

int game_init(game_t *game,
              const char *const *player_paths,
              unsigned int player_count,
              int headless,
              unsigned int render_every,
              unsigned int delay_ms);
void game_shutdown(game_t *game);
int game_run(game_t *game);
void game_print_results(const game_t *game);

#endif
