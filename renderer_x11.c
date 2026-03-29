#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "renderer.h"

#define PANEL_WIDTH 260
#define DEFAULT_BOARD_MARGIN 10

static unsigned long alloc_named_color(Display *display,
                                       Colormap colormap,
                                       const char *name,
                                       unsigned long fallback)
{
    unsigned long ret = fallback;
    XColor color;
    XColor exact;

    if (XAllocNamedColor(display, colormap, name, &color, &exact) != 0)
    {
        ret = color.pixel;
    }

    return ret;
}

int renderer_init(renderer_t *renderer, unsigned int cell_size)
{
    int ret = -1;
    Display *display = NULL;
    int screen = 0;
    Window window = 0;
    GC gc = NULL;
    Colormap colormap = 0;
    Atom wm_delete = 0;
    unsigned int board_width = 0;
    unsigned int board_height = 0;
    unsigned int window_width = 0;
    unsigned int window_height = 0;

    if (!renderer)
        goto end;

    memset(renderer, 0, sizeof(*renderer));

    display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "Info: affichage X11 indisponible, passage en mode headless.\n");
        ret = 1;
        goto end;
    }

    screen = DefaultScreen(display);
    colormap = DefaultColormap(display, screen);

    board_width = (unsigned int)(BOARD_WIDTH * (int)cell_size);
    board_height = (unsigned int)(BOARD_HEIGHT * (int)cell_size);
    window_width = board_width + PANEL_WIDTH + (DEFAULT_BOARD_MARGIN * 3u);
    window_height = board_height + (DEFAULT_BOARD_MARGIN * 2u);

    window = XCreateSimpleWindow(display,
                                 RootWindow(display, screen),
                                 0,
                                 0,
                                 window_width,
                                 window_height,
                                 1,
                                 BlackPixel(display, screen),
                                 WhitePixel(display, screen));
    if (!window)
    {
        fprintf(stderr, "Erreur: creation fenetre X11 impossible.\n");
        goto end;
    }

    XStoreName(display, window, "Splashmem");
    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);

    wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete, 1);

    gc = XCreateGC(display, window, 0, NULL);
    if (!gc)
    {
        fprintf(stderr, "Erreur: creation GC X11 impossible.\n");
        goto end;
    }

    XMapWindow(display, window);
    XFlush(display);

    renderer->enabled = 1;
    renderer->display = display;
    renderer->window = window;
    renderer->gc = gc;
    renderer->cell_size = cell_size;
    renderer->board_px_width = board_width;
    renderer->board_px_height = board_height;
    renderer->window_width = window_width;
    renderer->window_height = window_height;
    renderer->wm_delete_message = (unsigned long)wm_delete;

    renderer->background_color = alloc_named_color(display, colormap, "#111111", WhitePixel(display, screen));
    renderer->empty_color = alloc_named_color(display, colormap, "#202020", BlackPixel(display, screen));
    renderer->panel_color = alloc_named_color(display, colormap, "#171717", WhitePixel(display, screen));
    renderer->border_color = alloc_named_color(display, colormap, "#404040", BlackPixel(display, screen));
    renderer->text_color = alloc_named_color(display, colormap, "#f0f0f0", BlackPixel(display, screen));
    renderer->player_cursor_color = alloc_named_color(display, colormap, "#ffffff", WhitePixel(display, screen));
    renderer->colors[0] = alloc_named_color(display, colormap, "#e63946", BlackPixel(display, screen));
    renderer->colors[1] = alloc_named_color(display, colormap, "#457b9d", BlackPixel(display, screen));
    renderer->colors[2] = alloc_named_color(display, colormap, "#2a9d8f", BlackPixel(display, screen));
    renderer->colors[3] = alloc_named_color(display, colormap, "#ffb703", BlackPixel(display, screen));

    ret = 0;
end:
    if (ret != 0)
    {
        if (gc)
            XFreeGC(display, gc);
        if (window)
            XDestroyWindow(display, window);
        if (display)
            XCloseDisplay(display);
    }
    return ret;
}

void renderer_shutdown(renderer_t *renderer)
{
    Display *display = NULL;

    if (!renderer || !renderer->enabled)
        return;

    display = (Display *)renderer->display;

    if (renderer->gc)
    {
        XFreeGC(display, (GC)renderer->gc);
    }

    if (renderer->window)
    {
        XDestroyWindow(display, (Window)renderer->window);
    }

    XCloseDisplay(display);
    memset(renderer, 0, sizeof(*renderer));
}

void renderer_poll(renderer_t *renderer, int *stop_requested)
{
    Display *display = NULL;

    if (!renderer || !renderer->enabled)
        return;

    display = (Display *)renderer->display;

    while (XPending(display) > 0)
    {
        XEvent event;
        memset(&event, 0, sizeof(event));
        XNextEvent(display, &event);

        if (event.type == ClientMessage)
        {
            if ((unsigned long)event.xclient.data.l[0] == renderer->wm_delete_message)
            {
                if (stop_requested)
                    *stop_requested = 1;
            }
        }
        else if (event.type == DestroyNotify)
        {
            if (stop_requested)
                *stop_requested = 1;
        }
        else if (event.type == KeyPress)
        {
            char key_buffer[8];
            KeySym key = 0;
            int len = XLookupString(&event.xkey, key_buffer, sizeof(key_buffer), &key, NULL);
            if (len > 0 && (key_buffer[0] == 27 || key_buffer[0] == 'q' || key_buffer[0] == 'Q'))
            {
                if (stop_requested)
                    *stop_requested = 1;
            }
        }
    }
}

