#ifndef POLIGONO_H
#define POLIGONO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//struct poligono;
typedef struct poligono poligono_t;

typedef enum {COLOR_AZUL, COLOR_NARANJA, COLOR_VERDE, COLOR_GRIS} color_t;
typedef enum {MOVIMIENTO_INMOVIL, MOVIMIENTO_CIRCULAR, MOVIMIENTO_HORIZONTAL} movimiento_t;
typedef enum {GEOMETRIA_CIRCULO, GEOMETRIA_RECTANGULO, GEOMETRIA_POLIGONO} geometria_t;

poligono_t *poligono_crear(float vertices [][2], size_t n);
poligono_t *poligono_circular_crear(float cx, float cy , float r);
void poligono_destruir(poligono_t *poligono);

size_t poligono_cantidad_vertices(const poligono_t *poligono);
bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y);
poligono_t *poligono_clonar(const poligono_t *poligono);
bool poligono_agregar_vertice(poligono_t *poligono, float x, float y);

bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria); 

bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros);
bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros);
bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros);
bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros);

poligono_t *leer_geometria_circulo(FILE *f);
poligono_t *leer_geometria_rectangulo(FILE *f);
poligono_t *leer_geometria_poligono(FILE *f);
poligono_t *leer_geometria(FILE*f, geometria_t geometria);

void poligono_trasladar(poligono_t *p, float dx, float dy);
void poligono_rotar(poligono_t *p, double rad);
void poligono_rotar_centrado(poligono_t *p, float cx, float cy, float ang);

double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y);

void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy); 

#endif 
