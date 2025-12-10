#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

//imprime el estado actual de las distancias y visitados en Dijkstra
static void print_estado_dijkstra(int paso, int actual, const int *val, const int *visitado, int n) {
    printf("\nPaso %d: procesamos el nodo %d\n", paso, actual);
    printf("Distancias despues de relajar vecinos:\n");

    for (int i = 0; i < n; ++i) {
        char dist[16];
        if (val[i] >= INT_MAX / 8) {
            snprintf(dist, sizeof(dist), "inf");
        } else {
            snprintf(dist, sizeof(dist), "%d", val[i]);
        }

        const char* estado;
        if (visitado[i]) {
            estado = "visitado";
        } else if (val[i] >= INT_MAX / 8) {
            estado = "sin ruta";
        } else {
            estado = "pendiente";
        }

        const char* marca = (i == actual) ? " <- actual" : "";
        printf("  %2d) dist=%-7s estado=%s%s\n", i, dist, estado, marca);
    }
}

//FUNCIONES PARA EL HEAP (COLA DE PRIORIDAD)

/*
E: cola de prioridad y dos indices validos dentro del heap
S: void (intercambia elementos y actualiza posiciones)
R: cola no nula; indices menores al tamano
*/
static void intercambiar(struct ColaPrioridad* cola, int i, int j) {
    //validar restricciones
    if (cola == NULL || i < 0 || j < 0 || i >= cola->tamano || j >= cola->tamano) {
        return;
    }

    //intercambia los elementos en las posiciones i y j del heap
    struct NodoPrioridad tmp = cola->heap[i];
    cola->heap[i] = cola->heap[j];
    cola->heap[j] = tmp;

    //actualiza posiciones
    cola->posiciones[cola->heap[i].vertice] = i;
    cola->posiciones[cola->heap[j].vertice] = j;
}

/*
E: cola y posicion de un elemento ya insertado
S: sube el elemento hasta restaurar propiedad de min-heap
R: cola existe, indice en rango
*/
static void subir(struct ColaPrioridad* cola, int idx) {
    //validar restricciones
    if (cola == NULL || idx < 0 || idx >= cola->tamano) {
        return;
    }

    //sube el elemento mientras su valor sea menor que el de su padre
    while (idx > 0) {
        int padre = (idx - 1) / 2; //calcular el indice del padre
        
        //si el padre ya tiene un valor menor o igual, la propiedad del heap se cumple
        if (cola->heap[padre].valor <= cola->heap[idx].valor) {
            break;
        }
        
        //intercambiar con el padre y seguir subiendo
        intercambiar(cola, padre, idx);
        idx = padre;
    }
}

/*
E: cola y posicion de un elemento ya insertado
S: baja el elemento hasta restaurar propiedad de min-heap
R: cola existe, indice en rango
*/
static void bajar(struct ColaPrioridad* cola, int idx) {
    //validar restricciones
    if (cola == NULL || idx < 0 || idx >= cola->tamano) {
        return;
    }

    //baja el elemento mientras tenga un hijo con valor menor
    while (1) {
        int izquierdo = 2 * idx + 1; //calcular indice del hijo izquierdo
        int derecho = 2 * idx + 2;   //calcular indice del hijo derecho
        int menor = idx; //asumimos que el actual es el menor

        //verificar si el hijo izquierdo existe y tiene un valor menor
        if (izquierdo < cola->tamano && cola->heap[izquierdo].valor < cola->heap[menor].valor) {
            menor = izquierdo;
        }
        
        //verificar si el hijo derecho existe y tiene un valor aun menor
        if (derecho < cola->tamano && cola->heap[derecho].valor < cola->heap[menor].valor) {
            menor = derecho;
        }
        
        //si el elemento actual ya es el menor, la propiedad del heap se cumple
        if (menor == idx) {
            break;
        }
        
        //intercambiar con el hijo menor y seguir bajando
        intercambiar(cola, idx, menor);
        idx = menor;
    }
}

