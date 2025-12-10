/*
E: n/a (archivo de cabecera).
S: define constantes y estructuras para manejar laberintos.
R: usar junto a laberinto.c para funciones de carga.
.*/
#ifndef LABERINTO_H
#define LABERINTO_H

#include <stddef.h>

#define MAX_ROWS 128
#define MAX_COLS 128
#define WALL '#'
#define START 'S'
#define END 'E'

typedef struct Point {
    int row;
    int col;
} Point;

typedef struct Maze {
    int rows;
    int cols;
    char cells[MAX_ROWS][MAX_COLS + 1];
} Maze;

/*
E: cadena con posible salto de linea.
S: elimina '\n' o '\r' al final de la cadena.
R: puntero no nulo a un buffer modificable.
.*/
void trim_newline(char *s);

/*
E: ruta de archivo de laberinto y puntero Maze.
S: carga celdas, filas y columnas; retorna 0 si OK, -1 si error.
R: archivo legible, filas con la misma longitud, maximo 128x128.
.*/
int load_maze(const char *filename, Maze *maze);

#endif
