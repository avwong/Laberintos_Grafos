#ifndef LABERINTO_H
#define LABERINTO_H

#include <stddef.h>

#define MAX_ROWS 128
#define MAX_COLS 128
#define WALL 'X'
#define START 'I'
#define END 'F'

struct Point {
    int row;
    int col;
} ;

struct Maze {
    int rows;
    int cols;
    char cells[MAX_ROWS][MAX_COLS + 1];
} ;

void trim_newline(char *s);
int load_maze(const char *filename, struct Maze *maze);

#endif
