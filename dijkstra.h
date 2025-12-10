#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

// Alias de structs para usar nombres cortos sin escribir "struct".
typedef struct Camino Camino;
typedef struct NodoPrioridad NodoPrioridad;
typedef struct ColaPrioridad ColaPrioridad;

// estructura para el resultado del camino
struct Camino {
    int* nodos;            // lista de nodos/vertices en el camino
    int longitud;          // numero de nodos
    int distanciaTotal;    // distancia total del camino
};

// estructura para la cola de prioridad (min-heap)
struct NodoPrioridad {
    int vertice;   // nodo/vertice del grafo
    int distancia; // distancia desde el inicio
};

struct ColaPrioridad {
    NodoPrioridad* heap;
    int* posiciones; // para actualizaciones de prioridades (decrease-key)
    int tamanio;
    int capacidad;
};

// funciones de Dijkstra
Camino* dijkstra(Grafo* grafo, int inicio, int fin);
void imprimirCamino(Camino* camino);
void liberarCamino(Camino* camino);

// funciones de cola de prioridad
ColaPrioridad* crearColaPrioridad(int capacidad);
void insertarCola(ColaPrioridad* cola, int vertice, int distancia);
NodoPrioridad extraerMinimo(ColaPrioridad* cola);
void liberarColaPrioridad(ColaPrioridad* cola);

#endif
