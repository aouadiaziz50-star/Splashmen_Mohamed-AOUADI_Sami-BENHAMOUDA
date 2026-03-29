#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "actions.h"
#include "game.h"

static void sleep_ms(unsigned int delay_ms)
{
    struct timeval tv;

    if (delay_ms == 0u)
        return;

    tv.tv_sec = (long)(delay_ms / 1000u);
    tv.tv_usec = (long)((delay_ms % 1000u) * 1000u);
    select(0, NULL, NULL, NULL, &tv);
}

static int wrap_coord(int value, int max_value)
{
    if (max_value <= 0)
        return 0;

    while (value < 0)
        value += max_value;

    while (value >= max_value)
        value -= max_value;

    return value;
}

static int action_cost(char action)
{
    int ret = 1;

    switch (action)
    {
        case ACTION_MOVE_L:
        case ACTION_MOVE_R:
        case ACTION_MOVE_U:
        case ACTION_MOVE_D:
        case ACTION_STILL:
            ret = 1;
            break;

        case ACTION_DASH_L:
        case ACTION_DASH_R:
        case ACTION_DASH_U:
        case ACTION_DASH_D:
            ret = 10;
            break;

        case ACTION_TELEPORT_L:
        case ACTION_TELEPORT_R:
        case ACTION_TELEPORT_U:
        case ACTION_TELEPORT_D:
            ret = 2;
            break;

        default:
            ret = 1;
            break;
    }

    return ret;
}

static char normalize_action(char action)
{
    char ret = ACTION_STILL;

    switch (action)
    {
        case ACTION_MOVE_L:
        case ACTION_MOVE_R:
        case ACTION_MOVE_U:
        case ACTION_MOVE_D:
        case ACTION_DASH_L:
        case ACTION_DASH_R:
        case ACTION_DASH_U:
        case ACTION_DASH_D:
        case ACTION_TELEPORT_L:
        case ACTION_TELEPORT_R:
        case ACTION_TELEPORT_U:
        case ACTION_TELEPORT_D:
        case ACTION_STILL:
            ret = action;
            break;

        default:
            ret = ACTION_STILL;
            break;
    }

    return ret;
}

static void action_to_delta(char action, int *dx, int *dy)
{
    int local_dx = 0;
    int local_dy = 0;

    switch (action)
    {
        case ACTION_MOVE_L:
        case ACTION_DASH_L:
        case ACTION_TELEPORT_L:
            local_dx = -1;
            break;

        case ACTION_MOVE_R:
        case ACTION_DASH_R:
        case ACTION_TELEPORT_R:
            local_dx = 1;
            break;

        case ACTION_MOVE_U:
        case ACTION_DASH_U:
        case ACTION_TELEPORT_U:
            local_dy = -1;
            break;

        case ACTION_MOVE_D:
        case ACTION_DASH_D:
        case ACTION_TELEPORT_D:
            local_dy = 1;
            break;

        default:
            break;
    }

    if (dx)
        *dx = local_dx;
    if (dy)
        *dy = local_dy;
}

static void move_one_step(game_t *game, game_player_t *player, int dx, int dy)
{
    player->x = wrap_coord(player->x + dx, BOARD_WIDTH);
    player->y = wrap_coord(player->y + dy, BOARD_HEIGHT);
    board_mark(&game->board, (int)(player - game->players), player->x, player->y);
}

static void apply_action(game_t *game, game_player_t *player, char action)
{
    int dx = 0;
    int dy = 0;
    int i = 0;

    if (!game || !player)
        return;

    action_to_delta(action, &dx, &dy);

    switch (action)
    {
        case ACTION_MOVE_L:
        case ACTION_MOVE_R:
        case ACTION_MOVE_U:
        case ACTION_MOVE_D:
            move_one_step(game, player, dx, dy);
            break;

        case ACTION_DASH_L:
        case ACTION_DASH_R:
        case ACTION_DASH_U:
        case ACTION_DASH_D:
            for (i = 0; i < 8; i++)
            {
                move_one_step(game, player, dx, dy);
            }
            break;

        case ACTION_TELEPORT_L:
        case ACTION_TELEPORT_R:
        case ACTION_TELEPORT_U:
        case ACTION_TELEPORT_D:
            player->x = wrap_coord(player->x + (dx * 8), BOARD_WIDTH);
            player->y = wrap_coord(player->y + (dy * 8), BOARD_HEIGHT);
            board_mark(&game->board, (int)(player - game->players), player->x, player->y);
            break;

        case ACTION_STILL:
        default:
            break;
    }
}

static int has_active_players(const game_t *game)
{
    unsigned int i = 0;

    if (!game)
        return 0;

    for (i = 0; i < game->player_count; i++)
    {
        if (game->players[i].active)
            return 1;
    }

    return 0;
}

static void fill_renderer_states(const game_t *game,
                                 renderer_player_state_t *states,
                                 unsigned int max_count)
{
    unsigned int i = 0;

    if (!game || !states)
        return;

    for (i = 0; i < game->player_count && i < max_count; i++)
    {
        memset(&states[i], 0, sizeof(states[i]));
        snprintf(states[i].name, sizeof(states[i].name), "%s", game->players[i].lib.name);
        states[i].x = game->players[i].x;
        states[i].y = game->players[i].y;
        states[i].credits = game->players[i].credits;
        states[i].active = game->players[i].active;
    }
}

