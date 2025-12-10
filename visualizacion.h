/*
E: n/a (cabecera).
S: declara utilidades de impresion para recorridos y caminos.
R: usar con visualizacion.c.
.*/
#ifndef VISUALIZACION_H
#define VISUALIZACION_H

#include "grafo.h"
#include "laberinto.h"

/*
E: grafo y arreglo de visita BFS con su cantidad.
S: imprime numero de nodo y coordenadas visitadas.
R: indices validos dentro de graph->vertices.
.*/
void print_visit_order(const struct Grafo *graph, const int *visitOrder, int visitCount);

/*
E: arreglo de visita y cantidad.
S: imprime orden de visita solo con indices.
R: indices validos.
.*/
void print_visit_order_simple(const int *visitOrder, int visitCount);

/*
E: laberinto, grafo, arreglo parent y nodos inicio/goal.
S: imprime laberinto con camino marcado con 'o'.
R: parent describe una ruta valida entre goal y start.
.*/
void print_path_on_maze(const struct Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal);

/*
E: arreglo parent, indices inicio/goal y numero de vertices.
S: imprime camino por indices o indica si no existe.
R: parent de tamano vertices; indices dentro de rango.
.*/
void print_path_indices(const int *parent, int start, int goal, int vertices);

/*
E: grafo con matriz de adyacencia.
S: imprime matriz completa o primeras 30 filas/cols.
R: matriz cuadrada de tamano vertices.
.*/
void print_adjacency_matrix(const struct Grafo *graph);

#endif
