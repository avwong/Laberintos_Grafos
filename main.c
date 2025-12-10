#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ROWS 128
#define MAX_COLS 128
#define WALL '#'
#define START 'S'
#define END 'E'

struct Point {
    int row;
    int col;
};

struct Maze {
    int rows;
    int cols;
    char cells[MAX_ROWS][MAX_COLS + 1];
};

struct Graph {
    int vertices;
    int **adj;
    struct Point *indexToCoord;
};

/* E: cadena con posible salto de linea. 
 * S: elimina '\n' o '\r' al final de la cadena. 
 * R: puntero no nulo a un buffer modificable. */
static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

/* E: grafo previamente creado (puede estar vacio). 
 * S: libera matrices internas y reinicia contadores. 
 * R: puntero valido; usar antes de reusar el grafo. */
static void free_graph(struct Graph *graph) {
    if (graph->adj != NULL) {
        for (int i = 0; i < graph->vertices; ++i) {
            free(graph->adj[i]);
        }
        free(graph->adj);
    }
    free(graph->indexToCoord);
    graph->adj = NULL;
    graph->indexToCoord = NULL;
    graph->vertices = 0;
}

/* E: ruta de archivo de laberinto y puntero Maze. 
 * S: carga celdas, filas y columnas; retorna 0 si OK, -1 si error. 
 * R: archivo legible, filas con misma longitud, maximo 128x128. */
static int load_maze(const char *filename, struct Maze *maze) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("No se pudo abrir el archivo");
        return -1;
    }

    char buffer[MAX_COLS + 4];
    int row = 0;
    int expectedCols = -1;

    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        trim_newline(buffer);
        size_t len = strlen(buffer);
        if (len == 0) {
            continue; // ignora lineas vacias
        }
        if ((int)len > MAX_COLS) {
            printf("Linea %d excede el maximo de columnas (%d)\n", row + 1, MAX_COLS);
            fclose(f);
            return -1;
        }
        if (expectedCols == -1) {
            expectedCols = (int)len;
        } else if ((int)len != expectedCols) {
            printf("Las lineas deben tener la misma longitud. Linea %d tiene %zu en lugar de %d.\n", row + 1, len, expectedCols);
            fclose(f);
            return -1;
        }
        if (row >= MAX_ROWS) {
            printf("Numero de filas excede el maximo permitido (%d).\n", MAX_ROWS);
            fclose(f);
            return -1;
        }
        strcpy(maze->cells[row], buffer);
        row++;
    }

    fclose(f);

    if (row == 0) {
        printf("El archivo esta vacio o no tiene filas validas.\n");
        return -1;
    }

    maze->rows = row;
    maze->cols = expectedCols;
    return 0;
}

/* E: laberinto cargado, punteros a grafo/start/goal. 
 * S: construye matriz de adyacencia, mapea indices y ubica S/E; 0 si OK. 
 * R: laberinto valido, memoria disponible, debe existir S y E. */
static int build_graph(const struct Maze *maze, struct Graph *graph, int *startIndex, int *goalIndex) {
    int indexMap[MAX_ROWS][MAX_COLS];
    int openCells = 0;
    *startIndex = -1;
    *goalIndex = -1;

    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            indexMap[r][c] = -1;
            if (maze->cells[r][c] != WALL) {
                openCells++;
            }
        }
    }

    if (openCells == 0) {
        printf("No hay celdas transitables en el laberinto.\n");
        return -1;
    }

    free_graph(graph);
    graph->vertices = openCells;
    graph->adj = (int **)calloc(openCells, sizeof(int *));
    graph->indexToCoord = (struct Point *)calloc(openCells, sizeof(struct Point));
    if (graph->adj == NULL || graph->indexToCoord == NULL) {
        printf("No se pudo reservar memoria para el grafo.\n");
        free_graph(graph);
        return -1;
    }
    for (int i = 0; i < openCells; ++i) {
        graph->adj[i] = (int *)calloc(openCells, sizeof(int));
        if (graph->adj[i] == NULL) {
            printf("No se pudo reservar memoria para la matriz de adyacencia.\n");
            free_graph(graph);
            return -1;
        }
    }

    int currentIndex = 0;
    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            if (maze->cells[r][c] != WALL) {
                indexMap[r][c] = currentIndex;
                graph->indexToCoord[currentIndex].row = r;
                graph->indexToCoord[currentIndex].col = c;
                if (maze->cells[r][c] == START) {
                    *startIndex = currentIndex;
                } else if (maze->cells[r][c] == END) {
                    *goalIndex = currentIndex;
                }
                currentIndex++;
            }
        }
    }

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    for (int r = 0; r < maze->rows; ++r) {
        for (int c = 0; c < maze->cols; ++c) {
            int from = indexMap[r][c];
            if (from == -1) {
                continue;
            }
            for (int k = 0; k < 4; ++k) {
                int nr = r + dr[k];
                int nc = c + dc[k];
                if (nr < 0 || nr >= maze->rows || nc < 0 || nc >= maze->cols) {
                    continue;
                }
                int to = indexMap[nr][nc];
                if (to != -1) {
                    graph->adj[from][to] = 1;
                }
            }
        }
    }

    if (*startIndex == -1 || *goalIndex == -1) {
        printf("Faltan los puntos de inicio (S) y/o meta (E) en el laberinto.\n");
        free_graph(graph);
        return -1;
    }

    return 0;
}

