#ifndef VISUALIZACION_H
#define VISUALIZACION_H

#include "grafo.h"
#include "laberinto.h"
#include "dijkstra.h"

//funciones de impresion para visualizacion de resultados

void print_visit_order(const struct Grafo *graph, const int *visitOrder, int visitCount);
void print_visit_order_simple(const int *visitOrder, int visitCount);
void print_path_on_maze(const struct Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal);
void print_path_steps(const struct Maze *maze, const struct Grafo *graph, const int *parent, int start, int goal);
void print_path_indices(const int *parent, int start, int goal, int vertices);
int expand_path_with_intermediate_cells(const struct Maze *maze, const struct Grafo *graph, const int *path, int pathLen, struct Point *expandedPath);
int build_path_sequence(const int *parent, int start, int goal, int vertices, int *out);
void print_adjacency_matrix(const struct Grafo *graph);
void print_estado_dijkstra(int paso, int actual, const int *val, const int *visitado, int n);
void imprimirCaminoDijkstra(struct Camino* camino);

#endif
