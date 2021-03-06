#include <SDL2/SDL.h>
#include <stdbool.h>

#include "lista.h"
#include "config.h"
#include "obstaculo.h"

#define DT (1.0 / JUEGO_FPS)

#ifdef TTF
#include <SDL2/SDL_ttf.h>

const uint8_t colores[][3] = {
  [COLOR_NARANJA] = {0xff, 0x80, 0x00},
  [COLOR_AZUL] = {0x00, 0x00, 0xff},
  [COLOR_VERDE] = {0x00, 0xff, 0x00},
  [COLOR_GRIS] = {0x9b, 0x9b, 0x9b},
  [COLOR_AMARILLO] = {0xff, 0xff, 0x00},
  [COLOR_BLANCO] = {0xff, 0xff, 0xff}
};

void escribir_texto(SDL_Renderer *renderer, TTF_Font *font, const char *s, int x, int y, color_t clr) {
  SDL_Color color = {colores[clr][0], colores[clr][1], colores[clr][2]}; 
  SDL_Surface *surface = TTF_RenderText_Solid(font, s, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = surface->w;
  rect.h = surface->h;

  SDL_RenderCopy(renderer, texture, NULL, &rect);

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}
#endif

void dibujar_bola(SDL_Renderer *renderer, float cx, float cy, float r){
  for(size_t i = 0; i < 20; i++){
    SDL_RenderDrawLine(renderer, r *cos(i * PI/10.0) + cx, r * sin(i * PI/10.0) + cy, r * cos((i + 1) * PI/10.0) +cx, r * sin((i +1) * PI/10.0) + cy);
  }
}     

double computar_velocidad(double vi, double a, double dt) {
  return vi += a * dt; 
}

double computar_posicion(double pi, double vi, double dt) {
  return pi += vi * dt;                    
}     

float producto_interno(float ax, float ay, float bx, float by){
  return ax * bx + ay * by;
}

void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy) {
  float proy = producto_interno(norm_x, norm_y, *vx, *vy);

  if(proy >= 0){
    return;
  }
  
  *vx -= 2 * norm_x * proy;
  *vy -= 2 * norm_y * proy;
   
  *cx += norm_x * 0.1;
  *cy += norm_y * 0.1;
}

void _obstaculo_destruir(void *obs){
  obstaculo_destruir(obs);
}

void establecer_multiplicador(int count, int *mult){
  if(count == 15)
    *mult = 2;
  else if(count == 10)
    *mult = 3;
  else if(count == 6)
    *mult = 5;
  else if(count == 4)
    *mult = 10;
}

int main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
    return 1;
  }

  FILE *f = fopen(argv[1], "rb");
  if(f == NULL) {
    fprintf(stderr, "No pudo abrirse \"%s\"\n", argv[1]);
    return 1;
  }
  
  SDL_Init(SDL_INIT_VIDEO);

#ifdef TTF
  TTF_Init();
  TTF_Font* font = TTF_OpenFont("FreeSansBold.ttf", 24);
#endif

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "Peggle");

  int dormir = 0;
  // BEGIN c??digo del alumno
  float canon_angulo = 0; // ??ngulo del ca????n
  bool cayendo = false;   // ??Hay bola disparada?

  float cx, cy;   // Centro de la bola
  float vx, vy;   // Velocidad de la bola
 
  int16_t obstaculos;
  int nivel = 0;
  
  unsigned int puntos = 0;
  int mult;
  char spuntos[11];
  lista_t *obs = NULL;
  lista_iter_t *li;
  
  float norm_x, norm_y;
   
  int count = 0;
    // END c??digo del alumno

  unsigned int ticks = SDL_GetTicks();
  
  while(1) {
    if(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT){
        break; 
      }
    // BEGIN c??digo del alumno
      if(event.type == SDL_MOUSEBUTTONDOWN) {
        if(! cayendo){
          cayendo = true;
        }  
      }
      else if (event.type == SDL_MOUSEMOTION) {
        canon_angulo = atan2(event.motion.x - CANON_X, event.motion.y - CANON_Y);
        if(canon_angulo > CANON_MAX){
          canon_angulo = CANON_MAX;
        }
        if(canon_angulo < -CANON_MAX){
          canon_angulo = -CANON_MAX;
        }
      }
      // END c??digo del alumno

      continue;
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);


      // BEGIN c??digo del alumno  
    
    
#ifdef TTF
    sprintf(spuntos, "%d", puntos);
    
    escribir_texto(renderer, font, "Peggle", 100, 20, COLOR_BLANCO);
    escribir_texto(renderer, font, "Puntos", 500, 5, COLOR_BLANCO);
    escribir_texto(renderer, font, spuntos, 500, 25, COLOR_BLANCO);