/* E: grafo, nodo inicio y meta, arreglos parent/visitOrder y contador. 
 * S: ejecuta BFS, llena parent y orden visitado, retorna 1 si encontro goal. 
 * R: arreglos de tamano vertices; indices validos; memoria disponible. */
static int bfs(const struct Graph *graph, int start, int goal, int *parent, int *visitOrder, int *visitCount) {
    int *visited = (int *)calloc(graph->vertices, sizeof(int));
    int *queue = (int *)calloc(graph->vertices, sizeof(int));
    int head = 0;
    int tail = 0;

    if (visited == NULL || queue == NULL) {
        printf("No se pudo reservar memoria para BFS.\n");
        free(visited);
        free(queue);
        return 0;
    }

    for (int i = 0; i < graph->vertices; ++i) {
        parent[i] = -1;
    }

    queue[tail++] = start;
    visited[start] = 1;
    *visitCount = 0;

    while (head < tail) {
        int v = queue[head++];
        visitOrder[(*visitCount)++] = v;

        if (v == goal) {
            free(visited);
            free(queue);
            return 1;
        }

        for (int u = 0; u < graph->vertices; ++u) {
            if (graph->adj[v][u] == 1 && !visited[u]) {
                visited[u] = 1;
                parent[u] = v;
                queue[tail++] = u;
            }
        }
    }

    free(visited);
    free(queue);
    return 0;
}

/* E: grafo y arreglo de visita BFS con su cantidad. 
 * S: imprime numero de nodo y coordenadas visitadas. 
 * R: indices validos dentro de graph->vertices. */
static void print_visit_order(const struct Graph *graph, const int *visitOrder, int visitCount) {
    printf("Orden de visita (BFS):\n");
    for (int i = 0; i < visitCount; ++i) {
        struct Point p = graph->indexToCoord[visitOrder[i]];
        printf("%3d) nodo %d -> (%d, %d)\n", i + 1, visitOrder[i], p.row, p.col);
    }
}

/* E: arreglo de visita y cantidad. 
 * S: imprime orden de visita solo con indices. 
 * R: indices validos. */
static void print_visit_order_simple(const int *visitOrder, int visitCount) {
    printf("Orden de visita (BFS):\n");
    for (int i = 0; i < visitCount; ++i) {
        printf("%3d) nodo %d\n", i + 1, visitOrder[i]);
    }
}

/* E: laberinto, grafo, arreglo parent y nodos inicio/goal. 
 * S: imprime laberinto con camino marcado con 'o'. 
 * R: parent describe una ruta valida entre goal y start. */
static void print_path_on_maze(const struct Maze *maze, const struct Graph *graph, const int *parent, int start, int goal) {
    char display[MAX_ROWS][MAX_COLS + 1];
    for (int r = 0; r < maze->rows; ++r) {
        strcpy(display[r], maze->cells[r]);
    }

    int v = goal;
    while (v != -1) {
        struct Point p = graph->indexToCoord[v];
        char *cell = &display[p.row][p.col];
        if (v == start) {
            *cell = START;
        } else if (v == goal) {
            *cell = END;
        } else {
            *cell = 'o';
        }
        v = parent[v];
    }

    printf("Camino encontrado (marcado con 'o'):\n");
    for (int r = 0; r < maze->rows; ++r) {
        printf("%s\n", display[r]);
    }
}

