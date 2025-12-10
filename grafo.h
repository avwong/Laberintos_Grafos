#ifndef GRAFO_H
#define GRAFO_H

// Representacion simple de grafo no dirigido con matriz de pesos.
typedef struct Grafo {
    int vertices; // numero de nodos
    int** peso;   // matriz de pesos; 0 o negativo indica que no hay arista
} Grafo;

// E: cantidad de vertices (>0).
// S: puntero a Grafo con matriz de pesos inicializada en 0; NULL en error.
// R: memoria disponible; vertices positivo.
Grafo* crearGrafo(int vertices);

// E: puntero a grafo, indices origen/destino y peso (>=0).
// S: asigna peso simetrico; retorna 0 si OK, -1 si indices fuera de rango.
// R: grafo valido; indices dentro de [0, vertices-1].
int asignarArista(Grafo* grafo, int origen, int destino, int peso);

// E: puntero a grafo previamente creado.
// S: libera memoria interna y el grafo.
// R: no usar el puntero despues; grafo puede ser NULL.
void liberarGrafo(Grafo* grafo);

#endif
