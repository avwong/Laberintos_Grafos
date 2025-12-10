#include <stdio.h>
#include <stdlib.h>

#include "bfs.h"

//algoritmo BFS para grafos con matriz 0/1 de adyacencia

/*
E: grafo, nodo inicio y meta, arreglos parent/visitOrder y contador.
S: ejecuta BFS, llena parent y orden visitado, retorna 1 si encontro goal.
R: arreglos de tamano vertices; indices validos; memoria disponible.
*/
int bfs(const struct Grafo *graph, int start, int goal, int *parent, int *visitOrder, int *visitCount) {
    //reservar memoria para el arreglo de visitados (1 = visitado, 0 = no visitado)
    int *visited = calloc(graph->vertices, sizeof(int));
    
    //reservar memoria para la cola
    int *queue = calloc(graph->vertices, sizeof(int));
    
    //indices para controlar la cola
    int head = 0;
    int tail = 0;

    //verificar que la asignacion de memoria fue exitosa
    if (visited == NULL || queue == NULL) {
        printf("No se pudo reservar memoria para BFS.\n");
        free(visited);
        free(queue);
        return 0;
    }

    //inicializar el arreglo de padres en -1 (indica que no tienen padre)
    for (int i = 0; i < graph->vertices; ++i) {
        parent[i] = -1;
    }

    //agregar el nodo de inicio a la cola
    queue[tail++] = start;
    
    //marcar el nodo de inicio como visitado
    visited[start] = 1;
    
    //inicializar el contador de nodos visitados
    *visitCount = 0;

    //mientras haya elementos en la cola
    while (head < tail) {
        //sacar el primer elemento de la cola (FIFO: First In First Out)
        int v = queue[head++];
        
        //registrar este nodo en el orden de visita
        visitOrder[(*visitCount)++] = v;

        //si encontramos el nodo meta, terminar
        if (v == goal) {
            free(visited);
            free(queue);
            return 1; //exito
        }

        //explorar todos los vecinos del nodo actual
        //recorrer la fila v de la matriz de pesos
        for (int u = 0; u < graph->vertices; ++u) {
            //si hay una arista de v a u (peso > 0) y u no ha sido visitado
            if (graph->peso[v][u] > 0 && !visited[u]) {
                //marcar el vecino como visitado
                visited[u] = 1;
                
                //registrar que llegamos a u desde v (para reconstruir el camino)
                parent[u] = v;
                
                //agregar el vecino a la cola para explorarlo despues
                queue[tail++] = u;
            }
        }
    }

    //si llegamos aqui, no se encontro un camino al nodo meta
    free(visited);
    free(queue);
    return 0; //no se encontro camino
}