/*
E: capacidad maxima del heap
S: puntero a cola inicializada con posiciones en -1 o NULL en error
R: memoria disponible; capacidad mayor a 0
*/
struct ColaPrioridad* crearColaPrioridad(int capacidad) {
    //validaciones
    if (capacidad <= 0) {
        return NULL;
    }

    //reservar memoria para la estructura principal de la cola
    struct ColaPrioridad* cola = calloc(1, sizeof(struct ColaPrioridad));
    if (cola == NULL) {
        return NULL;
    }

    //reservar memoria para el heap (arreglo de nodos con prioridad)
    cola->heap = calloc(capacidad, sizeof(struct NodoPrioridad));
    
    //reservar memoria para el arreglo que rastrea donde esta cada vertice en el heap
    cola->posiciones = calloc(capacidad, sizeof(int));

    //si fallo alguna asignacion de memoria, liberar todo y retornar NULL
    if (cola->heap == NULL || cola->posiciones == NULL) {
        free(cola->heap);
        free(cola->posiciones);
        free(cola);
        return NULL;
    }

    //inicializar todas las posiciones en -1 (indica que el vertice no esta en el heap)
    for (int i = 0; i < capacidad; ++i) {
        cola->posiciones[i] = -1;
    }
    
    //establecer la capacidad maxima y el tamano inicial en 0
    cola->capacidad = capacidad;
    cola->tamano = 0;
    return cola;
}

/*
E: cola valida, vertice y nuevo valor
S: inserta vertice si no existe; si existe y valor mejora, actualiza prioridad
R: cola con espacio suficiente; vertice dentro de rango [0, capacidad-1]
*/
void insertarCola(struct ColaPrioridad* cola, int vertice, int valor) {
    //validar restricciones
    if (cola == NULL || vertice < 0 || vertice >= cola->capacidad) {
        return;
    }

    //obtener la posicion actual del vertice en el heap
    int pos = cola->posiciones[vertice];
    
    //si el vertice ya esta en el heap
    if (pos != -1) {
        //solo actualizar si el nuevo valor es menor (mejora el camino)
        if (valor < cola->heap[pos].valor) {
            cola->heap[pos].valor = valor; //actualizar el valor
            subir(cola, pos); //restaurar la propiedad del heap subiendo el elemento
        }
        return;
    }

    //verificar que haya espacio en el heap
    if (cola->tamano >= cola->capacidad) {
        return; // sin espacio; no deberia ocurrir si se usa bien
    }

    //insertar el nuevo vertice al final del heap
    int idx = cola->tamano++;
    cola->heap[idx].vertice = vertice;
    cola->heap[idx].valor = valor;
    cola->posiciones[vertice] = idx; //registrar donde esta el vertice
    
    //subir el elemento para mantener la propiedad de min-heap
    subir(cola, idx);
}

/*
E: cola no vacia.
S: extrae y retorna el elemento con menor valor; si esta vacia, vertice=-1.
R: heap existe; actualizar posiciones.
*/
struct NodoPrioridad extraerMinimo(struct ColaPrioridad* cola) {
    //nodo nulo para retornar en caso de error
    struct NodoPrioridad nulo = {-1, -1};
    
    //validar restricciones
    if (cola == NULL || cola->tamano == 0) {
        return nulo;
    }

    //guardar el elemento de la raiz (el de menor valor)
    struct NodoPrioridad raiz = cola->heap[0];
    
    //marcar que este vertice ya no esta en el heap
    cola->posiciones[raiz.vertice] = -1;

    //reducir el tamano del heap
    cola->tamano--;
    
    //si todavia quedan elementos en el heap
    if (cola->tamano > 0) {
        //mover el ultimo elemento a la raiz
        cola->heap[0] = cola->heap[cola->tamano];
        cola->posiciones[cola->heap[0].vertice] = 0; //actualizar su posicion
        
        //bajar este elemento para restaurar la propiedad de min-heap
        bajar(cola, 0);
    }

