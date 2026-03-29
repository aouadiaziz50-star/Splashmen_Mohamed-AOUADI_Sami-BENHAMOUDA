CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c11
LDFLAGS=-ldl -lX11

OBJS=splash.o game.o board.o player.o renderer_x11.o
PLAYER_LIBS=p1_square.so p2_snake.so p3_spiral.so p4_dash.so

all: splash players

splash: $(OBJS)
	$(CC) $(CFLAGS) -o splash $(OBJS) $(LDFLAGS)

players: $(PLAYER_LIBS)

splash.o: splash.c game.h
	$(CC) $(CFLAGS) -c splash.c

game.o: game.c game.h board.h player.h renderer.h actions.h
	$(CC) $(CFLAGS) -c game.c

board.o: board.c board.h
	$(CC) $(CFLAGS) -c board.c

player.o: player.c player.h
	$(CC) $(CFLAGS) -c player.c

renderer_x11.o: renderer_x11.c renderer.h board.h
	$(CC) $(CFLAGS) -c renderer_x11.c

p1_square.so: p1_square.c actions.h
	$(CC) $(CFLAGS) -fPIC -shared -o p1_square.so p1_square.c

p2_snake.so: p2_snake.c actions.h
	$(CC) $(CFLAGS) -fPIC -shared -o p2_snake.so p2_snake.c

p3_spiral.so: p3_spiral.c actions.h
	$(CC) $(CFLAGS) -fPIC -shared -o p3_spiral.so p3_spiral.c

p4_dash.so: p4_dash.c actions.h
	$(CC) $(CFLAGS) -fPIC -shared -o p4_dash.so p4_dash.c

run: all
	./splash --delay 1 --render-every 32 ./p1_square.so ./p2_snake.so ./p3_spiral.so ./p4_dash.so

run-headless: all
	./splash --headless ./p1_square.so ./p2_snake.so ./p3_spiral.so ./p4_dash.so

clean:
	rm -f *.o splash *.so
