#ifndef OBSTACULO_H
#define OBSTACULO_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "poligono.h"

typedef enum {COLOR_AZUL, COLOR_NARANJA, COLOR_VERDE, COLOR_GRIS, COLOR_AMARILLO, COLOR_BLANCO} color_t;
typedef enum {MOVIMIENTO_INMOVIL, MOVIMIENTO_CIRCULAR, MOVIMIENTO_HORIZONTAL} movimiento_t;
typedef enum {GEOMETRIA_CIRCULO, GEOMETRIA_RECTANGULO, GEOMETRIA_POLIGONO} geometria_t;

typedef struct obstaculo obstaculo_t;

obstaculo_t *obstaculo_crear(color_t color, movimiento_t mov, geometria_t geo, poligono_t *cuerpo, size_t n_parameros, int16_t *parametros);
void obstaculo_destruir(obstaculo_t *obs);

color_t obstaculo_color(obstaculo_t *obs);
movimiento_t obstaculo_movimiento(obstaculo_t *obs);
geometria_t obstaculo_geometria(obstaculo_t *obs);

//lee un byte del archivo y devuelve por la interfaz los valores correspondientes de color, movimiento y geometria 
bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria); 

//lee del archivo la cantidad de parametros corespondientes al movimiento y devuelve por la interfaz la cantidad de parametros leidos y dichos parametros 
bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros);
bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros);
bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros);
//recibe un archivo y un movimiento y lee los parametros de acuerdo a ese movimiento, devuelve la cantidad de parametros y dichos parametros 
bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros);

//lee del archivo los datos correspondientes a la geometria y crea un poligono con dichos datos
poligono_t *leer_geometria_circulo(FILE *f);
poligono_t *leer_geometria_rectangulo(FILE *f);
poligono_t *leer_geometria_poligono(FILE *f);
//recibe un archivo y una geometria y de acuerdo a la geometria lee el archivo y crea el poligono 
poligono_t *leer_geometria(FILE*f, geometria_t geometria);

// devuelve un obstaculo creado a partir de lo leido en el archivo
obstaculo_t *obstaculo_leer(FILE *f);

void obstaculo_dibujar(SDL_Renderer *renderer, obstaculo_t *obs);

//aplica el movimiento al obstaculo
void obstaculo_mover_horizontal(obstaculo_t *obs, double dt);
void obstaculo_mover_circular(obstaculo_t *obs, double dt);
void obstaculo_no_mover(obstaculo_t *obs, double dt);
void obstaculo_mover(obstaculo_t *obs, double dt);

bool obstaculo_esta_marcado(obstaculo_t *obs);
void obstaculo_marcar(obstaculo_t *obs);

void obstaculo_cambiar_color(obstaculo_t *obs, color_t nuevo);

// devuelve la distancia de un punto a un obstaculo por el nombre y las componentes de la normal entre ambos por la interfaz
double obstaculo_distancia(const obstaculo_t *obs, float xp, float yp, float *nor_x, float *nor_y);

#endif 