    return raiz;
}

/*
E: cola previamente creada.
S: libera memoria de heap, posiciones y la estructura.
R: cola puede ser NULL, no usar despues.
*/
void liberarColaPrioridad(struct ColaPrioridad* cola) {
    if (cola == NULL) {
        return;
    }
    //liberar el arreglo del heap
    free(cola->heap);
    //liberar el arreglo de posiciones
    free(cola->posiciones);
    //liberar la estructura principal
    free(cola);
}

/*
E: arreglo de padres, indices inicio y fin, valor final y numero de vertices.
S: construye estructura Camino con nodos en orden y valor total.
R: parent tiene una cadena valida hasta inicio, memoria disponible.
.*/
static struct Camino* reconstruirCamino(const int* parent, int inicio, int fin, int valorFin, int vertices) {
    //validar restricciones
    if (parent == NULL || vertices <= 0 || inicio < 0 || fin < 0 || inicio >= vertices || fin >= vertices) {
        return NULL;
    }

    //usar una pila temporal para guardar el camino (se guarda al reves)
    int* pila = calloc(vertices, sizeof(int));
    if (pila == NULL) {
        return NULL;
    }

    //reconstruir el camino desde fin hasta inicio usando el arreglo de padres
    int len = 0;
    int v = fin;
    while (v != -1 && len < vertices) {
        pila[len++] = v; //agregar el vertice actual a la pila
        
        if (v == inicio) {
            break; //llegamos al inicio, terminar
        }
        
        v = parent[v]; //moverse al padre
    }

    //si no llegamos al inicio, no hay camino valido
    if (v == -1) {
        free(pila);
        return NULL; // sin camino
    }

    //crear la estructura Camino para retornar
    struct Camino* camino = calloc(1, sizeof(struct Camino));
    if (camino == NULL) {
        free(pila);
        return NULL;
    }
    
    //establecer la longitud y el valor total del camino
    camino->longitud = len;
    camino->valorTotal = valorFin;
    
    //reservar memoria para el arreglo de nodos
    camino->nodos = calloc(len, sizeof(int));
    if (camino->nodos == NULL) {
        free(camino);
        free(pila);
        return NULL;
    }

    //invertir la pila para obtener el orden correcto: inicio ... fin
    for (int i = 0; i < len; ++i) {
        camino->nodos[i] = pila[len - 1 - i];
    }

    free(pila);
    return camino;
}

/*
Calcula el camino mas corto entre dos nodos usando el algoritmo de Dijkstra.
E: grafo con pesos no negativos, indices inicio y fin validos.
S: retorna puntero a Camino minimo o NULL si no hay ruta/error.
R: grafo->peso cuadrada, memoria disponible; pesos >=0.
*/
struct Camino* dijkstra(struct Grafo* grafo, int inicio, int fin) {
    //validar restricciones basicas
    if (grafo == NULL || grafo->vertices <= 0 || grafo->peso == NULL) {
        return NULL;
    }
    int n = grafo->vertices;
    if (inicio < 0 || fin < 0 || inicio >= n || fin >= n) {
        return NULL;
    }

    //validar que grafo->peso sea una matriz cuadrada valida y pesos no negativos
    for (int i = 0; i < n; ++i) {
        if (grafo->peso[i] == NULL) {
            return NULL;
        }
        for (int j = 0; j < n; ++j) {
            if (grafo->peso[i][j] < 0) {
                return NULL; //pesos negativos no permitidos
            }
        }
    }

    //reservar memoria para los arreglos auxiliares
    int* val = calloc(n, sizeof(int));         //valores acumulados minimos a cada vertice
    int* parent = calloc(n, sizeof(int));      //para reconstruir el camino
    int* visitado = calloc(n, sizeof(int));    //vertices ya procesados
    
    if (val == NULL || parent == NULL || visitado == NULL) {
        free(val);
        free(parent);
        free(visitado);
        return NULL;
    }

