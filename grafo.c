#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

/*
E: cantidad de vertices mayor a 0.
S: puntero a Grafo con matriz de pesos inicializada en 0 o NULL en error.
R: memoria disponible; vertices positivo.
.*/
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
    
    //reservar memoria para la matriz de pesos (arreglo de punteros a filas)
    grafo->peso = calloc(vertices, sizeof(int*));
    
    //reservar memoria para el mapeo de indice a coordenadas (para laberintos)
    grafo->indexToCoord = calloc(vertices, sizeof(struct Point));
    
    //verificar que ambas asignaciones fueron exitosas
    if (grafo->peso == NULL || grafo->indexToCoord == NULL) {
        free(grafo->peso);
        free(grafo->indexToCoord);
        free(grafo);
        return NULL;
    }

    //reservar memoria para cada fila de la matriz de pesos
    for (int i = 0; i < vertices; ++i) {
        grafo->peso[i] = calloc(vertices, sizeof(int));
        
        //si falla la asignacion, liberar toda la memoria previamente reservada
        if (grafo->peso[i] == NULL) {
            //liberar las filas ya asignadas
            for (int j = 0; j < i; ++j) {
                free(grafo->peso[j]);
            }
            free(grafo->peso);
            free(grafo->indexToCoord);
            free(grafo);
            return NULL;
        }
    }

    //la matriz queda inicializada en 0 por calloc (sin aristas)
    return grafo;
}

/*
E: puntero a grafo, indices origen/destino y peso mayor o igual a 0.
S: asigna peso simetrico, retorna 0 si esta bien, -1 si indices fuera de rango.
R: grafo valido, indices dentro de [0, vertices-1].
.*/
int asignarArista(struct Grafo* grafo, int origen, int destino, int peso) {
    //validar que el grafo no sea NULL
    if (grafo == NULL) {
        return -1;
    }
    
    //validar que los indices de origen y destino esten dentro del rango valido
    if (origen < 0 || destino < 0 || origen >= grafo->vertices || destino >= grafo->vertices) {
        return -1;
    }
    
    //validar que el peso sea no negativo
    if (peso < 0) {
        return -1;
    }

    //asignar el peso en ambas direcciones (grafo no dirigido)
    //esto crea una arista bidireccional entre origen y destino
    grafo->peso[origen][destino] = peso;
    grafo->peso[destino][origen] = peso;
    
    return 0; //exito
}

/*
E: puntero a grafo previamente creado.
S: libera memoria interna y el grafo.
R: no usar el puntero despues, grafo puede ser NULL.
.*/
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
    
    //liberar el arreglo de mapeo de indices a coordenadas
    free(grafo->indexToCoord);
    
    //liberar la estructura principal del grafo
    free(grafo);
}

