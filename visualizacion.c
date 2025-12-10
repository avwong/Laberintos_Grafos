/*
E: n/a (implementacion).
S: utilidades de impresion para recorridos y caminos en el laberinto/grafo.
R: usar con visualizacion.h.
.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visualizacion.h"

/*
E: grafo y arreglo de visita BFS con su cantidad.
S: imprime numero de nodo y coordenadas visitadas.
R: indices validos dentro de graph->vertices.
.*/
void print_visit_order(const struct Grafo *graph, const int *visitOrder, int visitCount) {
    printf("Orden de visita (BFS):\n");
    for (int i = 0; i < visitCount; ++i) {
        Point p = graph->indexToCoord[visitOrder[i]];
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
void print_path_on_maze(const Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal) {
    char display[MAX_ROWS][MAX_COLS + 1];
    for (int r = 0; r < maze->rows; ++r) {
        strcpy(display[r], maze->cells[r]);
    }

    // Recorre desde goal hacia start usando parent para marcar el camino.
    int v = goal;
    while (v != -1) {
        Point p = graph->indexToCoord[v];
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
.*/
void print_path_indices(const int *parent, int start, int goal, int vertices) {
    int *stack = (int *)calloc(vertices, sizeof(int));
    if (stack == NULL) {
        printf("No se pudo reservar memoria para mostrar el camino.\n");
        return;
    }

    // Apila el camino desde la meta hacia el inicio para invertirlo.
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