#endif
    if(!count){
      if(nivel == 6){
        break;
      }
      
      if(obs != NULL){
        lista_destruir(obs, _obstaculo_destruir);
      }
           
      nivel++;
      count = 25;
      puntos = 0;
      mult = 1;
         
      cayendo = false; 
         
      obs = lista_crear();
      if(obs == NULL){
        fprintf(stderr, "no se pudo crear la lista de obstaculos\n");
        return 1;
      }
  
      if(! fread(&obstaculos, sizeof(int16_t), 1, f)){
        fprintf(stderr, "no se pudo leer los obstaculos\n");
        return 1;
      }
  
      for(size_t i = 0; i < obstaculos; i++){
        obstaculo_t *obstaculo = obstaculo_leer(f);
        if(obstaculo == NULL){
          fprintf(stderr, "no se pudo leer los obstaculos\n");
          lista_destruir(obs, _obstaculo_destruir);
          return 1;
        }
      
        lista_insertar_ultimo(obs, obstaculo);
      }
          
    }
    if(cayendo) {
       // Si la bola est?? cayendo actualizamos su posici??n
      vy = computar_velocidad(vy, G, DT);
      vx *= ROZAMIENTO;
      vy *= ROZAMIENTO;
      cx = computar_posicion(cx, vx, DT);
      cy = computar_posicion(cy, vy, DT);
      
      for(
        li = lista_iter_crear(obs); 
        !lista_iter_al_final(li);
        lista_iter_avanzar(li)
     ) {
        obstaculo_t *obstaculo = lista_iter_ver_actual(li);
           
        if(obstaculo_distancia(obstaculo, cx, cy, &norm_x, &norm_y) <= BOLA_RADIO) {
          vx *= PLASTICIDAD;
          vy *= PLASTICIDAD;
          reflejar(norm_x, norm_y, &cx, &cy, &vx, &vy);
             
          if(obstaculo_color(obstaculo) != COLOR_GRIS){
            if(obstaculo_color(obstaculo) == COLOR_NARANJA){
               count--; 
               establecer_multiplicador(count, &mult);
               
               if(! obstaculo_esta_marcado(obstaculo))
                 puntos += 100 * mult;
             }
             if(! obstaculo_esta_marcado(obstaculo))
                puntos += 10 * mult;
                
             obstaculo_marcar(obstaculo);
             obstaculo_cambiar_color(obstaculo, COLOR_AMARILLO);             
          }
        }             
        if(((vx <= 1) && (vx >= -1)) && ((vy <= 1) && (vy >= -1)) && obstaculo_esta_marcado((obstaculo))){
          obstaculo_destruir(lista_iter_borrar(li));
        }
      }
      
    lista_iter_destruir(li);
    }
    else {
     // Si la bola no se dispar?? establecemos condiciones iniciales
      cx = CANON_X + CANON_LARGO * sin(canon_angulo);
      cy = CANON_Y + CANON_LARGO * cos(canon_angulo);
      vx = BOLA_VI * sin(canon_angulo);
      vy = BOLA_VI * cos(canon_angulo);
        
      for(
        li = lista_iter_crear(obs); 
        !lista_iter_al_final(li);
        lista_iter_avanzar(li)
      ) {
        obstaculo_t *obstaculo = lista_iter_ver_actual(li);
        if(obstaculo_esta_marcado(obstaculo)){
          obstaculo_destruir(lista_iter_borrar(li));
        }
      } 
        lista_iter_destruir(li); 
    }

      // Rebote contra las paredes:
    if(cx < MIN_X + BOLA_RADIO || cx > MAX_X - BOLA_RADIO) vx = - vx;
    if(cy < MIN_Y + BOLA_RADIO) vy = -vy;

     // Se sali?? de la pantalla:
     if(cy > MAX_Y - BOLA_RADIO){
       cayendo = false;
     }          
       // Dibujamos el ca????n:
     SDL_RenderDrawLine(renderer, CANON_X, CANON_Y, CANON_X + sin(canon_angulo) * CANON_LARGO, CANON_Y + cos(canon_angulo) * CANON_LARGO);

     // Dibujamos la bola
     dibujar_bola(renderer, cx, cy, BOLA_RADIO);

      // Dibujamos las paredes:
     SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
     SDL_RenderDrawLine(renderer, MIN_X, MIN_Y, MAX_X, MIN_Y);
     SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MAX_X, MAX_Y);
     SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MIN_X, MIN_Y);
     SDL_RenderDrawLine(renderer, MAX_X, MAX_Y, MAX_X, MIN_Y);

     // Dibujamos el vector de velocidad:
     SDL_RenderDrawLine(renderer, cx, cy, cx + vx, cy + vy);
            
     for(
        li = lista_iter_crear(obs); 
        !lista_iter_al_final(li);
        lista_iter_avanzar(li)
     ) {
        obstaculo_t *obstaculo = lista_iter_ver_actual(li);
        obstaculo_dibujar(renderer, obstaculo);
        obstaculo_mover(obstaculo, DT);
     }            
        
     lista_iter_destruir(li);             
     // END c??digo del alumno

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

  // BEGIN c??digo del alumno
 // lista_iter_destruir(li);
  lista_destruir(obs, _obstaculo_destruir); 
  fclose(f);
    // END c??digo del alumno

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

#ifdef TTF
  TTF_CloseFont(font);
  TTF_Quit();
#endif
  SDL_Quit();
  return 0;
}

