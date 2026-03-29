#ifndef RENDERER_H
#define RENDERER_H

#include "board.h"

typedef struct
{
    char name[128];
    int x;
    int y;
    int credits;
    int active;
} renderer_player_state_t;

typedef struct
{
    int enabled;
    void *display;
    unsigned long window;
    void *gc;
    unsigned int cell_size;
    unsigned int board_px_width;
    unsigned int board_px_height;
    unsigned int window_width;
    unsigned int window_height;
    unsigned long colors[4];
    unsigned long background_color;
    unsigned long empty_color;
    unsigned long player_cursor_color;
    unsigned long panel_color;
    unsigned long border_color;
    unsigned long text_color;
    unsigned long wm_delete_message;
} renderer_t;

int renderer_init(renderer_t *renderer, unsigned int cell_size);
void renderer_shutdown(renderer_t *renderer);
void renderer_poll(renderer_t *renderer, int *stop_requested);
void renderer_draw(renderer_t *renderer,
                   const board_t *board,
                   const renderer_player_state_t *players,
                   unsigned int player_count,
                   unsigned long turn_count);

#endif
