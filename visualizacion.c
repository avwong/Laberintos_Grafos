#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "visualizacion.h"

/*
E: arreglo parent, indices start/goal, numero de vertices y arreglo de salida.
S: llena out con la secuencia start->goal y retorna su longitud; -1 si no hay ruta.
R: parent describe padres validos, tiene espacio para 'vertices' elementos.
.*/
int build_path_sequence(const int *parent, int start, int goal, int vertices, int *out) {
    if (parent == NULL || out == NULL || vertices <= 0 || start < 0 || goal < 0 || start >= vertices || goal >= vertices) {
        return -1;
    }

    int len = 0;
    int v = goal;

    //recorrer hacia atras usando parent hasta llegar a start
    while (v != -1 && len < vertices) {
        out[len++] = v;
        if (v == start) {
            break;
        }
        v = parent[v];
    }

    //si no se llego a start, el camino es invalido
    if (v == -1) {
        return -1;
    }

    //invertir el arreglo para tener orden start -> goal
    for (int i = 0; i < len / 2; ++i) {
        int tmp = out[i];
        out[i] = out[len - 1 - i];
        out[len - 1 - i] = tmp;
    }

    return len;
}

/*
E: paso actual, nodo procesado, arreglos de valores y visitados, numero de nodos.
S: imprime el estado actual de las distancias y visitados en Dijkstra.
R: arreglos validos de tamano n.
*/
void print_estado_dijkstra(int paso, int actual, const int *val, const int *visitado, int n) {
    printf("\nPaso %d: procesamos el nodo %d\n", paso, actual);

    for (int i = 0; i < n; ++i) {
        char dist[16];
        if (val[i] >= INT_MAX / 8) {
            snprintf(dist, sizeof(dist), "inf");
        } else {
            snprintf(dist, sizeof(dist), "%d", val[i]);
        }

        const char* estado;
        if (visitado[i]) {
            estado = "visitado";
        } else if (val[i] >= INT_MAX / 8) {
            estado = "sin ruta";
        } else {
            estado = "pendiente";
        }

        const char* marca = (i == actual) ? " <- actual" : "";
        printf("  %2d) dist=%-7s estado=%s%s\n", i, dist, estado, marca);
    }
}

/*
E: grafo y arreglo de visita BFS con su cantidad.
S: imprime numero de nodo y coordenadas visitadas.
R: indices validos dentro de graph->vertices.
.*/
void print_visit_order(const struct Grafo *graph, const int *visitOrder, int visitCount) {
    printf("Orden de visita (BFS):\n");
    for (int i = 0; i < visitCount; ++i) {
        struct Point p = graph->indexToCoord[visitOrder[i]];
        printf("%3d) nodo %d -> (%d, %d)\n", i + 1, visitOrder[i], p.row, p.col);
    }
}

/*
E: arreglo de visita y cantidad.
S: imprime orden de visita solo con indices.
R: indices validos.
.*/
void print_visit_order_simple(const int *visitOrder, int visitCount) {
    printf("Orden de visita (BFS):\n");
    for (int i = 0; i < visitCount; ++i) {
        printf("%3d) nodo %d\n", i + 1, visitOrder[i]);
    }
}

/*
E: laberinto, grafo, arreglo parent y nodos inicio/goal.
S: imprime laberinto con camino marcado con 'o'.
R: parent describe una ruta valida entre goal y start.
.*/
void print_path_on_maze(const struct Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal) {
    int *path = calloc(graph->vertices, sizeof(int));
    if (path == NULL) {
        printf("No se pudo reservar memoria para mostrar el laberinto.\n");
        return;
    }

    int len = build_path_sequence(parent, start, goal, graph->vertices, path);
    if (len <= 0) {
        printf("No hay camino entre I y F.\n");
        free(path);
        return;
    }

    //expandir el camino para incluir celdas intermedias
    struct Point *expandedPath = calloc(MAX_ROWS * MAX_COLS, sizeof(struct Point));
    if (expandedPath == NULL) {
        printf("No se pudo reservar memoria para expandir el camino.\n");
        free(path);
        return;
    }

    int expandedLen = expand_path_with_intermediate_cells(maze, graph, path, len, expandedPath);

    char display[MAX_ROWS][MAX_COLS + 1];
    for (int r = 0; r < maze->rows; ++r) {
        strcpy(display[r], maze->cells[r]);
    }

    //marcar todas las celdas del camino expandido con 'o', preservando I y F
    for (int i = 0; i < expandedLen; ++i) {
        struct Point p = expandedPath[i];

        //validar coordenadas
        if (p.row < 0 || p.row >= maze->rows || p.col < 0 || p.col >= maze->cols) {
            continue;
        }

        char *cell = &display[p.row][p.col];

        //preservar I y F, marcar todo lo demas con 'o'
        if (maze->cells[p.row][p.col] != START && maze->cells[p.row][p.col] != END) {
            *cell = 'o';
        }
    }

    printf("\n=== Camino final (marcado con 'o') ===\n");
    for (int r = 0; r < maze->rows; ++r) {
        printf("%s\n", display[r]);
    }
    printf("\n");

    free(path);
    free(expandedPath);
}

