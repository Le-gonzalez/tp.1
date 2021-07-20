#include "obstaculo.h"
#include "poligono.h"
#include "config.h"

#include <stdlib.h>
#include <string.h>

#define MAX_PARAMETROS 4

#define DT (1.0 / JUEGO_FPS)

struct obstaculo {
  color_t color;
  movimiento_t mov;
  geometria_t geo;
  poligono_t *cuerpo;
  size_t n_parametros; 
  int16_t *parametros_mov;
  bool marca;
};

uint8_t color[][3] = {
  [COLOR_NARANJA] = {0xff, 0x80, 0x00},
  [COLOR_AZUL] = {0x00, 0x00, 0xff},
  [COLOR_VERDE] = {0x00, 0xff, 0x00},
  [COLOR_GRIS] = {0x9b, 0x9b, 0x9b}
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
  
  int16_t *parametros_mov = malloc(sizeof(int16_t) * n_parametros);
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
  
void obstaculo_mover_horizontal(obstaculo_t *obs, double dt){
  
    
  if((obs->parametros_mov[1] <= 0)  && (obs->parametros_mov[2] < 0) ){
    obs->parametros_mov[2] *= -1;
  }
  
  if((obs->parametros_mov[1] >= obs->parametros_mov[0] ) && (obs->parametros_mov[2] > 0)){
    obs->parametros_mov[2] *= -1;
  } 
 // if(! ((obs->parametros_mov[1] >= 0) && (obs->parametros_mov[1] <= obs->parametros_mov[0]))) 
   // obs->parametros_mov[2] *= -1;
  
  
  obs->parametros_mov[1] += obs->parametros_mov[2] * dt;
  poligono_trasladar(obs->cuerpo, obs->parametros_mov[2] * dt, 0);     
}

void obstaculo_mover_circular(obstaculo_t *obs, double dt){
  poligono_t *p = poligono_clonar(obs->cuerpo);
   
  poligono_rotar_centrado(p, obs->parametros_mov[0], obs->parametros_mov[1], obs->parametros_mov[2] * dt);
  poligono_destruir(obs->cuerpo);
  obs->cuerpo = p;
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

int main (int argc, char *argv[]){
  if(argc != 2) {
    fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
    return 1;
  }

  FILE *f = fopen(argv[1], "rb");
  if(f == NULL) {
    fprintf(stderr, "No pudo abrirse \"%s\"\n", argv[1]);
    return 1;
  }
  
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "Peggle");

 int dormir = 0;
  
  unsigned int ticks = SDL_GetTicks();
  int16_t obstaculos;
  if(! fread(&obstaculos, sizeof(int16_t), 1, f))
    return 1;
  obstaculo_t *obs[obstaculos];
  
  for(size_t i = 0; i < obstaculos; i++){
    obs[i] = obstaculo_leer(f);
  }
  while(1){
     if(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
    }
    
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
        
    for(size_t i = 0; i < obstaculos; i++){
     // obstaculo_dibujar(renderer, obs[i]);
      obstaculo_mover(obs[i], DT);
      obstaculo_dibujar(renderer, obs[i]);
    }
    
      SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
        SDL_RenderDrawLine(renderer, MIN_X, MIN_Y, MAX_X, MIN_Y);
        SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MAX_X, MAX_Y);
        SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MIN_X, MIN_Y);
        SDL_RenderDrawLine(renderer, MAX_X, MAX_Y, MAX_X, MIN_Y);
     
      SDL_RenderPresent(renderer);
        ticks = SDL_GetTicks() - ticks;
        if(dormir) {
            SDL_Delay(dormir);
            dormir = 0;
        }
        else if(ticks < 1000 / JUEGO_FPS)
            SDL_Delay(1000 / JUEGO_FPS - ticks);
        ticks = SDL_GetTicks();
    }
  
  
   SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();
    fclose(f);
    for(size_t i = 0; i < obstaculos; i++){
      obstaculo_destruir(obs[i]);
    }
  return 0;
}
