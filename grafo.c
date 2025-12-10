#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        grafo->peso = NULL;
    }
    
    //liberar el arreglo de mapeo de indices a coordenadas
    if (grafo->indexToCoord != NULL) {
        free(grafo->indexToCoord);
        grafo->indexToCoord = NULL;
    }
    
    //reiniciar el contador de vertices
    grafo->vertices = 0;
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
        printf("Faltan los puntos de inicio (I) y/o meta (F) en el laberinto.\n");
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

    //distribuir los nodos en una cuadricula logica
    //calcular dimensiones de la cuadricula (aproximadamente cuadrada)
    int gridCols = (int)(sqrt((double)vertices) + 0.5);
    if (gridCols < 1) gridCols = 1;

    //asignar coordenadas a cada nodo en la cuadricula
    for (int i = 0; i < vertices; ++i) {
        grafo->indexToCoord[i].row = i / gridCols;
        grafo->indexToCoord[i].col = i % gridCols;
    }

    //generar aristas aleatorias SOLO entre nodos adyacentes en la cuadricula
    //esto asegura que el grafo corresponda al laberinto visual
    for (int i = 0; i < vertices; ++i) {
        int iRow = grafo->indexToCoord[i].row;
        int iCol = grafo->indexToCoord[i].col;

        //revisar los 4 vecinos potenciales (arriba, abajo, izquierda, derecha)
        int dr[4] = {-1, 1, 0, 0};
        int dc[4] = {0, 0, -1, 1};

        for (int dir = 0; dir < 4; ++dir) {
            int neighborRow = iRow + dr[dir];
            int neighborCol = iCol + dc[dir];

            //buscar el nodo que corresponde a esta posicion adyacente
            for (int j = 0; j < vertices; ++j) {
                if (grafo->indexToCoord[j].row == neighborRow &&
                    grafo->indexToCoord[j].col == neighborCol) {

                    //evitar crear la arista dos veces
                    if (i < j) {
                        //generar numero aleatorio para decidir si crear la arista
                        double r = (double)rand() / (double)RAND_MAX;

                        if (r <= edgeProb) {
                            //crear arista bidireccional con peso 1
                            grafo->peso[i][j] = 1;
                            grafo->peso[j][i] = 1;
                        }
                    }
                    break;
                }
            }
        }
    }

    return 0; //exito
}

/*
E: grafo con indexToCoord lleno, puntero a maze, indices de start y goal.
S: llena maze con representacion visual del grafo; 0 si OK, -1 en error.
R: grafo valido con vertices > 0, indices dentro de rango.
*/
int build_maze_from_graph(struct Grafo* grafo, struct Maze* maze, int startIndex, int goalIndex) {
    if (grafo == NULL || maze == NULL || grafo->vertices <= 0) {
        return -1;
    }

    if (startIndex < 0 || goalIndex < 0 || startIndex >= grafo->vertices || goalIndex >= grafo->vertices) {
        return -1;
    }

    //guardar las coordenadas originales del grafo antes de modificarlas
    struct Point* originalCoords = calloc(grafo->vertices, sizeof(struct Point));
    if (originalCoords == NULL) {
        return -1;
    }
    for (int i = 0; i < grafo->vertices; ++i) {
        originalCoords[i] = grafo->indexToCoord[i];
    }

    //encontrar las dimensiones del laberinto basadas en las coordenadas de los nodos
    int maxRow = 0, maxCol = 0;
    for (int i = 0; i < grafo->vertices; ++i) {
        if (originalCoords[i].row > maxRow) {
            maxRow = originalCoords[i].row;
        }
        if (originalCoords[i].col > maxCol) {
            maxCol = originalCoords[i].col;
        }
    }

    //crear un laberinto con espacio para paredes entre nodos y bordes
    //cada nodo ocupa 2x2 caracteres (nodo + espacio horizontal/vertical)
    int mazeRows = maxRow * 2 + 3; //+3 para bordes superior e inferior y el nodo final
    int mazeCols = maxCol * 2 + 3; //+3 para bordes izquierdo y derecho y el nodo final

    //validar que no exceda los limites
    if (mazeRows > MAX_ROWS || mazeCols > MAX_COLS) {
        printf("El laberinto generado excede las dimensiones maximas.\n");
        free(originalCoords);
        return -1;
    }

    maze->rows = mazeRows;
    maze->cols = mazeCols;

    //inicializar todo con muros
    for (int r = 0; r < mazeRows; ++r) {
        for (int c = 0; c < mazeCols; ++c) {
            maze->cells[r][c] = WALL;
        }
        maze->cells[r][mazeCols] = '\0';
    }

    //colocar los nodos en el laberinto y actualizar sus coordenadas
    for (int i = 0; i < grafo->vertices; ++i) {
        int nodeRow = originalCoords[i].row;
        int nodeCol = originalCoords[i].col;

        //mapear coordenadas del nodo a posicion en el laberinto
        int mazeRow = nodeRow * 2 + 1;
        int mazeCol = nodeCol * 2 + 1;

        //actualizar las coordenadas del grafo para que apunten al laberinto visual
        grafo->indexToCoord[i].row = mazeRow;
        grafo->indexToCoord[i].col = mazeCol;

        //marcar la posicion del nodo
        if (i == startIndex) {
            maze->cells[mazeRow][mazeCol] = START;
        } else if (i == goalIndex) {
            maze->cells[mazeRow][mazeCol] = END;
        } else {
            maze->cells[mazeRow][mazeCol] = '.';
        }
    }

    //crear caminos entre nodos conectados
    for (int i = 0; i < grafo->vertices; ++i) {
        int fromRow = originalCoords[i].row;
        int fromCol = originalCoords[i].col;
        int mazeFromRow = fromRow * 2 + 1;
        int mazeFromCol = fromCol * 2 + 1;

        for (int j = i + 1; j < grafo->vertices; ++j) {
            //si hay una arista entre i y j
            if (grafo->peso[i][j] > 0) {
                int toRow = originalCoords[j].row;
                int toCol = originalCoords[j].col;
                int mazeToRow = toRow * 2 + 1;
                int mazeToCol = toCol * 2 + 1;

                //si son nodos adyacentes horizontalmente
                if (fromRow == toRow && abs(fromCol - toCol) == 1) {
                    int wallCol = (mazeFromCol + mazeToCol) / 2;
                    maze->cells[mazeFromRow][wallCol] = '.';
                }
                //si son nodos adyacentes verticalmente
                else if (fromCol == toCol && abs(fromRow - toRow) == 1) {
                    int wallRow = (mazeFromRow + mazeToRow) / 2;
                    maze->cells[wallRow][mazeFromCol] = '.';
                }
            }
        }
    }

    //agregar bordes decorativos con '#'
    for (int c = 0; c < mazeCols; ++c) {
        maze->cells[0][c] = '#';
        maze->cells[mazeRows - 1][c] = '#';
    }
    for (int r = 0; r < mazeRows; ++r) {
        maze->cells[r][0] = '#';
        maze->cells[r][mazeCols - 1] = '#';
    }

    free(originalCoords);
    return 0;
}
