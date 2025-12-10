#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

/* E: cantidad de vertices (>0). 
 * S: puntero a Grafo con matriz de pesos inicializada en 0; NULL en error. 
 * R: memoria disponible; vertices positivo. */
Grafo* crearGrafo(int vertices) {
    if (vertices <= 0) {
        return NULL;
    }

    Grafo* grafo = (Grafo*)calloc(1, sizeof(Grafo));
    if (grafo == NULL) {
        return NULL;
    }

    grafo->vertices = vertices;
    grafo->peso = (int**)calloc(vertices, sizeof(int*));
    if (grafo->peso == NULL) {
        free(grafo);
        return NULL;
    }

    for (int i = 0; i < vertices; ++i) {
        grafo->peso[i] = (int*)calloc(vertices, sizeof(int));
        if (grafo->peso[i] == NULL) {
            for (int j = 0; j < i; ++j) {
                free(grafo->peso[j]);
            }
            free(grafo->peso);
            free(grafo);
            return NULL;
        }
    }

    return grafo;
}

/* E: puntero a grafo, indices origen/destino y peso (>=0). 
 * S: asigna peso simetrico; retorna 0 si OK, -1 si indices fuera de rango. 
 * R: grafo valido; indices dentro de [0, vertices-1]. */
int asignarArista(Grafo* grafo, int origen, int destino, int peso) {
    if (grafo == NULL || origen < 0 || destino < 0 || origen >= grafo->vertices || destino >= grafo->vertices) {
        return -1;
    }
    if (peso < 0) {
        return -1;
    }
    grafo->peso[origen][destino] = peso;
    grafo->peso[destino][origen] = peso; // grafo no dirigido
    return 0;
}

/* E: puntero a grafo previamente creado. 
 * S: libera memoria interna y el grafo. 
 * R: no usar el puntero despues; grafo puede ser NULL. */
void liberarGrafo(Grafo* grafo) {
    if (grafo == NULL) {
        return;
    }
    if (grafo->peso != NULL) {
        for (int i = 0; i < grafo->vertices; ++i) {
            free(grafo->peso[i]);
        }
        free(grafo->peso);
    }
    free(grafo);
}
