#ifndef POLIGONO_H
#define POLIGONO_H

#include <stdbool.h>
#include <stddef.h>

typedef struct poligono poligono_t;

poligono_t *poligono_crear(float vertices [][2], size_t n);
poligono_t *poligono_circular_crear(float cx, float cy , float r);
void poligono_destruir(poligono_t *poligono);

size_t poligono_cantidad_vertices(const poligono_t *poligono);
bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y); 
poligono_t *poligono_clonar(const poligono_t *poligono);
bool poligono_agregar_vertice(poligono_t *poligono, float x, float y);

void poligono_trasladar(poligono_t *p, float dx, float dy);
void poligono_rotar(poligono_t *p, double rad);
void poligono_rotar_centrado(poligono_t *p, float cx, float cy, float ang);

//recibe un poligono y un coordenadas x e y de un punto devuelve por el nombre la distancia del poligono a ellos, y por la interfaz los valores de la normal entre el punto y el poligono 
double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y);

#endif 
