/*
E: n/a (implementacion).
S: algoritmo BFS para grafos con matriz 0/1 de adyacencia.
R: usar con bfs.h y grafo.h.
.*/
#include <stdio.h>
#include <stdlib.h>

#include "bfs.h"

/*
E: grafo, nodo inicio y meta, arreglos parent/visitOrder y contador.
S: ejecuta BFS, llena parent y orden visitado, retorna 1 si encontro goal.
R: arreglos de tamano vertices; indices validos; memoria disponible.
.*/
int bfs(const struct Grafo *graph, int start, int goal, int *parent, int *visitOrder, int *visitCount) {
    int *visited = (int *)calloc(graph->vertices, sizeof(int));
    int *queue = (int *)calloc(graph->vertices, sizeof(int));
    int head = 0;
    int tail = 0;

    if (visited == NULL || queue == NULL) {
        printf("No se pudo reservar memoria para BFS.\n");
        free(visited);
        free(queue);
        return 0;
    }

    // Inicializa padres en -1 y arranca la cola con el nodo de inicio.
    for (int i = 0; i < graph->vertices; ++i) {
        parent[i] = -1;
    }

    queue[tail++] = start;
    visited[start] = 1;
    *visitCount = 0;

    while (head < tail) {
        int v = queue[head++];
        visitOrder[(*visitCount)++] = v;

        if (v == goal) {
            free(visited);
            free(queue);
            return 1;
        }

        // Explora vecinos marcados con 1 en la fila v.
        for (int u = 0; u < graph->vertices; ++u) {
            if (graph->peso[v][u] > 0 && !visited[u]) {
                visited[u] = 1;
                parent[u] = v;
                queue[tail++] = u;
            }
        }
    }

    free(visited);
    free(queue);
    return 0;
}
