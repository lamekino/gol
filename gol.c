#define _XOPEN_SOURCE 500
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SECOND 1000000
#define FPS    15

#define HEIGHT 32
#define WIDTH  HEIGHT * 2
#define BOUND_CHECK(c) \
        c.x >= 0 && c.x < HEIGHT && \
        c.y >= 0 && c.y < WIDTH

#define EMPTY '.'
#define CELL  '#'

char grid[HEIGHT][WIDTH];

typedef struct coord {
    int x;
    int y;
} Coord;

void print_grid() {
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            printf("%c", grid[x][y]);
        }
        puts("");
    }
}

void fill_grid(char ch) {
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            grid[x][y] = ch;
        }
    }
}

void place_cell(Coord c) {
    assert(BOUND_CHECK(c));

    grid[c.x][c.y] = CELL;
}

void clear_cell(Coord c) {
    assert(BOUND_CHECK(c));

    grid[c.x][c.y] = EMPTY;
}

int is_alive(Coord c) {
    return grid[c.x][c.y] == CELL;
}

int neighbors(Coord c) {
    assert(BOUND_CHECK(c));

    int sum = 0;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            if (x != 1 || y != 1) {
                Coord d = {
                    c.x + (1 - x % 3),
                    c.y + (1 - y % 3),
                };


                if (BOUND_CHECK(d) && grid[d.x][d.y] == CELL)
                    sum++;
            }
        }
    }

    return sum;
}

typedef struct task {
    enum {
        KILL,
        BIRTH,
    } type;
    Coord coords;
} Task;

void process_task(Task t) {
    if (t.type == KILL) {
        clear_cell(t.coords);
    }
    else if (t.type == BIRTH) {
        place_cell(t.coords);
    }
}

typedef struct stack {
    Task *head;
    Task *tasks;
    size_t size;
} TaskStack;

void push(TaskStack *s, const Task t) {
    s->size++;
    s->head = realloc(s->head, sizeof(Task) * s->size);
    
    memcpy(&s->head[s->size - 1], &t, sizeof(Task));
}

Task pop(TaskStack *s) {
    Task t = s->head[s->size - 1];

    s->size--;
    s->head = realloc(s->head, sizeof(Task) * s->size);

    return t;
}

int update() {
    TaskStack *stack = calloc(1, sizeof(TaskStack));

    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            Task t;
            t.coords = (Coord) { .x = x, .y = y };
            int num_neighbors = neighbors(t.coords);
            
            // dead state
            if (num_neighbors < 2 || num_neighbors > 3) {
                if (is_alive(t.coords)) {
                    t.type = KILL;
                    push(stack, t);
                }
            }
            // reproduction state
            else if (num_neighbors == 3) {
                if (!is_alive(t.coords)) {
                    t.type = BIRTH;
                    push(stack, t);
                }
            }
        }
    }

    // clear task stack, for each action type
    // the cell at t.coords gets either cleared
    // or placed
    int num_updates = stack->size;
    while (stack->size) {
        process_task(pop(stack));
    }

    free(stack);
    return num_updates;
}

void reset_cursor() {
    printf("\x1b[%dD", WIDTH);
    printf("\x1b[%dA", HEIGHT);
}

void read_file(FILE *fd) {
    int chars = 0;
    int lines = 0;
    char ch;
    while ((ch = fgetc(fd)) != EOF) {
        if (ch != '\n') {
            assert(ch == EMPTY || ch == CELL);
            chars++;
            assert(chars <= WIDTH);
            grid[lines][chars] = ch;
        }
        else {
            chars = 0;
            lines++;
            assert(lines <= HEIGHT);
        }
    }
}

int main(int argc, char **argv) {
    fill_grid(EMPTY);

    FILE *fd = stdin;
    if (argc > 1) {
        fd = fopen(argv[1], "r");
    }
    read_file(fd);
    fclose(fd);

    const unsigned int delay = SECOND / FPS;
    print_grid();
    while (update()) {
        reset_cursor();
        print_grid();
        usleep(delay);
    }

    return 0;
}
