#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

/*
E: cola de prioridad y dos indices validos dentro del heap.
S: intercambia elementos y actualiza posiciones.
R: cola no nula; indices en [0, tamanio).
*/
static void intercambiar(ColaPrioridad* cola, int i, int j) {
    NodoPrioridad tmp = cola->heap[i];
    cola->heap[i] = cola->heap[j];
    cola->heap[j] = tmp;
    cola->posiciones[cola->heap[i].vertice] = i;
    cola->posiciones[cola->heap[j].vertice] = j;
}

/*
E: cola y posicion de un elemento ya insertado.
S: flota el elemento hasta restaurar propiedad de min-heap.
R: cola consistente; idx en rango.
*/
static void flotar(ColaPrioridad* cola, int idx) {
    while (idx > 0) {
        int padre = (idx - 1) / 2;
        if (cola->heap[padre].distancia <= cola->heap[idx].distancia) {
            break;
        }
        intercambiar(cola, padre, idx);
        idx = padre;
    }
}

/*
E: cola y posicion de un elemento ya insertado.
S: hunde el elemento hasta restaurar propiedad de min-heap.
R: cola consistente; idx en rango.
.*/
static void hundir(ColaPrioridad* cola, int idx) {
    while (1) {
        int izquierdo = 2 * idx + 1;
        int derecho = 2 * idx + 2;
        int menor = idx;

        if (izquierdo < cola->tamanio && cola->heap[izquierdo].distancia < cola->heap[menor].distancia) {
            menor = izquierdo;
        }
        if (derecho < cola->tamanio && cola->heap[derecho].distancia < cola->heap[menor].distancia) {
            menor = derecho;
        }
        if (menor == idx) {
            break;
        }
        intercambiar(cola, idx, menor);
        idx = menor;
    }
}

/*
E: capacidad maxima del heap (>0).
S: puntero a cola inicializada con posiciones en -1; NULL en error.
R: memoria disponible; capacidad positiva.
.*/
ColaPrioridad* crearColaPrioridad(int capacidad) {
    if (capacidad <= 0) {
        return NULL;
    }

    ColaPrioridad* cola = (ColaPrioridad*)calloc(1, sizeof(ColaPrioridad));
    if (cola == NULL) {
        return NULL;
    }

    cola->heap = (NodoPrioridad*)calloc(capacidad, sizeof(NodoPrioridad));
    cola->posiciones = (int*)calloc(capacidad, sizeof(int));
    if (cola->heap == NULL || cola->posiciones == NULL) {
        free(cola->heap);
        free(cola->posiciones);
        free(cola);
        return NULL;
    }

    for (int i = 0; i < capacidad; ++i) {
        cola->posiciones[i] = -1;
    }
    cola->capacidad = capacidad;
    cola->tamanio = 0;
    return cola;
}

/*
E: cola valida, vertice y nueva distancia.
S: inserta vertice si no existe; si existe y distancia mejora, actualiza prioridad.
R: cola con espacio suficiente; vertice dentro de rango [0, capacidad-1].
.*/
void insertarCola(ColaPrioridad* cola, int vertice, int distancia) {
    if (cola == NULL || vertice < 0 || vertice >= cola->capacidad) {
        return;
    }

    int pos = cola->posiciones[vertice];
    if (pos != -1) {
        // Ya existe; solo hacemos decrease-key si mejora.
        if (distancia < cola->heap[pos].distancia) {
            cola->heap[pos].distancia = distancia;
            flotar(cola, pos);
        }
        return;
    }

    if (cola->tamanio >= cola->capacidad) {
        return; // sin espacio; no deberia ocurrir si se usa bien
    }

    int idx = cola->tamanio++;
    cola->heap[idx].vertice = vertice;
    cola->heap[idx].distancia = distancia;
    cola->posiciones[vertice] = idx;
    flotar(cola, idx);
}

/*
E: cola no vacia.
S: extrae y retorna el elemento con menor distancia; si esta vacia, vertice=-1.
R: heap consistente; actualizar posiciones.
.*/
NodoPrioridad extraerMinimo(ColaPrioridad* cola) {
    NodoPrioridad nulo = {-1, -1};
    if (cola == NULL || cola->tamanio == 0) {
        return nulo;
    }

    NodoPrioridad raiz = cola->heap[0];
    cola->posiciones[raiz.vertice] = -1;

    cola->tamanio--;
    if (cola->tamanio > 0) {
        cola->heap[0] = cola->heap[cola->tamanio];
        cola->posiciones[cola->heap[0].vertice] = 0;
        hundir(cola, 0);
    }

    return raiz;
}

