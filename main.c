#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bfs.h"
#include "grafo.h"
#include "laberinto.h"
#include "visualizacion.h"

//muestra el menu principal
static void show_menu() {
    printf("\n--- Menu ---\n");
    printf("1) Cargar laberinto desde archivo\n");
    printf("2) Ejecutar BFS (S -> E)\n");
    printf("3) Mostrar matriz de adyacencia\n");
    printf("4) Generar grafo aleatorio y ejecutar BFS\n");
    printf("0) Salir\n");
    printf("> ");
}

int main(void) {
    Maze maze = (Maze){0};
    struct Grafo graph = {0};
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
            int *parent = calloc(graph.vertices, sizeof(int));
            int *visitOrder = calloc(graph.vertices, sizeof(int));
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

                int *parent = calloc(graph.vertices, sizeof(int));
                int *visitOrder = calloc(graph.vertices, sizeof(int));
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

    liberarGrafo(&graph);
    printf("Saliendo...\n");
    return 0;
}
