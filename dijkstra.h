#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

//estructura para el resultado del camino
struct Camino {
    int* nodos; //lista de nodos/vertices en el camino creado
    int longitud; //numero de nodos en el camino
    int valorTotal; //valor total del camino
};

//nodo actual al que se esta comparando el valor
//estructura para la cola de prioridad (min-heap)
struct NodoPrioridad {
    int vertice;   //nodo/vertice del grafo
    int valor; //valor acumulado desde el inicio
};

//estructura que mantiene los nodos ordenados por valor (el de menor valor esta al frente)
//se ocupa el menor para saber por cuales aristas son las que menos costo de pasar tienen
struct ColaPrioridad {
    struct NodoPrioridad* heap;
    int* posiciones; //para actualizaciones de prioridades
    int tamano; //cuantos elementos hay en el heap
    int capacidad; //tamano maximo del heap (numero de vertices del grafo)
};

// funciones de Dijkstra
struct Camino* dijkstra(struct Grafo* grafo, int inicio, int fin);
void liberarCamino(struct Camino* camino);

// funciones de cola de prioridad
struct ColaPrioridad* crearColaPrioridad(int capacidad);
void insertarCola(struct ColaPrioridad* cola, int vertice, int valor);
struct NodoPrioridad extraerMinimo(struct ColaPrioridad* cola);
void liberarColaPrioridad(struct ColaPrioridad* cola);

#endif