/*
E: cola previamente creada.
S: libera memoria de heap, posiciones y la estructura.
R: cola puede ser NULL; no usar despues.
.*/
void liberarColaPrioridad(ColaPrioridad* cola) {
    if (cola == NULL) {
        return;
    }
    free(cola->heap);
    free(cola->posiciones);
    free(cola);
}

/*
E: arreglo de padres, indices inicio y fin, distancia final y numero de vertices.
S: construye estructura Camino con nodos en orden y distancia total.
R: parent tiene una cadena valida hasta inicio; memoria disponible.
.*/
static Camino* reconstruirCamino(const int* parent, int inicio, int fin, int distanciaFin, int vertices) {
    int* pila = (int*)calloc(vertices, sizeof(int));
    if (pila == NULL) {
        return NULL;
    }

    int len = 0;
    int v = fin;
    while (v != -1 && len < vertices) {
        pila[len++] = v;
        if (v == inicio) {
            break;
        }
        v = parent[v];
    }

    if (v == -1) {
        free(pila);
        return NULL; // sin camino
    }

    Camino* camino = (Camino*)calloc(1, sizeof(Camino));
    if (camino == NULL) {
        free(pila);
        return NULL;
    }
    camino->longitud = len;
    camino->distanciaTotal = distanciaFin;
    camino->nodos = (int*)calloc(len, sizeof(int));
    if (camino->nodos == NULL) {
        free(camino);
        free(pila);
        return NULL;
    }

    // invertir pila -> orden inicio ... fin
    for (int i = 0; i < len; ++i) {
        camino->nodos[i] = pila[len - 1 - i];
    }

    free(pila);
    return camino;
}

/*
E: grafo con pesos no negativos, indices inicio y fin validos.
S: retorna puntero a Camino minimo o NULL si no hay ruta/error.
R: grafo->peso cuadrada; memoria disponible; pesos >=0.
.*/
Camino* dijkstra(Grafo* grafo, int inicio, int fin) {
    if (grafo == NULL || grafo->vertices <= 0) {
        return NULL;
    }
    int n = grafo->vertices;
    if (inicio < 0 || fin < 0 || inicio >= n || fin >= n) {
        return NULL;
    }

    int* dist = (int*)calloc(n, sizeof(int));
    int* parent = (int*)calloc(n, sizeof(int));
    int* visitado = (int*)calloc(n, sizeof(int));
    if (dist == NULL || parent == NULL || visitado == NULL) {
        free(dist);
        free(parent);
        free(visitado);
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        dist[i] = INT_MAX / 4;
        parent[i] = -1;
        visitado[i] = 0;
    }
    dist[inicio] = 0;

    ColaPrioridad* cola = crearColaPrioridad(n);
    if (cola == NULL) {
        free(dist);
        free(parent);
        free(visitado);
        return NULL;
    }
    insertarCola(cola, inicio, 0);

    while (cola->tamanio > 0) {
        NodoPrioridad actual = extraerMinimo(cola);
        if (actual.vertice == -1) {
            break;
        }
        int v = actual.vertice;
        if (visitado[v]) {
            continue;
        }
        visitado[v] = 1;
        if (v == fin) {
            break;
        }

        for (int u = 0; u < n; ++u) {
            int peso = grafo->peso[v][u];
            if (peso <= 0) {
                continue; // sin arista
            }
            if (visitado[u]) {
                continue;
            }
            int nuevaDist = dist[v] + peso;
            if (nuevaDist < dist[u]) {
                dist[u] = nuevaDist;
                parent[u] = v;
                insertarCola(cola, u, nuevaDist); // decrease-key si ya estaba
            }
        }
    }

    Camino* camino = NULL;
    if (dist[fin] < INT_MAX / 8) {
        camino = reconstruirCamino(parent, inicio, fin, dist[fin], n);
    }

    liberarColaPrioridad(cola);
    free(dist);
    free(parent);
    free(visitado);
    return camino;
}

/*
E: puntero a Camino valido.
S: imprime nodos en orden y distancia total; indica si es NULL.
R: camino no nulo; longitud coherente con arreglo nodos.
.*/
void imprimirCamino(Camino* camino) {
    if (camino == NULL) {
        printf("No hay camino calculado.\n");
        return;
    }
    printf("Camino (distancia %d): ", camino->distanciaTotal);
    for (int i = 0; i < camino->longitud; ++i) {
        printf("%d", camino->nodos[i]);
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
.*/
void liberarCamino(Camino* camino) {
    if (camino == NULL) {
        return;
    }
    free(camino->nodos);
    free(camino);
}