/*
E: laberinto, grafo, camino de nodos y su longitud, arreglo de salida.
S: expande el camino incluyendo todas las celdas intermedias entre nodos; retorna longitud del camino expandido.
R: path valido con pathLen nodos; expandedPath tiene espacio suficiente.
*/
int expand_path_with_intermediate_cells(const struct Maze *maze, const struct Grafo *graph, const int *path, int pathLen, struct Point *expandedPath) {
    if (path == NULL || expandedPath == NULL || pathLen <= 0) {
        return 0;
    }

    int expandedLen = 0;

    //agregar el primer nodo
    struct Point firstPoint = graph->indexToCoord[path[0]];

    //validar que el primer punto este dentro de los limites
    if (firstPoint.row < 0 || firstPoint.row >= maze->rows ||
        firstPoint.col < 0 || firstPoint.col >= maze->cols) {
        return 0;
    }

    expandedPath[expandedLen++] = firstPoint;

    //para cada par de nodos consecutivos, agregar las celdas intermedias
    for (int i = 1; i < pathLen; ++i) {
        struct Point from = graph->indexToCoord[path[i - 1]];
        struct Point to = graph->indexToCoord[path[i]];

        //validar que ambos puntos esten dentro de los limites
        if (from.row < 0 || from.row >= maze->rows || from.col < 0 || from.col >= maze->cols ||
            to.row < 0 || to.row >= maze->rows || to.col < 0 || to.col >= maze->cols) {
            continue;
        }

        //determinar la direccion del movimiento
        int rowDiff = to.row - from.row;
        int colDiff = to.col - from.col;

        //verificar que los nodos sean adyacentes (distancia Manhattan de 2)
        //en el laberinto visual, nodos adyacentes estan separados por 2 celdas
        int distance = abs(rowDiff) + abs(colDiff);

        if (distance == 0) {
            //mismo nodo, continuar
            continue;
        }

        if (distance != 2) {
            //nodos no adyacentes, algo esta mal
            //intentar agregar solo el nodo destino
            expandedPath[expandedLen++] = to;
            continue;
        }

        //normalizar la direccion (solo movimientos horizontales o verticales)
        int rowStep = (rowDiff > 0) ? 1 : (rowDiff < 0) ? -1 : 0;
        int colStep = (colDiff > 0) ? 1 : (colDiff < 0) ? -1 : 0;

        //agregar la celda intermedia entre los dos nodos
        int currentRow = from.row + rowStep;
        int currentCol = from.col + colStep;

        //verificar que la celda intermedia este dentro de los limites y no sea pared
        if (currentRow >= 0 && currentRow < maze->rows &&
            currentCol >= 0 && currentCol < maze->cols &&
            maze->cells[currentRow][currentCol] != WALL) {
            expandedPath[expandedLen].row = currentRow;
            expandedPath[expandedLen].col = currentCol;
            expandedLen++;
        }

        //agregar el nodo destino
        expandedPath[expandedLen++] = to;
    }

    return expandedLen;
}