static void draw_text_line(Display *display,
                           Window window,
                           GC gc,
                           int x,
                           int y,
                           const char *text)
{
    if (!text)
        return;

    XDrawString(display, window, gc, x, y, text, (int)strlen(text));
}

void renderer_draw(renderer_t *renderer,
                   const board_t *board,
                   const renderer_player_state_t *players,
                   unsigned int player_count,
                   unsigned long turn_count)
{
    Display *display = NULL;
    Window window = 0;
    GC gc = NULL;
    unsigned int x = 0;
    unsigned int y = 0;
    int board_origin_x = DEFAULT_BOARD_MARGIN;
    int board_origin_y = DEFAULT_BOARD_MARGIN;
    int panel_origin_x = 0;
    char line[256];

    if (!renderer || !renderer->enabled || !board || !players)
        return;

    display = (Display *)renderer->display;
    window = (Window)renderer->window;
    gc = (GC)renderer->gc;
    panel_origin_x = board_origin_x + (int)renderer->board_px_width + DEFAULT_BOARD_MARGIN;

    XSetForeground(display, gc, renderer->background_color);
    XFillRectangle(display, window, gc, 0, 0, renderer->window_width, renderer->window_height);

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            int owner = board_get_owner(board, (int)x, (int)y);
            unsigned long color = renderer->empty_color;

            if (owner >= 0 && owner < 4)
            {
                color = renderer->colors[owner];
            }

            XSetForeground(display, gc, color);
            XFillRectangle(display,
                           window,
                           gc,
                           board_origin_x + ((int)x * (int)renderer->cell_size),
                           board_origin_y + ((int)y * (int)renderer->cell_size),
                           renderer->cell_size,
                           renderer->cell_size);
        }
    }

    XSetForeground(display, gc, renderer->border_color);
    XDrawRectangle(display,
                   window,
                   gc,
                   board_origin_x - 1,
                   board_origin_y - 1,
                   renderer->board_px_width + 1,
                   renderer->board_px_height + 1);

    for (x = 0; x < player_count; x++)
    {
        int px = board_origin_x + (players[x].x * (int)renderer->cell_size);
        int py = board_origin_y + (players[x].y * (int)renderer->cell_size);
        unsigned int marker_size = renderer->cell_size;

        if (marker_size > 4u)
            marker_size -= 2u;

        XSetForeground(display, gc, renderer->player_cursor_color);
        XDrawRectangle(display, window, gc, px, py, marker_size, marker_size);
    }

    XSetForeground(display, gc, renderer->panel_color);
    XFillRectangle(display,
                   window,
                   gc,
                   panel_origin_x,
                   DEFAULT_BOARD_MARGIN,
                   PANEL_WIDTH,
                   renderer->board_px_height);

    XSetForeground(display, gc, renderer->border_color);
    XDrawRectangle(display,
                   window,
                   gc,
                   panel_origin_x,
                   DEFAULT_BOARD_MARGIN,
                   PANEL_WIDTH,
                   renderer->board_px_height);

    XSetForeground(display, gc, renderer->text_color);
    snprintf(line, sizeof(line), "Splashmem IA");
    draw_text_line(display, window, gc, panel_origin_x + 12, 30, line);

    snprintf(line, sizeof(line), "Tour global : %lu", turn_count);
    draw_text_line(display, window, gc, panel_origin_x + 12, 55, line);

    snprintf(line, sizeof(line), "Grille : %dx%d", BOARD_WIDTH, BOARD_HEIGHT);
    draw_text_line(display, window, gc, panel_origin_x + 12, 80, line);

    for (x = 0; x < player_count; x++)
    {
        int line_y = 130 + ((int)x * 120);

        XSetForeground(display, gc, renderer->colors[x]);
        XFillRectangle(display, window, gc, panel_origin_x + 12, line_y - 12, 18, 18);

        XSetForeground(display, gc, renderer->text_color);
        snprintf(line, sizeof(line), "Joueur %u : %s", x + 1u, players[x].name);
        draw_text_line(display, window, gc, panel_origin_x + 40, line_y, line);

        snprintf(line, sizeof(line), "Position : (%d, %d)", players[x].x, players[x].y);
        draw_text_line(display, window, gc, panel_origin_x + 12, line_y + 24, line);

        snprintf(line, sizeof(line), "Credits  : %d", players[x].credits);
        draw_text_line(display, window, gc, panel_origin_x + 12, line_y + 48, line);

        snprintf(line, sizeof(line), "Cases    : %d", board_get_score(board, (int)x));
        draw_text_line(display, window, gc, panel_origin_x + 12, line_y + 72, line);

        snprintf(line, sizeof(line), "Actif    : %s", players[x].active ? "oui" : "non");
        draw_text_line(display, window, gc, panel_origin_x + 12, line_y + 96, line);
    }

    XFlush(display);
}
