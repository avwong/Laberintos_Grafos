/*
E: n/a (implementacion).
S: utilidades para cargar laberintos desde archivo.
R: usar con laberinto.h.
.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "laberinto.h"

/*
E: cadena con posible salto de linea.
S: elimina '\n' o '\r' al final de la cadena.
R: puntero no nulo a un buffer modificable.
.*/
void trim_newline(char *s) {
    if (s == NULL) {
        return;
    }
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

/*
E: ruta de archivo de laberinto y puntero Maze.
S: carga celdas, filas y columnas; retorna 0 si OK, -1 si error.
R: archivo legible, filas con la misma longitud, maximo 128x128.
.*/
int load_maze(const char *filename, Maze *maze) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("No se pudo abrir el archivo");
        return -1;
    }

    char buffer[MAX_COLS + 4];
    int row = 0;
    int expectedCols = -1;

    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        trim_newline(buffer);
        size_t len = strlen(buffer);
        if (len == 0) {
            continue; // ignora lineas vacias
        }
        if ((int)len > MAX_COLS) {
            printf("Linea %d excede el maximo de columnas (%d)\n", row + 1, MAX_COLS);
            fclose(f);
            return -1;
        }
        if (expectedCols == -1) {
            expectedCols = (int)len;
        } else if ((int)len != expectedCols) {
            printf("Las lineas deben tener la misma longitud. Linea %d tiene %zu en lugar de %d.\n", row + 1, len, expectedCols);
            fclose(f);
            return -1;
        }
        if (row >= MAX_ROWS) {
            printf("Numero de filas excede el maximo permitido (%d).\n", MAX_ROWS);
            fclose(f);
            return -1;
        }
        strcpy(maze->cells[row], buffer);
        row++;
    }

    fclose(f);

    if (row == 0) {
        printf("El archivo esta vacio o no tiene filas validas.\n");
        return -1;
    }

    maze->rows = row;
    maze->cols = expectedCols;
    return 0;
}