/*
E: laberinto, grafo, arreglo parent y nodos start/goal.
S: imprime frame a frame el recorrido usando 'A' para la posicion actual y 'o' para visitados.
R: parent describe una ruta valida; el grafo tiene mapeo indexToCoord; buffers dentro de limites.
.*/
void print_path_steps(const struct Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal) {
    int *path = calloc(graph->vertices, sizeof(int));
    if (path == NULL) {
        printf("No se pudo reservar memoria para animar el recorrido.\n");
        return;
    }

    int len = build_path_sequence(parent, start, goal, graph->vertices, path);
    if (len <= 0) {
        printf("No hay camino entre I y F.\n");
        free(path);
        return;
    }

    //expandir el camino para incluir todas las celdas intermedias
    struct Point *expandedPath = calloc(MAX_ROWS * MAX_COLS, sizeof(struct Point));
    if (expandedPath == NULL) {
        printf("No se pudo reservar memoria para expandir el camino.\n");
        free(path);
        return;
    }

    int expandedLen = expand_path_with_intermediate_cells(maze, graph, path, len, expandedPath);
    if (expandedLen <= 0) {
        printf("Error: No se pudo expandir el camino.\n");
        printf("Longitud del camino original: %d nodos\n", len);
        free(path);
        free(expandedPath);
        return;
    }

    printf("\n=== Recorrido paso a paso (A = posicion actual) ===\n");
    for (int step = 0; step < expandedLen; ++step) {
        char frame[MAX_ROWS][MAX_COLS + 1];
        for (int r = 0; r < maze->rows; ++r) {
            strcpy(frame[r], maze->cells[r]);
        }

        //marcar el progreso: 'o' para lo recorrido, 'A' para la celda actual
        for (int k = 0; k <= step; ++k) {
            struct Point p = expandedPath[k];

            //validar que las coordenadas esten dentro de los limites
            if (p.row < 0 || p.row >= maze->rows || p.col < 0 || p.col >= maze->cols) {
                continue;
            }

            char *cell = &frame[p.row][p.col];

            //verificar que la celda no sea una pared antes de modificarla
            if (maze->cells[p.row][p.col] == WALL) {
                continue;
            }

            if (k == step) {
                //mostrar la posicion actual con 'A'
                *cell = 'A';
            } else {
                //marcar las posiciones ya visitadas con 'o', preservando I y F
                if (maze->cells[p.row][p.col] != START && maze->cells[p.row][p.col] != END) {
                    *cell = 'o';
                }
            }
        }

        printf("Paso %d de %d:\n", step + 1, expandedLen);
        for (int r = 0; r < maze->rows; ++r) {
            printf("%s\n", frame[r]);
        }
        printf("\n");
    }

    printf("=== Animacion completada: %d pasos totales ===\n\n", expandedLen);

    free(path);
    free(expandedPath);
}

/*
E: arreglo parent, indices inicio/goal y numero de vertices.
S: imprime camino por indices o indica si no existe.
R: parent de tamano vertices; indices dentro de rango.
*/
void print_path_indices(const int *parent, int start, int goal, int vertices) {
    int *stack = calloc(vertices, sizeof(int));
    if (stack == NULL) {
        printf("No se pudo reservar memoria para mostrar el camino.\n");
        return;
    }

    //apila el camino desde la meta hacia el inicio para invertirlo.
    int len = 0;
    int v = goal;
    while (v != -1 && len < vertices) {
        stack[len++] = v;
        if (v == start) {
            break;
        }
        v = parent[v];
    }

    if (v == -1) {
        printf("No hay camino entre %d y %d.\n", start, goal);
        free(stack);
        return;
    }

    printf("Camino encontrado:\n");
    for (int i = len - 1; i >= 0; --i) {
        printf("%d", stack[i]);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\n");
    free(stack);
}

/*
E: grafo con matriz de adyacencia.
S: imprime matriz completa o primeras 30 filas/cols.
R: matriz cuadrada de tamano vertices.
.*/
void print_adjacency_matrix(const struct Grafo *graph) {
    int limit = graph->vertices;
    if (limit > 30) {
        printf("Matriz de adyacencia parcial: mostrando primeras %d filas/cols de %d nodos.\n", 30, graph->vertices);
        limit = 30;
    } else {
        printf("Matriz de adyacencia completa (%d nodos):\n", graph->vertices);
    }

    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < limit; ++j) {
            int val = graph->peso[i][j] > 0 ? 1 : 0;
            printf("%d ", val);
        }
        printf("\n");
    }
}

/*
E: puntero a Camino valido.
S: imprime nodos en orden y valor total; indica si es NULL.
R: camino no nulo; longitud coherente con arreglo nodos.
*/
void imprimirCaminoDijkstra(struct Camino* camino) {
    //validaciones
    if (camino == NULL) {
        printf("No hay camino calculado.\n");
        return;
    }

    if (camino->nodos == NULL || camino->longitud <= 0) {
        printf("Error: camino invalido (nodos NULL o longitud <= 0).\n");
        return;
    }
    
    //imprimir el valor total del camino
    printf("Camino (valor %d): ", camino->valorTotal);
    
    //imprimir cada nodo del camino
    for (int i = 0; i < camino->longitud; ++i) {
        printf("%d", camino->nodos[i]);
        
        //imprimir flecha si no es el ultimo nodo
        if (i + 1 < camino->longitud) {
            printf(" -> ");
        }
    }
    printf("\n");
}
