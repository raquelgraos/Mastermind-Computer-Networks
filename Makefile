CC = gcc

CFLAGS = -Wall -Wextra -std=c11

TARGET = player

SRCS = client/player_main.c client/parser.c client/handler.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
