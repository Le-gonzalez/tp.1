#include "lista.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct nodo {
    void *dato;
    struct nodo *prox;
} nodo_t;

struct lista {
    nodo_t *primero;
    nodo_t *ultimo;
    size_t largo;
};

struct lista_iter {
    lista_t *lista;
    nodo_t *ant;
    nodo_t *act;
};

lista_t *lista_crear() {
    lista_t *lista = malloc(sizeof(lista_t));

    if (!lista)
        return NULL;

    lista->primero = NULL;
    lista->ultimo = NULL;
    lista->largo = 0;

    return lista;
}

bool lista_esta_vacia(const lista_t *lista) {
    return lista && lista->largo == 0;
}

static nodo_t *crear_nodo(void *dato, nodo_t *prox) {
    nodo_t *nuevo = malloc(sizeof(nodo_t));

    if (!nuevo)
        return NULL;

    nuevo->dato = dato;
    nuevo->prox = prox;
    return nuevo;
}

bool lista_insertar_primero(lista_t *lista, void *dato) {
    nodo_t *nuevo = crear_nodo(dato, lista->primero);

    if (!nuevo)
        return false;

    lista->largo++;
    lista->primero = nuevo;

    if (!lista->ultimo)
        lista->ultimo = nuevo;

    return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato) {
    nodo_t *nuevo = crear_nodo(dato, NULL);

    if (!nuevo)
        return false;

    if (lista->ultimo)
        lista->ultimo->prox = nuevo;

    lista->ultimo = nuevo;

    if (!lista->primero)
        lista->primero = nuevo;

    lista->largo++;

    return true;
}

void *lista_borrar_primero(lista_t *lista) {
    if (!lista->primero)
        return NULL;

    nodo_t *aux = lista->primero;
    lista->primero = aux->prox;
    void *dato = aux->dato;
    free(aux);

    if (lista->ultimo == aux)
        lista->ultimo = NULL;

    lista->largo--;

    return dato;
}

void *lista_ver_primero(const lista_t *lista) {
    if (lista_esta_vacia(lista))
        return NULL;

    return lista->primero->dato;
}

void *lista_ver_ultimo(const lista_t *lista) {
    if (lista_esta_vacia(lista))
        return NULL;

    return lista->ultimo->dato;
}

size_t lista_largo(const lista_t *lista) {
    return lista->largo;
}

void lista_destruir(lista_t *lista, void destruir_dato(void *)) {
    while (lista->primero) {
        nodo_t *nodo = lista->primero;
        if (destruir_dato)
            destruir_dato(nodo->dato);
        lista->primero = nodo->prox;
        free(nodo);
    }
    free(lista);
}


lista_iter_t *lista_iter_crear(lista_t *lista) {
    lista_iter_t *iter = malloc(sizeof(lista_iter_t));

    if (!iter)
        return NULL;

    iter->lista = lista;
    iter->ant = NULL;
    iter->act = lista->primero;

    return iter;
}

bool lista_iter_avanzar(lista_iter_t *iter) {
    if (lista_iter_al_final(iter))
        return false;

    iter->ant = iter->act;
    iter->act = iter->act->prox;

    return true;
}

void *lista_iter_ver_actual(const lista_iter_t *iter) {
    if (lista_iter_al_final(iter))
        return NULL;

    return iter->act->dato;
}

bool lista_iter_al_final(const lista_iter_t *iter) {
    return iter->act == NULL;
}

void lista_iter_destruir(lista_iter_t *iter) {
    free(iter);
}


bool lista_iter_insertar(lista_iter_t *iter, void *dato) {
    nodo_t *nuevo = crear_nodo(dato, iter->act);

    if (!nuevo)
        return false;

    // Primera posición
    if (iter->ant == NULL) {
        iter->lista->primero = nuevo;
    }
    else {
        iter->ant->prox = nuevo;
    }

    // Última posición
    if (iter->act == NULL) {
        iter->lista->ultimo = nuevo;
    }
    iter->act = nuevo;
    iter->lista->largo++;

    return true;
}

void *lista_iter_borrar(lista_iter_t *iter) {
    if (!iter->act)
        return NULL;

    nodo_t *nodo = iter->act;
    void *dato = nodo->dato;

    if (!iter->ant) {
        iter->lista->primero = nodo->prox;
    }
    else {
        iter->ant->prox = nodo->prox;
    }

    if (!nodo->prox)
        iter->lista->ultimo = iter->ant;

    iter->act = nodo->prox;
    iter->lista->largo--;
    free(nodo);

    return dato;
}


void lista_iterar(lista_t *lista,
                  bool (*visitar)(void *, void *),
                  void *extra) {
    nodo_t *nodo = lista->primero;
    while (nodo) {
        if (!visitar(nodo->dato, extra))
            return;
        nodo = nodo->prox;
    }
}
