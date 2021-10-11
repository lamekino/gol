#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define HEIGHT 32
#define WIDTH  HEIGHT * 2
#define EMPTY '.'
#define CELL  '#'
#define BOUND_CHECK(c) \
        c.x >= 0 && c.x < HEIGHT && \
        c.y >= 0 && c.y < WIDTH

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
    Coord coord;
} Task;

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
            Coord cs = { .x = x, .y = y };
            int num_neighbors = neighbors(cs);
            
            // dead state
            if (num_neighbors < 2 || num_neighbors > 3) {
                if (is_alive(cs)) {
                    Task t = { .coord = cs, .type = KILL };
                    push(stack, t);
                }
            }
            // reproduction state
            else if (num_neighbors == 3) {
                if (!is_alive(cs)) {
                    Task t = { .coord = cs, .type = BIRTH };
                    push(stack, t);
                }
            }
        }
    }

    // clear task stack, for each action type
    // the cell at t.coord gets either cleared
    // or placed
    int num_updates = stack->size;
    while (stack->size) {
        Task t = pop(stack);
        if (t.type == KILL) {
            clear_cell(t.coord);
        }
        else if (t.type == BIRTH) {
            place_cell(t.coord);
        }
    }

    free(stack);
    return num_updates;
}

void reset_cursor() {
    printf("\x1b[%dD", WIDTH);
    printf("\x1b[%dA", HEIGHT);
}

int main() {
    fill_grid(EMPTY);

    Coord cs[] = {
        { .x = 5, .y = 5 },
        { .x = 6, .y = 6 },
        { .x = 6, .y = 7 },
        { .x = 7, .y = 5 },
        { .x = 7, .y = 6 },
    };

    for (int i = 0; i < sizeof(cs) / sizeof(Coord); i++) {
        place_cell(cs[i]);
    }

    print_grid();
    while (update()) {
        reset_cursor();
        print_grid();
        usleep(100000);
    }

    return 0;
}
