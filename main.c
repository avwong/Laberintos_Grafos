#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//headers de los modulos del proyecto
#include "bfs.h"           //algoritmo de busqueda en amplitud
#include "dijkstra.h"      //algoritmo de Dijkstra para camino mas corto
#include "grafo.h"         //estructura y funciones para manejar grafos
#include "laberinto.h"     //carga y representacion de laberintos
#include "visualizacion.h" //funciones para imprimir resultados

//muestra el menu principal con todas las opciones disponibles
static void show_menu() {
    printf("\n--- Menu ---\n");
    printf("1) Cargar laberinto desde archivo\n");
    printf("2) Ejecutar BFS (I -> F)\n");
    printf("3) Ejecutar Dijkstra (I -> F)\n");
    printf("4) Mostrar matriz de adyacencia\n");
    printf("5) Generar grafo aleatorio y ejecutar BFS\n");
    printf("6) Generar grafo aleatorio y ejecutar Dijkstra\n");
    printf("0) Salir\n");
    printf("> ");
}

int main() {
    //estructura que almacena el laberinto cargado desde archivo
    struct Maze maze = (struct Maze){0};
    
    //estructura del grafo que representa las conexiones entre celdas/nodos
    struct Grafo graph = {0};
    
    //indices del nodo de inicio (I) y meta (F) en el grafo
    int startIndex = -1;
    int goalIndex = -1;
    
    //flags para controlar el estado del programa
    int mazeLoaded = 0;  //indica si hay un laberinto cargado
    int graphReady = 0;  //indica si el grafo esta listo para usar
    
    //buffer para leer entrada del usuario
    char input[256];

    //inicializar el generador de numeros aleatorios
    srand((unsigned)time(NULL));

    printf("Proyecto Laberinto + Grafo\n");
    printf("Formato de archivo: mismo numero de columnas por fila. Usa 'X' para muro, '.' o espacio para camino, 'I' inicio, 'F' meta.\n");

    for (;;) {
        show_menu();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        int option = atoi(input);

        if (option == 0) {
            break; //salir del programa
        }

        //Cargar laberinto desde archivo
        if (option == 1) {
            printf("Ruta del archivo: ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            trim_newline(input);
            
            //cargar el laberinto desde el archivo
            if (load_maze(input, &maze) == 0) {
                //convertir el laberinto en un grafo
                //cada celda transitable se convierte en un nodo
                //las celdas adyacentes se conectan con aristas
                if (build_graph(&maze, &graph, &startIndex, &goalIndex) == 0) {
                    //verificar que exista al menos un camino entre inicio y meta
                    int *parent = calloc(graph.vertices, sizeof(int));
                    int *visitOrder = calloc(graph.vertices, sizeof(int));
                    int visitCount = 0;
                    if (parent == NULL || visitOrder == NULL) {
                        printf("No se pudo reservar memoria para validar el laberinto.\n");
                        free(parent);
                        free(visitOrder);
                        liberarGrafo(&graph);
                        mazeLoaded = 0;
                        graphReady = 0;
                        continue;
                    }
                    int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);
                    free(parent);
                    free(visitOrder);
                    
                    if (!found) {
                        printf("El laberinto no tiene camino entre I y F. Cargue otro archivo.\n");
                        liberarGrafo(&graph);
                        mazeLoaded = 0;
                        graphReady = 0;
                        continue;
                    }

                    mazeLoaded = 1;
                    graphReady = 1;
                    printf("Laberinto cargado. Dimensiones: %d x %d. Nodos: %d.\n", maze.rows, maze.cols, graph.vertices);
                    printf("Inicio (I): indice %d, Meta (F): indice %d.\n", startIndex, goalIndex);
                } else {
                    mazeLoaded = 0;
                    graphReady = 0;
                }
            }
        } else if (option == 2) {
            //OEjecutar BFS en el laberinto cargado
            if (!mazeLoaded) {
                printf("Primero cargue un laberinto valido.\n");
                continue;
            }
            
            //reservar memoria para los arreglos auxiliares de BFS
            int *parent = calloc(graph.vertices, sizeof(int));       //para reconstruir el camino
            int *visitOrder = calloc(graph.vertices, sizeof(int));   //orden de exploracion
            int visitCount = 0;
            int *pathSeq = calloc(graph.vertices, sizeof(int));      //secuencia start->goal para mostrar
            
            if (parent == NULL || visitOrder == NULL || pathSeq == NULL) {
                printf("No se pudo reservar memoria para BFS.\n");
                free(parent);
                free(visitOrder);
                free(pathSeq);
                continue;
            }
            
            //ejecutar el algoritmo BFS
            int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);
            
            //mostrar el orden en que se visitaron los nodos
            print_visit_order(&graph, visitOrder, visitCount);
            
            int len = -1;
            if (found) {
                //reconstruir secuencia del camino para visualizarlo
                len = build_path_sequence(parent, startIndex, goalIndex, graph.vertices, pathSeq);
                if (len > 0) {
                    //mostrar el camino encontrado en el laberinto
                    print_path_steps(&maze, &graph, parent, startIndex, goalIndex);
                    print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
                } else {
                    printf("No se pudo reconstruir el camino.\n");
                }
            } else {
                printf("No hay camino entre I y F.\n");
            }
            
            free(parent);
            free(visitOrder);
            free(pathSeq);
        } else if (option == 3) {
            // Ejecutar Dijkstra en el laberinto cargado
            if (!mazeLoaded) {
                printf("Primero cargue un laberinto valido.\n");
                continue;
            }
            
            //ejecutar el algoritmo de Dijkstra para encontrar el camino optimo
            struct Camino* camino = dijkstra(&graph, startIndex, goalIndex);
            
            if (camino != NULL) {
                printf("Dijkstra encontro un camino:\n");
                //imprimir el camino con su valor total
                imprimirCaminoDijkstra(camino);
                
                //reconstruir arreglo parent para visualizacion en el laberinto
                //el algoritmo Dijkstra retorna una estructura Camino, pero la funcion
                //de visualizacion necesita un arreglo parent (como el que usa BFS)
                int* parent = calloc(graph.vertices, sizeof(int));
                if (parent != NULL) {
                    //inicializar todos los padres en -1
                    for (int i = 0; i < graph.vertices; ++i) {
                        parent[i] = -1;
                    }
                    //reconstruir el arreglo parent desde el camino encontrado
                    //para cada nodo en el camino, su padre es el nodo anterior
                    for (int i = 1; i < camino->longitud; ++i) {
                        parent[camino->nodos[i]] = camino->nodos[i - 1];
                    }
                    //mostrar el camino en el laberinto
                    print_path_steps(&maze, &graph, parent, startIndex, goalIndex);
                    print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
                    free(parent);
                }
                
                liberarCamino(camino);
            } else {
                printf("No hay camino entre I y F.\n");
            }
        } else if (option == 4) {
            // Mostrar la matriz de adyacencia del grafo
            if (!graphReady) {
                printf("Primero cargue un laberinto valido o genere un grafo aleatorio.\n");
                continue;
            }
            //imprimir la matriz que muestra las conexiones entre nodos
            //un valor > 0 indica que hay una arista entre dos nodos
            print_adjacency_matrix(&graph);
        } else if (option == 5) {
            //OPCION 5: Generar grafo aleatorio y ejecutar BFS
            printf("Numero de nodos (2-100): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //leer el numero de nodos para el grafo
            int vertices = atoi(input);
            if (vertices < 2 || vertices > 100) {
                printf("Valor fuera de rango. Debe ser entre 2 y 100.\n");
                continue;
            }
            //leer la probabilidad de arista entre nodos
            printf("Probabilidad de arista (0-100%%): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //convertir el porcentaje a probabilidad entre 0.0 y 1.0
            double probPct = atof(input);
            if (probPct < 0.0 || probPct > 100.0) {
                printf("Probabilidad fuera de rango. Debe ser 0 a 100.\n");
                continue;
            }
            double edgeProb = probPct / 100.0;

            //generar un grafo aleatorio con el numero de nodos y probabilidad especificados
            if (generate_random_graph(&graph, vertices, edgeProb) == 0) {
                graphReady = 1;

                //seleccionar nodos de inicio y meta aleatorios
                startIndex = rand() % vertices;
                do {
                    goalIndex = rand() % vertices;
                } while (goalIndex == startIndex && vertices > 1);

                //construir representacion visual del laberinto a partir del grafo
                if (build_maze_from_graph(&graph, &maze, startIndex, goalIndex) == 0) {
                    mazeLoaded = 1;
                    printf("Laberinto generado. Dimensiones: %d x %d. Nodos: %d.\n", maze.rows, maze.cols, graph.vertices);
                    printf("Inicio (I): nodo %d, Meta (F): nodo %d.\n", startIndex, goalIndex);
                } else {
                    mazeLoaded = 0;
                    printf("No se pudo generar la representacion visual del laberinto.\n");
                    continue;
                }

                //reservar memoria para BFS
                int *parent = calloc(graph.vertices, sizeof(int));
                int *visitOrder = calloc(graph.vertices, sizeof(int));
                int visitCount = 0;

                if (parent == NULL || visitOrder == NULL) {
                    printf("No se pudo reservar memoria para BFS.\n");
                    free(parent);
                    free(visitOrder);
                    continue;
                }

                //ejecutar BFS en el grafo aleatorio
                int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);

                //mostrar resultados
                print_adjacency_matrix(&graph); //mostrar la matriz de adyacencia
                printf("BFS desde nodo %d hasta nodo %d\n", startIndex, goalIndex);

                if (found) {
                    //mostrar laberinto con animacion paso a paso
                    print_path_steps(&maze, &graph, parent, startIndex, goalIndex);
                    print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
                } else {
                    printf("No hay camino entre %d y %d.\n", startIndex, goalIndex);
                }
                free(parent);
                free(visitOrder);
            }
        } else if (option == 6) {
            // Generar grafo aleatorio y ejecutar Dijkstra
            printf("Numero de nodos (2-100): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //leer el numero de nodos para el grafo
            int vertices = atoi(input);
            if (vertices < 2 || vertices > 100) {
                printf("Valor fuera de rango. Debe ser entre 2 y 100.\n");
                continue;
            }
            //leer la probabilidad de arista entre nodos
            printf("Probabilidad de arista (0-100%%): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //convertir el porcentaje a probabilidad entre 0.0 y 1.0
            double probPct = atof(input);
            if (probPct < 0.0 || probPct > 100.0) {
                printf("Probabilidad fuera de rango. Debe ser 0 a 100.\n");
                continue;
            }
            double edgeProb = probPct / 100.0;

            //generar un grafo aleatorio
            if (generate_random_graph(&graph, vertices, edgeProb) == 0) {
                graphReady = 1;

                //seleccionar nodos de inicio y meta aleatorios
                startIndex = rand() % vertices;
                do {
                    goalIndex = rand() % vertices;
                } while (goalIndex == startIndex && vertices > 1);

                //construir representacion visual del laberinto a partir del grafo
                if (build_maze_from_graph(&graph, &maze, startIndex, goalIndex) == 0) {
                    mazeLoaded = 1;
                    printf("Laberinto generado. Dimensiones: %d x %d. Nodos: %d.\n", maze.rows, maze.cols, graph.vertices);
                    printf("Inicio (I): nodo %d, Meta (F): nodo %d.\n", startIndex, goalIndex);
                } else {
                    mazeLoaded = 0;
                    printf("No se pudo generar la representacion visual del laberinto.\n");
                    continue;
                }

                //mostrar la matriz de adyacencia del grafo generado
                print_adjacency_matrix(&graph);
                printf("Dijkstra desde nodo %d hasta nodo %d\n", startIndex, goalIndex);

                //ejecutar el algoritmo de Dijkstra
                struct Camino* camino = dijkstra(&graph, startIndex, goalIndex);
                if (camino != NULL) {
                    printf("Camino encontrado:\n");
                    //mostrar el camino y su valor total
                    imprimirCaminoDijkstra(camino);

                    //reconstruir arreglo parent para visualizacion
                    int* parent = calloc(graph.vertices, sizeof(int));
                    if (parent != NULL) {
                        //inicializar todos los padres en -1
                        for (int i = 0; i < graph.vertices; ++i) {
                            parent[i] = -1;
                        }
                        //reconstruir el arreglo parent desde el camino encontrado
                        for (int i = 1; i < camino->longitud; ++i) {
                            parent[camino->nodos[i]] = camino->nodos[i - 1];
                        }
                        //mostrar laberinto con animacion paso a paso
                        print_path_steps(&maze, &graph, parent, startIndex, goalIndex);
                        print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
                        free(parent);
                    }

                    liberarCamino(camino);
                } else {
                    printf("No hay camino entre %d y %d.\n", startIndex, goalIndex);
                }
            }
        } else {
            printf("Opcion no valida.\n");
        }
    }

    //liberar toda la memoria del grafo antes de salir
    liberarGrafo(&graph);
    printf("Saliendo...\n");
    return 0;
}
