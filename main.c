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
    printf("2) Ejecutar BFS (I -> f)\n");
    printf("3) Ejecutar Dijkstra (I -> f)\n");
    printf("4) Mostrar matriz de adyacencia\n");
    printf("5) Generar grafo aleatorio y ejecutar BFS\n");
    printf("6) Generar grafo aleatorio y ejecutar Dijkstra\n");
    printf("0) Salir\n");
    printf("> ");
}

int main(void) {
    //estructura que almacena el laberinto cargado desde archivo
    struct Maze maze = (struct Maze){0};
    
    //estructura del grafo que representa las conexiones entre celdas/nodos
    struct Grafo graph = {0};
    
    //indices del nodo de inicio (S) y meta (E) en el grafo
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
    printf("Formato de archivo: mismo numero de columnas por fila. Usa 'X' para muro, '.' o espacio para camino, 'I' inicio, 'f' meta.\n");

    for (;;) {
        show_menu();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        int option = atoi(input);

        if (option == 0) {
            break; //salir del programa
        }

        //OPCION 1: Cargar laberinto desde archivo
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
                    mazeLoaded = 1;
                    graphReady = 1;
                    printf("Laberinto cargado. Dimensiones: %d x %d. Nodos: %d.\n", maze.rows, maze.cols, graph.vertices);
                    printf("Inicio (I): indice %d, Meta (f): indice %d.\n", startIndex, goalIndex);
                } else {
                    mazeLoaded = 0;
                    graphReady = 0;
                }
            }
        } else if (option == 2) {
            //OPCION 2: Ejecutar BFS en el laberinto cargado
            if (!mazeLoaded) {
                printf("Primero cargue un laberinto valido.\n");
                continue;
            }
            
            //reservar memoria para los arreglos auxiliares de BFS
            int *parent = calloc(graph.vertices, sizeof(int));       //para reconstruir el camino
            int *visitOrder = calloc(graph.vertices, sizeof(int));   //orden de exploracion
            int visitCount = 0;
            
            if (parent == NULL || visitOrder == NULL) {
                printf("No se pudo reservar memoria para BFS.\n");
                free(parent);
                free(visitOrder);
                continue;
            }
            
            //ejecutar el algoritmo BFS
            int found = bfs(&graph, startIndex, goalIndex, parent, visitOrder, &visitCount);
            
            //mostrar el orden en que se visitaron los nodos
            print_visit_order(&graph, visitOrder, visitCount);
            
            if (found) {
                //mostrar el camino encontrado en el laberinto
                print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
            } else {
                printf("No hay camino entre I y f.\n");
            }
            
            free(parent);
            free(visitOrder);
        } else if (option == 3) {
            //OPCION 3: Ejecutar Dijkstra en el laberinto cargado
            if (!mazeLoaded) {
                printf("Primero cargue un laberinto valido.\n");
                continue;
            }
            
            //ejecutar el algoritmo de Dijkstra para encontrar el camino optimo
            struct Camino* camino = dijkstra(&graph, startIndex, goalIndex);
            
            if (camino != NULL) {
                printf("Dijkstra encontro un camino:\n");
                //imprimir el camino con su valor total
                imprimirCamino(camino);
                
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
                    print_path_on_maze(&maze, &graph, parent, startIndex, goalIndex);
                    free(parent);
                }
                
                liberarCamino(camino);
            } else {
                printf("No hay camino entre I y f.\n");
            }
        } else if (option == 4) {
            //OPCION 4: Mostrar la matriz de adyacencia del grafo
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
                printf("Valor fuera de rango.\n");
                continue;
            }
            //leer la probabilidad de arista entre nodos
            printf("Probabilidad de arista (0-100%%): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //convertir el porcentaje a probabilidad entre 0.0 y 1.0
            double probPct = atof(input);
            if (probPct < 0.0) probPct = 0.0;
            if (probPct > 100.0) probPct = 100.0;
            double edgeProb = probPct / 100.0;

            //generar un grafo aleatorio con el numero de nodos y probabilidad especificados
            if (generate_random_graph(&graph, vertices, edgeProb) == 0) {
                graphReady = 1;
                mazeLoaded = 0; // este grafo no corresponde a ningun laberinto cargado
                
                //seleccionar nodos de inicio y meta aleatorios
                startIndex = rand() % vertices;
                do {
                    goalIndex = rand() % vertices;
                } while (goalIndex == startIndex && vertices > 1);

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
                printf("BFS desde %d hasta %d\n", startIndex, goalIndex); //mostrar nodos de inicio y meta
                print_visit_order_simple(visitOrder, visitCount); //mostrar orden de visita
                if (found) {
                    print_path_indices(parent, startIndex, goalIndex, graph.vertices); //mostrar camino encontrado
                } else {
                    printf("No hay camino entre %d y %d.\n", startIndex, goalIndex); //indicar si no hay camino
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
                printf("Valor fuera de rango.\n");
                continue;
            }
            //leer la probabilidad de arista entre nodos
            printf("Probabilidad de arista (0-100%%): ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            //convertir el porcentaje a probabilidad entre 0.0 y 1.0
            double probPct = atof(input);
            if (probPct < 0.0) probPct = 0.0;
            if (probPct > 100.0) probPct = 100.0;
            double edgeProb = probPct / 100.0;  

            //generar un grafo aleatorio
            if (generate_random_graph(&graph, vertices, edgeProb) == 0) {
                graphReady = 1;
                mazeLoaded = 0; // este grafo no corresponde a ningun laberinto cargado
                
                //seleccionar nodos de inicio y meta aleatorios
                startIndex = rand() % vertices;
                do {
                    goalIndex = rand() % vertices;
                } while (goalIndex == startIndex && vertices > 1);

                //mostrar la matriz de adyacencia del grafo generado
                print_adjacency_matrix(&graph);
                printf("Dijkstra desde %d hasta %d\n", startIndex, goalIndex);
                
                //ejecutar el algoritmo de Dijkstra
                struct Camino* camino = dijkstra(&graph, startIndex, goalIndex);
                if (camino != NULL) {
                    printf("Camino encontrado:\n");
                    //mostrar el camino y su valor total
                    imprimirCamino(camino);
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