/*
E: laberinto cargado, punteros a grafo/start/goal.
S: construye matriz de pesos (1) y mapea indices; 0 si OK.
R: laberinto valido, memoria disponible, debe existir S y E.
.*/
int build_graph(const struct Maze* maze, struct Grafo* grafo, int* startIndex, int* goalIndex) {
    //matriz temporal para mapear coordenadas (fila,col) a indices de vertices
    int indexMap[MAX_ROWS][MAX_COLS];
    int openCells = 0;
    *startIndex = -1;
    *goalIndex = -1;

    //primer recorrido: contar celdas transitables y marcar muros
    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            indexMap[r][c] = -1; //inicializar como no transitable
            
            //si la celda no es un muro, es transitable
            if (maze->cells[r][c] != WALL) {
                openCells++;
            }
        }
    }

    //validar que haya al menos una celda transitable
    if (openCells == 0) {
        printf("No hay celdas transitables en el laberinto.\n");
        return -1;
    }

    //liberar el grafo anterior y crear uno nuevo con el tamano correcto
    liberarGrafo(grafo);
    struct Grafo* nuevo = crearGrafo(openCells);
    if (nuevo == NULL) {
        printf("No se pudo reservar memoria para el grafo.\n");
        return -1;
    }
    
    //transferir los punteros del nuevo grafo al grafo existente
    grafo->vertices = nuevo->vertices;
    grafo->peso = nuevo->peso;
    grafo->indexToCoord = nuevo->indexToCoord;
    free(nuevo); // liberar la estructura temporal, las matrices quedan en grafo

    //segundo recorrido: asignar indices a las celdas transitables
    int currentIndex = 0;
    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            //si la celda es transitable
            if (maze->cells[r][c] != WALL) {
                //asignar el indice del vertice a esta posicion
                indexMap[r][c] = currentIndex;
                
                //guardar las coordenadas del vertice
                grafo->indexToCoord[currentIndex].row = r;
                grafo->indexToCoord[currentIndex].col = c;
                
                //identificar los puntos de inicio y meta
                if (maze->cells[r][c] == START) {
                    *startIndex = currentIndex;
                } else if (maze->cells[r][c] == END) {
                    *goalIndex = currentIndex;
                }
                currentIndex++;
            }
        }
    }

    //vectores de desplazamiento para los 4 vecinos (arriba, abajo, izquierda, derecha)
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    
    //tercer recorrido: crear aristas entre celdas adyacentes
    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            int from = indexMap[r][c];
            
            //si esta celda no es transitable, continuar
            if (from == -1) {
                continue;
            }
            
            //revisar los 4 vecinos
            for (int k = 0; k < 4; ++k) {
                int nr = r + dr[k]; //nueva fila
                int nc = c + dc[k]; //nueva columna
                
                //verificar que el vecino este dentro de los limites
                if (nr < 0 || nr >= maze->rows || nc < 0 || nc >= maze->cols) {
                    continue;
                }
                
                int to = indexMap[nr][nc];
                
                //si el vecino es transitable, crear una arista con peso 1
                if (to != -1) {
                    grafo->peso[from][to] = 1;
                }
            }
        }
    }

    //validar que se encontraron los puntos de inicio y meta
    if (*startIndex == -1 || *goalIndex == -1) {
        printf("Faltan los puntos de inicio (S) y/o meta (E) en el laberinto.\n");
        liberarGrafo(grafo);
        return -1;
    }

    return 0; //exito
}

/*
E: puntero a grafo, cantidad de vertices (2-100), probabilidad de arista [0,1].
S: crea matriz de pesos (1) aleatoria simetrica; retorna 0 si OK.
R: memoria disponible; vertices en rango; edgeProb se ajusta a [0,1].
.*/
int generate_random_graph(struct Grafo* grafo, int vertices, double edgeProb) {
    //validar que el numero de vertices este en un rango razonable
    if (vertices < 2 || vertices > 100) {
        printf("El numero de nodos debe estar entre 2 y 100.\n");
        return -1;
    }
    
    //ajustar la probabilidad al rango [0, 1]
    if (edgeProb < 0.0) edgeProb = 0.0;
    if (edgeProb > 1.0) edgeProb = 1.0;

    //liberar el grafo anterior y crear uno nuevo
    liberarGrafo(grafo);
    struct Grafo* nuevo = crearGrafo(vertices);
    if (nuevo == NULL) {
        printf("No se pudo reservar memoria para el grafo aleatorio.\n");
        return -1;
    }
    
    //transferir los punteros del nuevo grafo al grafo existente
    grafo->vertices = nuevo->vertices;
    grafo->peso = nuevo->peso;
    grafo->indexToCoord = nuevo->indexToCoord;
    free(nuevo); //liberar la estructura temporal

    //inicializar las coordenadas (no aplica para grafos aleatorios, solo por compatibilidad)
    for (int i = 0; i < vertices; ++i) {
        grafo->indexToCoord[i].row = i;
        grafo->indexToCoord[i].col = 0;
    }

    //generar aristas aleatorias
    //solo revisar la mitad superior de la matriz para evitar duplicados
    for (int i = 0; i < vertices; ++i) {
        for (int j = i + 1; j < vertices; ++j) {
            //generar un numero aleatorio entre 0 y 1
            double r = (double)rand() / (double)RAND_MAX;
            
            //si el numero es menor o igual a la probabilidad, crear la arista
            if (r <= edgeProb) {
                //asignar peso 1 en ambas direcciones (grafo no dirigido)
                grafo->peso[i][j] = 1;
                grafo->peso[j][i] = 1;
            }
        }
    }
    
    return 0; //exito
}