static void render_game(game_t *game)
{
    renderer_player_state_t states[GAME_MAX_PLAYERS];

    if (!game || !game->renderer.enabled)
        return;

    memset(states, 0, sizeof(states));
    fill_renderer_states(game, states, GAME_MAX_PLAYERS);
    renderer_draw(&game->renderer, &game->board, states, game->player_count, game->action_counter);
}

static void init_start_positions(game_t *game)
{
    static const int start_x[GAME_MAX_PLAYERS] = { 0, BOARD_WIDTH - 1, BOARD_WIDTH - 1, 0 };
    static const int start_y[GAME_MAX_PLAYERS] = { 0, 0, BOARD_HEIGHT - 1, BOARD_HEIGHT - 1 };
    unsigned int i = 0;

    for (i = 0; i < game->player_count; i++)
    {
        game->players[i].x = start_x[i];
        game->players[i].y = start_y[i];
        game->players[i].credits = GAME_START_CREDITS;
        game->players[i].active = 1;
        board_mark(&game->board, (int)i, game->players[i].x, game->players[i].y);
    }
}

int game_init(game_t *game,
              const char *const *player_paths,
              unsigned int player_count,
              int headless,
              unsigned int render_every,
              unsigned int delay_ms)
{
    int ret = -1;
    unsigned int i = 0;

    if (!game || !player_paths || player_count == 0u || player_count > GAME_MAX_PLAYERS)
        goto end;

    memset(game, 0, sizeof(*game));
    board_init(&game->board);

    game->player_count = player_count;
    game->render_every = (render_every == 0u) ? 1u : render_every;
    game->delay_ms = delay_ms;
    game->headless = headless;

    for (i = 0; i < player_count; i++)
    {
        if (player_load(&game->players[i].lib, player_paths[i]) != 0)
            goto end;
    }

    init_start_positions(game);

    if (!game->headless)
    {
        ret = renderer_init(&game->renderer, 7u);
        if (ret == 1)
        {
            game->headless = 1;
            ret = 0;
        }
        else if (ret != 0)
        {
            goto end;
        }
    }

    ret = 0;
end:
    if (ret != 0)
    {
        game_shutdown(game);
    }
    return ret;
}

void game_shutdown(game_t *game)
{
    unsigned int i = 0;

    if (!game)
        return;

    renderer_shutdown(&game->renderer);

    for (i = 0; i < GAME_MAX_PLAYERS; i++)
    {
        player_unload(&game->players[i].lib);
    }

    memset(game, 0, sizeof(*game));
}

int game_run(game_t *game)
{
    int ret = -1;
    unsigned int i = 0;

    if (!game)
        goto end;

    render_game(game);

    while (!game->stop_requested && has_active_players(game))
    {
        for (i = 0; i < game->player_count; i++)
        {
            char action = ACTION_STILL;
            int cost = 0;

            if (!game->players[i].active)
                continue;

            action = normalize_action(game->players[i].lib.get_action());
            cost = action_cost(action);

            if (cost > game->players[i].credits)
            {
                action = ACTION_STILL;
                cost = action_cost(action);
            }

            if (cost > game->players[i].credits)
            {
                game->players[i].credits = 0;
                game->players[i].active = 0;
                continue;
            }

            apply_action(game, &game->players[i], action);
            game->players[i].credits -= cost;
            if (game->players[i].credits <= 0)
            {
                game->players[i].credits = 0;
                game->players[i].active = 0;
            }

            game->action_counter++;

            if (game->renderer.enabled)
            {
                renderer_poll(&game->renderer, &game->stop_requested);
                if ((game->action_counter % game->render_every) == 0u || !has_active_players(game))
                {
                    render_game(game);
                    if (game->delay_ms > 0u)
                    {
                        sleep_ms(game->delay_ms);
                    }
                }
            }
            else if ((game->action_counter % 1000u) == 0u)
            {
                printf("Progression: %lu actions calculees...\n", game->action_counter);
            }

            if (game->stop_requested)
                break;
        }
    }

    if (game->renderer.enabled)
    {
        render_game(game);
        renderer_poll(&game->renderer, &game->stop_requested);
    }

    ret = 0;
end:
    return ret;
}

void game_print_results(const game_t *game)
{
    unsigned int i = 0;
    int best_score = -1;
    int winners[GAME_MAX_PLAYERS];
    int winner_count = 0;

    memset(winners, 0, sizeof(winners));

    if (!game)
        return;

    printf("\n===== RESULTATS =====\n");
    for (i = 0; i < game->player_count; i++)
    {
        int score = board_get_score(&game->board, (int)i);
        printf("Joueur %u (%s) : score=%d, credits=%d, position=(%d,%d)\n",
               i + 1u,
               game->players[i].lib.name,
               score,
               game->players[i].credits,
               game->players[i].x,
               game->players[i].y);

        if (score > best_score)
        {
            best_score = score;
            winner_count = 0;
            winners[winner_count++] = (int)i;
        }
        else if (score == best_score)
        {
            winners[winner_count++] = (int)i;
        }
    }

    if (winner_count == 1)
    {
        printf("Vainqueur : Joueur %d (%s) avec %d cases.\n",
               winners[0] + 1,
               game->players[winners[0]].lib.name,
               best_score);
    }
    else
    {
        printf("Egalite entre : ");
        for (i = 0; i < (unsigned int)winner_count; i++)
        {
            if (i > 0u)
                printf(", ");
            printf("Joueur %d (%s)", winners[i] + 1, game->players[winners[i]].lib.name);
        }
        printf(" avec %d cases.\n", best_score);
    }
}
