CC = gcc
CCFLAGS = -Wall -Wextra -Werror -std=c99
SRC = gol.c
OBJ = $(SRC:.c=.o)

all: gol

gol: $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

gol.o: gol.c

clean:
	rm *.o gol

.PHONY: all clean
