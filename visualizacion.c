#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "visualizacion.h"

/*
E: paso actual, nodo procesado, arreglos de valores y visitados, numero de nodos.
S: imprime el estado actual de las distancias y visitados en Dijkstra.
R: arreglos validos de tamano n.
*/
void print_estado_dijkstra(int paso, int actual, const int *val, const int *visitado, int n) {
    printf("\nPaso %d: procesamos el nodo %d\n", paso, actual);
    printf("Distancias despues de relajar vecinos:\n");

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
    char display[MAX_ROWS][MAX_COLS + 1];
    for (int r = 0; r < maze->rows; ++r) {
        strcpy(display[r], maze->cells[r]);
    }

    //recorre desde goal hacia start usando parent para marcar el camino.
    int v = goal;
    while (v != -1) {
        struct Point p = graph->indexToCoord[v];
        char *cell = &display[p.row][p.col];
        if (v == start) {
            *cell = START;
        } else if (v == goal) {
            *cell = END;
        } else {
            *cell = 'o';
        }
        v = parent[v];
    }

    printf("Camino encontrado (marcado con 'o'):\n");
    for (int r = 0; r < maze->rows; ++r) {
        printf("%s\n", display[r]);
    }
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
