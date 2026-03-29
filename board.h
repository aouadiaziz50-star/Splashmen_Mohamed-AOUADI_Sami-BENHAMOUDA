#ifndef BOARD_H
#define BOARD_H

#define BOARD_WIDTH 100
#define BOARD_HEIGHT 100
#define BOARD_EMPTY (-1)

typedef struct
{
    int cells[BOARD_HEIGHT][BOARD_WIDTH];
    int scores[4];
} board_t;

void board_init(board_t *board);
void board_mark(board_t *board, int player_id, int x, int y);
int board_get_owner(const board_t *board, int x, int y);
int board_get_score(const board_t *board, int player_id);

#endif
