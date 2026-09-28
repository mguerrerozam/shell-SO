CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -Iinclude

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = mishell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean