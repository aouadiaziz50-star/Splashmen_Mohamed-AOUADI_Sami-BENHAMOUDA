#include <string.h>

#include "board.h"

void board_init(board_t *board)
{
    int x = 0;
    int y = 0;

    memset(board->scores, 0, sizeof(board->scores));

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            board->cells[y][x] = BOARD_EMPTY;
        }
    }
}

void board_mark(board_t *board, int player_id, int x, int y)
{
    int old_owner = BOARD_EMPTY;

    if (!board)
        return;

    if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT)
        return;

    if (player_id < 0 || player_id >= 4)
        return;

    old_owner = board->cells[y][x];
    if (old_owner == player_id)
        return;

    if (old_owner >= 0 && old_owner < 4)
    {
        board->scores[old_owner]--;
    }

    board->cells[y][x] = player_id;
    board->scores[player_id]++;
}

int board_get_owner(const board_t *board, int x, int y)
{
    if (!board)
        return BOARD_EMPTY;

    if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT)
        return BOARD_EMPTY;

    return board->cells[y][x];
}

int board_get_score(const board_t *board, int player_id)
{
    if (!board)
        return 0;

    if (player_id < 0 || player_id >= 4)
        return 0;

    return board->scores[player_id];
}
