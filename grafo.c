#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

/*
E: cantidad de vertices mayor a 0
S: puntero a Grafo con matriz de pesos inicializada en 0 o NULL en error
R: memoria disponible; vertices positivo
*/
struct Grafo* crearGrafo(int vertices) {
    //validar que la cantidad de vertices sea positiva
    if (vertices <= 0) {
        return NULL;
    }

    //reservar memoria para la estructura principal del grafo
    struct Grafo* grafo = calloc(1, sizeof(struct Grafo));
    if (grafo == NULL) {
        return NULL;
    }

    //establecer el numero de vertices
    grafo->vertices = vertices;
    
    //reservar memoria para el arreglo de punteros (filas de la matriz)
    grafo->peso = calloc(vertices, sizeof(int*));
    if (grafo->peso == NULL) {
        free(grafo);
        return NULL;
    }

    //reservar memoria para cada fila de la matriz de pesos
    for (int i = 0; i < vertices; ++i) {
        grafo->peso[i] = calloc(vertices, sizeof(int));
        
        //si falla la asignacion, liberar toda la memoria ya reservada
        if (grafo->peso[i] == NULL) {
            //liberar las filas previamente asignadas
            for (int j = 0; j < i; ++j) {
                free(grafo->peso[j]);
            }
            free(grafo->peso);
            free(grafo);
            return NULL;
        }
    }

    //0 indica que no hay arista entre vertices
    return grafo;
}

/*
E: puntero a grafo, indices origen/destino y peso mayor a 0
S: asigna peso simetrico, retorna 0 si esta bien, -1 si indices fuera de rango
R: grafo valido, indices dentro de [0, vertices-1]
*/
int asignarArista(struct Grafo* grafo, int origen, int destino, int peso) {
    //validar que el grafo no sea NULL
    if (grafo == NULL) {
        return -1;
    }
    
    //validar que los indices esten dentro del rango valido
    if (origen < 0 || destino < 0 || origen >= grafo->vertices || destino >= grafo->vertices) {
        return -1;
    }
    
    //validar que el peso sea mayor a 0
    if (peso <= 0) {
        return -1;
    }
    
    //asignar el peso en ambas direcciones
    grafo->peso[origen][destino] = peso;
    grafo->peso[destino][origen] = peso;
    
    return 0; //exito
}

/*
E: puntero a grafo previamente creado
S: libera memoria interna y el grafo
R: no usar el puntero despues, grafo puede ser NULL
*/
void liberarGrafo(struct Grafo* grafo) {
    if (grafo == NULL) {
        return;
    }
    
    //liberar cada fila de la matriz de pesos
    if (grafo->peso != NULL) {
        for (int i = 0; i < grafo->vertices; ++i) {
            free(grafo->peso[i]);
        }
        //liberar el arreglo de punteros
        free(grafo->peso);
    }
    
    //liberar la estructura principal del grafo
    free(grafo);
}
