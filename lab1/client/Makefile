CC = gcc
CFLAGS = -std=c11 -Wall -Wextra

TARGET = client

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)