    //inicializar todos los valores a infinito (excepto el inicio)
    for (int i = 0; i < n; ++i) {
        val[i] = INT_MAX / 4; //valor muy grande (infinito)
        parent[i] = -1; //sin padre todavia
        visitado[i] = 0; //no visitado
    }
    val[inicio] = 0; //el valor para llegar al inicio es 0

    //crear la cola de prioridad
    struct ColaPrioridad* cola = crearColaPrioridad(n);
    if (cola == NULL) {
        free(val);
        free(parent);
        free(visitado);
        return NULL;
    }
    
    //insertar el vertice de inicio en la cola
    insertarCola(cola, inicio, 0);

    int paso = 1; //contador de pasos para imprimir estados

    //mientras haya vertices en la cola
    while (cola->tamano > 0) {
        //extraer el vertice con menor valor acumulado
        struct NodoPrioridad actual = extraerMinimo(cola);
        if (actual.vertice == -1) {
            break; //error o cola vacia
        }
        
        int v = actual.vertice;
        
        //si ya visitamos este vertice, ignorarlo
        if (visitado[v]) {
            continue;
        }
        
        //marcar como visitado
        visitado[v] = 1;

        //explorar todos los vecinos del vertice actual
        for (int u = 0; u < n; ++u) {
            int peso = grafo->peso[v][u];
            
            //si no hay arista, continuar
            if (peso <= 0) {
                continue; // sin arista
            }
            
            //si ya visitamos este vecino, ignorarlo
            if (visitado[u]) {
                continue;
            }
            
            //calcular el nuevo valor acumulado pasando por v
            int nuevoVal = val[v] + peso;
            
            //si encontramos un camino mejor al vecino u
            if (nuevoVal < val[u]) {
                val[u] = nuevoVal;  //actualizar el valor minimo
                parent[u] = v;      //registrar que llegamos a u desde v
                
                //insertar o actualizar el vertice en la cola
                //si ya estaba, la funcion insertarCola reduce su valor si mejora
                insertarCola(cola, u, nuevoVal);
            }
        }

        //imprimir estado intermedio despues de relajar vecinos
        print_estado_dijkstra(paso++, v, val, visitado, n);
        
        //si llegamos al destino, podemos terminar
        if (v == fin) {
            break;
        }
    }

    //reconstruir el camino si se encontro una ruta valida
    struct Camino* camino = NULL;
    if (val[fin] < INT_MAX / 8) {
        camino = reconstruirCamino(parent, inicio, fin, val[fin], n);
    }

    //liberar memoria
    liberarColaPrioridad(cola);
    free(val);
    free(parent);
    free(visitado);
    return camino;
}

/*
E: puntero a Camino valido.
S: imprime nodos en orden y valor total; indica si es NULL.
R: camino no nulo; longitud coherente con arreglo nodos.
*/
void imprimirCamino(struct Camino* camino) {
    //validaciones
    if (camino == NULL) {
        printf("No hay camino calculado.\n");
        return;
    }

    if (camino->nodos == NULL || camino->longitud <= 0) {
        printf("Error: camino invalido (nodos NULL o longitud <= 0).\n");
        return;
    }
    
    //imprimir el valor total del camino
    printf("Camino (valor %d): ", camino->valorTotal);
    
    //imprimir cada nodo del camino
    for (int i = 0; i < camino->longitud; ++i) {
        printf("%d", camino->nodos[i]);
        
        //imprimir flecha si no es el ultimo nodo
        if (i + 1 < camino->longitud) {
            printf(" -> ");
        }
    }
    printf("\n");
}

/*
E: puntero a Camino previamente creado por dijkstra.
S: libera arreglo de nodos y la estructura.
R: camino puede ser NULL; no usar despues.
*/
void liberarCamino(struct Camino* camino) {
    if (camino == NULL) {
        return;
    }
    //liberar el arreglo de nodos
    free(camino->nodos);
    //liberar la estructura del camino
    free(camino);
}
