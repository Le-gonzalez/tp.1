#include "poligono.h"

#include <math.h>
#include <stdlib.h>
#include "config.h"

struct poligono {
  float (*vertices)[2];
  size_t n;
};

poligono_t *poligono_crear(float vertices[][2], size_t n){
  poligono_t *pol = malloc(sizeof(poligono_t));
  if(pol == NULL){
    return NULL;
  }
  
  if(n != 0){  
    pol->vertices = malloc(2 * n * sizeof(float));
    if (pol->vertices == NULL){
      free(pol);
      return NULL;
    } 
    
    for(size_t i = 0; i < n; i++){
      pol->vertices[i][0] = vertices[i][0];
      pol->vertices[i][1] = vertices[i][1];
    }
  }
  else {
    pol->vertices = NULL; 
  }
  
  pol->n = n;
  
  return pol;
}

void poligono_destruir(poligono_t *poligono){
  free(poligono -> vertices);
  free(poligono);
}

size_t poligono_cantidad_vertices(const poligono_t *poligono){
  return poligono -> n;
}

bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y){
  if((poligono -> n <= pos))
    return false;
    
  *x = poligono -> vertices[pos][0]; 
  *y = poligono -> vertices[pos][1];
  
  return true;
}

poligono_t *poligono_clonar(const poligono_t *poligono){
  return poligono_crear(poligono -> vertices, poligono -> n);
}

bool poligono_agregar_vertice(poligono_t *poligono, float x, float y){
  float (*aux)[2] = realloc(poligono -> vertices, (poligono -> n + 1) * 2 *sizeof(float));
  if(aux == NULL){
    return false;  
  }
  aux[poligono -> n][0] = x;
  aux[poligono -> n][1] = y; 
  
  poligono -> vertices = aux;
  poligono -> n++;
  return true;   
}

poligono_t *poligono_circular_crear(float cx, float cy , float r){
  float puntos[20][2];
  
  for(size_t i = 0; i < 20; i++){
    puntos[i][0] = r * cos(i * PI/10.0);
    puntos[i][1] = r * sin(i * PI/10.0);
  }
  
  poligono_t *circulo = poligono_crear(puntos, 20);
  if(circulo == NULL){
    return NULL;
  }
   
  poligono_trasladar(circulo, cx, cy);
  return circulo; 
}

void poligono_trasladar(poligono_t *p, float dx, float dy) {
  for (size_t i =0; i < p->n ; i++){
    p->vertices[i][0] += dx;
    p->vertices[i][1] += dy;         
  }             
}
     
void poligono_rotar(poligono_t *p, double rad) {     
   float rotx;
   float roty;
   for (size_t i = 0; i < p->n ; i++){           
     rotx = p->vertices[i][0] * cos(rad) - p->vertices[i][1] * sin(rad);
     roty = p->vertices[i][0] * sin(rad) + p->vertices[i][1] * cos(rad);
     p->vertices[i][0] = rotx;
     p->vertices[i][1] = roty;
     }         
} 

void poligono_rotar_centrado(poligono_t *p, float cx, float cy, float ang){
  poligono_trasladar(p, -cx, -cy);
  poligono_rotar(p, ang);
  poligono_trasladar(p, cx, cy);
}

static float producto_interno(float ax, float ay, float bx, float by){
  return ax * bx + ay * by;
}

static float distancia(float ax, float ay, float bx, float by){
  float x = bx - ax;
  float y = by - ay;
  
  return sqrtf((x * x) + (y * y));
}

static void punto_mas_cercano(float x0, float y0, float x1, float y1, float xp, float yp, float *x, float *y) {
  float ax = xp - x0;
  float ay = yp - y0;
  float bx = x1 - x0;
  float by = y1 - y0;

  float alfa = producto_interno(ax, ay, bx, by) / producto_interno(bx, by, bx, by);

  if(alfa <= 0) {
    *x = x0;
    *y = y0;
  }
  else if(alfa >= 1) {
    *x = x1;
    *y = y1;
  }
  else {
    *x = alfa * bx + x0;
    *y = alfa * by + y0;
  }
}


double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y) {
  double d = 1 / 0.0;
  size_t idx = 0;

  for(size_t i = 0; i < p->n; i++) {
    float xi, yi;
    punto_mas_cercano(p->vertices[i][0], p->vertices[i][1], p->vertices[(i + 1) % p->n][0], p->vertices[(i + 1) % p->n][1], xp, yp, &xi, &yi);
    double di = distancia(xp, yp, xi, yi);

    if(di < d) {
      d = di;
      idx = i;
    }
  }

  float nx = p->vertices[(idx + 1) % p->n][1] - p->vertices[idx][1];
  float ny = p->vertices[idx][0] - p->vertices[(idx + 1) % p->n][0];
  float dn = distancia(nx, ny, 0, 0);

  nx /= dn;
  ny /= dn;

  *nor_x = nx;
  *nor_y = ny;

  return d;
}
