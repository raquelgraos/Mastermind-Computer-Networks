CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

TARGET = player
TARGET2 = GS

SRCS = client/player_main.c client/parser.c client/handler.c client/connections.c
SRCS2 = server/gs_main.c server/connections.c server/parser.c server/handler.c

OBJS = $(SRCS:.c=.o)
OBJS2 = $(SRCS2:.c=.o)

all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2)	

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(OBJS2) $(TARGET2)
	rm -rf server/GAMES server/SCORES client/SAVED