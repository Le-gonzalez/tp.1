#include "obstaculo.h"
#include "poligono.h"

#include <stdlib.h>
#include <string.h>

#define MAX_PARAMETROS 4

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define CLR_MSK 0xC0
#define MOV_MSK 0x30
#define GMT_MSK 0x0f

#define MIN_VAL 0x00
#define MAX_VAL 0x02

#define SHIFT_CLR 6
#define SHIFT_MOV 4

#define PAR_CIR 3
#define PAR_MOV_HZT 3
#define PAR_GEO_RTG 5

struct obstaculo {
  color_t color;
  movimiento_t mov;
  geometria_t geo;
  poligono_t *cuerpo;
  size_t n_parametros; 
  float *parametros_mov;
  bool marca;
};

uint8_t color[][3] = {
  [COLOR_NARANJA] = {0xff, 0x80, 0x00},
  [COLOR_AZUL] = {0x00, 0x00, 0xff},
  [COLOR_VERDE] = {0x00, 0xff, 0x00},
  [COLOR_GRIS] = {0x9b, 0x9b, 0x9b},
  [COLOR_AMARILLO] = {0xff, 0xff, 0x00}
};

bool (*movimientos[])(FILE*, int16_t*, size_t*) = {
  [MOVIMIENTO_INMOVIL] = leer_movimiento_inmovil,
  [MOVIMIENTO_CIRCULAR] = leer_movimiento_circular,
  [MOVIMIENTO_HORIZONTAL] = leer_movimiento_horizontal,
};

poligono_t *(*geometrias[])(FILE*) = {
  [GEOMETRIA_CIRCULO] = leer_geometria_circulo,
  [GEOMETRIA_RECTANGULO] = leer_geometria_rectangulo, 
  [GEOMETRIA_POLIGONO] = leer_geometria_poligono
};

void (*mover[])(obstaculo_t *obs, double dt) = {
  [MOVIMIENTO_INMOVIL] = obstaculo_no_mover,
  [MOVIMIENTO_HORIZONTAL] = obstaculo_mover_horizontal,
  [MOVIMIENTO_CIRCULAR] = obstaculo_mover_circular
};

obstaculo_t *obstaculo_crear(color_t color, movimiento_t mov, geometria_t geo, poligono_t *cuerpo, size_t n_parametros, int16_t *parametros){
  obstaculo_t *obstaculo = malloc(sizeof(obstaculo_t));
  if(obstaculo == NULL){
    return NULL;
  }
  
  float *parametros_mov = malloc(sizeof(float) * n_parametros);
  if(parametros_mov == NULL){
    free(obstaculo);
    return NULL;
  }
 
  for(size_t i = 0; i < n_parametros; i++){
    parametros_mov[i] = parametros[i];
  }
  
  obstaculo->color = color;
  obstaculo->mov = mov;
  obstaculo->geo = geo;
  obstaculo->cuerpo = cuerpo;
  obstaculo->n_parametros = n_parametros;
  obstaculo->parametros_mov = parametros_mov;
  obstaculo->marca = false;
  
  return obstaculo;
}

void obstaculo_destruir(obstaculo_t *obs){
  poligono_destruir(obs->cuerpo);
  free(obs->parametros_mov);
  free(obs);
}

void obstaculo_dibujar(SDL_Renderer *renderer, obstaculo_t *obs){
  size_t n = poligono_cantidad_vertices(obs->cuerpo);
  SDL_SetRenderDrawColor(renderer, color[obs->color][0], color[obs->color][1], color[obs->color][2], 0x00);
  
  float x1, y1, x2, y2;
  
  for(size_t i = 1; i < n; i++){
    poligono_obtener_vertice(obs->cuerpo, i, &x1, &y1);
    poligono_obtener_vertice(obs->cuerpo, i - 1, &x2, &y2); 
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
  }
  
  poligono_obtener_vertice(obs->cuerpo, n - 1, &x1, &y1);
  poligono_obtener_vertice(obs->cuerpo, 0, &x2, &y2);
  SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
  
}

color_t obstaculo_color(obstaculo_t *obs){
  return obs->color;
}

movimiento_t obstaculo_movimiento(obstaculo_t *obs){
  return obs->mov;
}

geometria_t obstaculo_geometria(obstaculo_t *obs){
  return obs->geo;
}

//------- funciones de lectura de archivo -----------//


bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria){
  int8_t encabezado;
  if(fread(&encabezado, sizeof(int8_t), 1, f) != 1) return false;
  
  *color = (encabezado & CLR_MSK) >> SHIFT_CLR;
  *movimiento = (encabezado & MOV_MSK) >> SHIFT_MOV;
  *geometria = encabezado & GMT_MSK;
  
  return (*movimiento >= MIN_VAL && *movimiento <= MAX_VAL) && (*geometria >= MIN_VAL && *geometria <= MAX_VAL);
}

bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros){
  *n_parametros = 0;
  return true;
}

bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros){
  *n_parametros = PAR_CIR;
  return fread(parametros, sizeof(int16_t), *n_parametros, f) == PAR_CIR;
}

bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros){
  *n_parametros = PAR_MOV_HZT;
  return fread(parametros, sizeof(int16_t), *n_parametros, f) == PAR_MOV_HZT;
}

bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros){
  return movimientos[movimiento](f, parametros, n_parametros);
}   

poligono_t *leer_geometria_circulo(FILE *f){
  int16_t parametros[PAR_CIR];
  
  if(PAR_CIR != fread(parametros, sizeof(int16_t), PAR_CIR, f)){
    return NULL;
  }
  
  poligono_t *circulo = poligono_circular_crear(parametros[0], parametros[1], parametros[2]);
  
  return circulo;
}

static double grados_a_radianes(int a){
  return a * PI/180.0;
}

poligono_t *leer_geometria_rectangulo(FILE *f){
  int16_t parametros[PAR_GEO_RTG];
  
  if(PAR_GEO_RTG != fread(parametros, sizeof(int16_t), PAR_GEO_RTG, f)){
    return NULL; 
  }  
  float puntos[][2] = {{parametros[2], parametros[3]},{-parametros[2], parametros[3]},{-parametros[2], -parametros[3]},{parametros[2], -parametros[3]}};
  
  for(size_t i = 0; i < 4; i++){
    puntos[i][0] /= 2.0;
    puntos[i][1] /= 2.0;
  }
  
  poligono_t *p = poligono_crear(puntos, 4);
  if( p == NULL){
    return NULL;
  }
    
  poligono_rotar(p, grados_a_radianes(parametros[4])); 
  poligono_trasladar(p, parametros[0], parametros[1]);
  
  return p;
}

poligono_t *leer_geometria_poligono(FILE *f){
  int16_t puntos;
  if(fread(&puntos, sizeof(int16_t), 1, f) != 1) return NULL;
  
  poligono_t *p = poligono_crear(NULL, 0);
  if(p == NULL){
    return NULL;
  }
   
  int16_t parametros[2];
    
  for(size_t i = 0; i < puntos; i++){
    if(fread(parametros, sizeof(int16_t), 2, f) != 2){
      poligono_destruir(p);
      return NULL;
    }
    poligono_agregar_vertice(p, parametros[0], parametros[1]);
  }
  
  return p;
}

poligono_t *leer_geometria(FILE*f, geometria_t geometria){
  return geometrias[geometria](f);
} 


obstaculo_t *obstaculo_leer(FILE *f){
  color_t color;
  movimiento_t movimiento;
  geometria_t geometria;
  
  if(! leer_encabezado(f, &color, &movimiento, &geometria))
    return NULL;
  
  size_t n_parametros_m;
  int16_t parametros_mov[MAX_PARAMETROS];
  
  if(! leer_movimiento(f, movimiento, parametros_mov, &n_parametros_m))
    return NULL;
    
  poligono_t *p = leer_geometria(f, geometria);
  if(p == NULL) 
    return NULL;
  
  obstaculo_t *obs = obstaculo_crear(color, movimiento, geometria, p, n_parametros_m, parametros_mov);
  if(obs == NULL) {
    poligono_destruir(p);
    return NULL;
  }
  
  return obs;
}
  //--------- fin de las funciones de lectura ---------//

void obstaculo_mover_horizontal(obstaculo_t *obs, double dt){
  
  if((obs->parametros_mov[1] >= obs->parametros_mov[0] ) && (obs->parametros_mov[2] > 0)) {
    obs->parametros_mov[2] *= -1;
  } 
    
  if((obs->parametros_mov[1] <= 0)  && (obs->parametros_mov[2] < 0) ){
    obs->parametros_mov[2] *= -1;
  } 
 
  poligono_trasladar(obs->cuerpo, obs->parametros_mov[2] * dt, 0);
  obs->parametros_mov[1] += obs->parametros_mov[2] * dt;
       
}

void obstaculo_mover_circular(obstaculo_t *obs, double dt){
  poligono_rotar_centrado(obs->cuerpo, obs->parametros_mov[0], obs->parametros_mov[1], obs->parametros_mov[2] * dt);
} 

void obstaculo_no_mover(obstaculo_t *obs, double dt){
}

void obstaculo_mover(obstaculo_t *obs, double dt){
  mover[obs->mov](obs, dt); 
}      

bool obstaculo_esta_marcado(obstaculo_t *obs){
  return obs->marca;
}

void obstaculo_marcar(obstaculo_t *obs){
  obs->marca = true;
}

void obstaculo_cambiar_color(obstaculo_t *obs, color_t nuevo){
  obs->color = nuevo;
}

double obstaculo_distancia(const obstaculo_t *obs, float xp, float yp, float *nor_x, float *nor_y){
  return poligono_distancia(obs->cuerpo, xp, yp, nor_x, nor_y);
}

