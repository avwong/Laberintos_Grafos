#ifndef GRAFO_H
#define GRAFO_H

//representacion simple de grafo no dirigido con matriz de pesos
struct Grafo {
    int vertices; // numero de nodos
    int** peso;   // matriz de pesos, 0 o negativo indica que no hay arista
};

//funciones
struct Grafo* crearGrafo(int vertices);
int asignarArista(struct Grafo* grafo, int origen, int destino, int peso);
void liberarGrafo(struct Grafo* grafo);

#endif
