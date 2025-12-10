#ifndef GRAFO_H
#define GRAFO_H

#include "laberinto.h"

//representacion de grafo no dirigido con matriz de pesos
struct Grafo {
    int vertices; // numero de nodos
    int** peso; // matriz de pesos, 0 o negativo indica que no hay arista
    struct Point* indexToCoord; // indice a coordenada (para laberintos)
};

// funciones para crear, modificar y liberar grafos
struct Grafo* crearGrafo(int vertices);
int asignarArista(struct Grafo* grafo, int origen, int destino, int peso);
void liberarGrafo(struct Grafo* grafo);

// construye grafo a partir de un laberinto; arma pesos 1 donde hay camino.
int build_graph(const struct Maze* maze, struct Grafo* grafo, int* startIndex, int* goalIndex);

// genera grafo aleatorio no dirigido con pesos 1 segun probabilidad.
int generate_random_graph(struct Grafo* grafo, int vertices, double edgeProb);

// construye una representacion visual de laberinto a partir de un grafo aleatorio
int build_maze_from_graph(struct Grafo* grafo, struct Maze* maze, int startIndex, int goalIndex);

#endif
