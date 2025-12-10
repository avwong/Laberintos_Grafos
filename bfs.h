/*
E: n/a (cabecera).
S: declara BFS para grafos con matriz 0/1.
R: usar con bfs.c y grafo.h.
.*/
#ifndef BFS_H
#define BFS_H

#include "grafo.h"

/*
E: grafo, nodo inicio y meta, arreglos parent/visitOrder y contador.
S: ejecuta BFS, llena parent y orden visitado, retorna 1 si encontro goal.
R: arreglos de tamano vertices; indices validos; memoria disponible.
.*/
int bfs(const struct Grafo *graph, int start, int goal, int *parent, int *visitOrder, int *visitCount);

#endif
