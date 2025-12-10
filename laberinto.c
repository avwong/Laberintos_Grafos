#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "laberinto.h"

/*
E: cadena de caracteres
S: elimina el salto de linea al final de la cadena
R: puntero no nulo
*/
void trim_newline(char *s) {
    //validar que el puntero no sea NULL
    if (s == NULL) {
        return;
    }
    
    //obtener la longitud actual de la cadena
    size_t len = strlen(s);
    
    //eliminar caracteres de nueva linea del final
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0'; //reemplazar el caracter con terminador nulo
        len--; //reducir la longitud
    }
}

/*
E: ruta de archivo de laberinto y puntero Maze.
S: carga celdas, filas y columnas; retorna 0 si OK, -1 si error.
R: archivo legible, filas con la misma longitud, maximo 128x128.
.*/
int load_maze(const char *filename, struct Maze *maze) {
    //intentar abrir el archivo en modo lectura
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("No se pudo abrir el archivo");
        return -1;
    }

    //buffer temporal para leer cada linea del archivo
    char buffer[MAX_COLS + 4];
    
    //contador de filas procesadas
    int row = 0;
    
    //almacena el numero esperado de columnas
    //esto asegura que todas las filas tengan la misma longitud
    int expectedCols = -1;

    //leer el archivo linea por linea
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        //eliminar el caracter de nueva linea al final
        trim_newline(buffer);
        
        //obtener la longitud de la linea leida
        size_t len = strlen(buffer);
        
        //ignorar lineas vacias (pueden aparecer en el archivo)
        if (len == 0) {
            continue;
        }
        
        //validar que la linea no exceda el maximo de columnas permitido
        if ((int)len > MAX_COLS) {
            printf("Linea %d excede el maximo de columnas (%d)\n", row + 1, MAX_COLS);
            fclose(f);
            return -1;
        }
        
        //en la primera fila valida, establecer el numero esperado de columnas
        if (expectedCols == -1) {
            expectedCols = (int)len;
        } 
        //en filas subsecuentes, verificar que tengan la misma longitud
        else if ((int)len != expectedCols) {
            printf("Las lineas deben tener la misma longitud. Linea %d tiene %zu en lugar de %d.\n", row + 1, len, expectedCols);
            fclose(f);
            return -1;
        }
        
        //validar que no se exceda el maximo de filas
        if (row >= MAX_ROWS) {
            printf("Numero de filas excede el maximo permitido (%d).\n", MAX_ROWS);
            fclose(f);
            return -1;
        }
        
        //copiar la linea al arreglo de celdas del laberinto
        strcpy(maze->cells[row], buffer);
        row++;
    }

    //cerrar el archivo
    fclose(f);

    //validar que se haya leido al menos una fila
    if (row == 0) {
        printf("El archivo esta vacio o no tiene filas validas.\n");
        return -1;
    }

    //establecer las dimensiones finales del laberinto
    maze->rows = row;
    maze->cols = expectedCols;
    
    return 0; //exito
}