/* E: arreglo parent, indices inicio/goal y numero de vertices. 
 * S: imprime camino por indices o indica si no existe. 
 * R: parent de tamano vertices; indices dentro de rango. */
static void print_path_indices(const int *parent, int start, int goal, int vertices) {
    int *stack = (int *)calloc(vertices, sizeof(int));
    if (stack == NULL) {
        printf("No se pudo reservar memoria para mostrar el camino.\n");
        return;
    }

    int len = 0;
    int v = goal;
    while (v != -1 && len < vertices) {
        stack[len++] = v;
        if (v == start) {
            break;
        }
        v = parent[v];
    }

    if (v == -1) {
        printf("No hay camino entre %d y %d.\n", start, goal);
        free(stack);
        return;
    }

    printf("Camino encontrado:\n");
    for (int i = len - 1; i >= 0; --i) {
        printf("%d", stack[i]);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\n");
    free(stack);
}

/* E: grafo con matriz de adyacencia. 
 * S: imprime matriz completa o primeras 30 filas/cols. 
 * R: matriz cuadrada de tamano vertices. */
static void print_adjacency_matrix(const struct Graph *graph) {
    int limit = graph->vertices;
    if (limit > 30) {
        printf("Matriz de adyacencia parcial: mostrando primeras %d filas/cols de %d nodos.\n", 30, graph->vertices);
        limit = 30;
    } else {
        printf("Matriz de adyacencia completa (%d nodos):\n", graph->vertices);
    }

    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < limit; ++j) {
            printf("%d ", graph->adj[i][j]);
        }
        printf("\n");
    }
}

/* E: ninguna. 
 * S: muestra opciones del menu en consola. 
 * R: requiere stdout disponible. */
static void show_menu(void) {
    printf("\n--- Menu ---\n");
    printf("1) Cargar laberinto desde archivo\n");
    printf("2) Ejecutar BFS (S -> E)\n");
    printf("3) Mostrar matriz de adyacencia\n");
    printf("4) Generar grafo aleatorio y ejecutar BFS\n");
    printf("0) Salir\n");
    printf("> ");
}

/* E: puntero a grafo, cantidad de vertices (2-100), probabilidad de arista [0,1]. 
 * S: crea matriz de adyacencia aleatoria simetrica; retorna 0 si OK. 
 * R: memoria disponible; vertices en rango; edgeProb se ajusta a [0,1]. */
static int generate_random_graph(struct Graph *graph, int vertices, double edgeProb) {
    if (vertices < 2 || vertices > 100) {
        printf("El numero de nodos debe estar entre 2 y 100.\n");
        return -1;
    }
    if (edgeProb < 0.0) edgeProb = 0.0;
    if (edgeProb > 1.0) edgeProb = 1.0;

    free_graph(graph);
    graph->vertices = vertices;
    graph->adj = (int **)calloc(vertices, sizeof(int *));
    graph->indexToCoord = (struct Point *)calloc(vertices, sizeof(struct Point));
    if (graph->adj == NULL || graph->indexToCoord == NULL) {
        printf("No se pudo reservar memoria para el grafo aleatorio.\n");
        free_graph(graph);
        return -1;
    }
    for (int i = 0; i < vertices; ++i) {
        graph->adj[i] = (int *)calloc(vertices, sizeof(int));
        if (graph->adj[i] == NULL) {
            printf("No se pudo reservar memoria para la matriz de adyacencia.\n");
            free_graph(graph);
            return -1;
        }
        graph->indexToCoord[i].row = i;
        graph->indexToCoord[i].col = 0;
    }

    for (int i = 0; i < vertices; ++i) {
        for (int j = i + 1; j < vertices; ++j) {
            double r = (double)rand() / (double)RAND_MAX;
            if (r <= edgeProb) {
                graph->adj[i][j] = 1;
                graph->adj[j][i] = 1;
            }
        }
    }
    return 0;
}

