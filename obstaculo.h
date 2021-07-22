#ifndef OBSTACULO_H
#define OBSTACULO_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include "poligono.h"

//struct obstaculo;
typedef struct obstaculo obstaculo_t;

color_t obstaculo_color(obstaculo_t *obs);
movimiento_t obstaculo_movimiento(obstaculo_t *obs);
geometria_t obstaculo_geometria(obstaculo_t *obs);

void obstaculo_dibujar(SDL_Renderer *renderer, obstaculo_t *obs);

obstaculo_t *obstaculo_crear(color_t color, movimiento_t mov, geometria_t geo, poligono_t *cuerpo, size_t n_parameros, int16_t *parametros);
obstaculo_t *obstaculo_leer(FILE *f);
void obstaculo_destruir(obstaculo_t *obs);

void obstaculo_mover_horizontal(obstaculo_t *obs, double dt);
void obstaculo_mover_circular(obstaculo_t *obs, double dt);
void obstaculo_no_mover(obstaculo_t *obs, double dt);
void obstaculo_mover(obstaculo_t *obs, double dt);

bool obstaculo_esta_marcado(obstaculo_t *obs);
void obstaculo_marcar(obstaculo_t *obs);

void obstaculo_cambiar_color(obstaculo_t *obs, color_t nuevo);

double obstaculo_distancia(const obstaculo_t *obs, float xp, float yp, float *nor_x, float *nor_y);

#endif 

