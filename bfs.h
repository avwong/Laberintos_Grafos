#ifndef BFS_H
#define BFS_H

#include "grafo.h"

//declara BFS para grafos con matriz 0/1
int bfs(const struct Grafo *graph, int start, int goal, int *parent, int *visitOrder, int *visitCount);

#endif