/* E: ninguna (interaccion por stdin). 
 * S: bucle de menu para cargar laberintos, ejecutar BFS y mostrar datos. 
 * R: entrada valida; archivos legibles; sin integracion de Dijkstra aun. */
int main(void) {
    struct Maze maze = {0};
    struct Graph graph = {0};
    int startIndex = -1;
    int goalIndex = -1;
    int mazeLoaded = 0;
    int graphReady = 0;
    char input[256];

    srand((unsigned)time(NULL));

    printf("Proyecto Laberinto + Grafo\n");
    printf("Formato de archivo: mismo numero de columnas por fila. Usa '#' para muro, '.' o espacio para camino, 'S' inicio, 'E' meta.\n");

    for (;;) {
        show_menu();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        int option = atoi(input);

        if (option == 0) {
            break;
        }

        if (option == 1) {
            printf("Ruta del archivo: ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            trim_newline(input);
            if (load_maze(input, &maze) == 0) {
                if (build_graph(&maze, &graph, &startIndex, &goalIndex) == 0) {
                    mazeLoaded = 1;
                    graphReady = 1;
                    printf("Laberinto cargado. Dimensiones: %d x %d. Nodos: %d.\n", maze.rows, maze.cols, graph.vertices);
                    printf("Inicio (S): indice %d, Meta (E): indice %d.\n", startIndex, goalIndex);
                } else {
                    mazeLoaded = 0;
                    graphReady = 0;
                }
            }
        } else if (option == 2) {
            if (!mazeLoaded) {
                printf("Primero cargue un laberinto valido.\n");
                continue;
            }
            int *parent = (int *)calloc(graph.vertices, sizeof(int));
            int *visitOrder = (int *)calloc(graph.vertices, sizeof(int));
            int visitCount = 0;
            if (parent == NULL || visitOrder == NULL) {
                printf("No se pudo reservar memoria para BFS.\n");
                free(parent);
                free(visitOrder);
                continue;
            }
            int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);
            print_visit_order(&graph, visitOrder, visitCount);
            if (found) {
                print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
            } else {
                printf("No hay camino entre S y E.\n");
            }
            free(parent);
            free(visitOrder);
        } else if (option == 3) {
            if (!graphReady) {
                printf("Primero cargue un laberinto valido o genere un grafo aleatorio.\n");
                continue;
            }
            print_adjacency_matrix(&graph);
        } else if (option == 4) {
            printf("Numero de nodos (2-100): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            int vertices = atoi(input);
            if (vertices < 2 || vertices > 100) {
                printf("Valor fuera de rango.\n");
                continue;
            }
            printf("Probabilidad de arista (0-100%%): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            double probPct = atof(input);
            if (probPct < 0.0) probPct = 0.0;
            if (probPct > 100.0) probPct = 100.0;
            double edgeProb = probPct / 100.0;

            if (generate_random_graph(&graph, vertices, edgeProb) == 0) {
                graphReady = 1;
                mazeLoaded = 0; // este grafo no corresponde a ningun laberinto cargado
                startIndex = rand() % vertices;
                do {
                    goalIndex = rand() % vertices;
                } while (goalIndex == startIndex && vertices > 1);

                int *parent = (int *)calloc(graph.vertices, sizeof(int));
                int *visitOrder = (int *)calloc(graph.vertices, sizeof(int));
                int visitCount = 0;
                if (parent == NULL || visitOrder == NULL) {
                    printf("No se pudo reservar memoria para BFS.\n");
                    free(parent);
                    free(visitOrder);
                    continue;
                }
                int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);
                print_adjacency_matrix(&graph);
                printf("BFS desde %d hasta %d\n", startIndex, goalIndex);
                print_visit_order_simple(visitOrder, visitCount);
                if (found) {
                    print_path_indices(parent, startIndex, goalIndex, graph.vertices);
                } else {
                    printf("No hay camino entre %d y %d.\n", startIndex, goalIndex);
                }
                free(parent);
                free(visitOrder);
            }
        } else {
            printf("Opcion no valida.\n");
        }
    }

    free_graph(&graph);
    printf("Saliendo...\n");
    return 0;
}
