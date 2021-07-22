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

//------- funciones de lectura de archivo -----------//

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
  poligono_t *circulo;
  
  if(PAR_CIR != fread(parametros, sizeof(int16_t), PAR_CIR, f)){
    return NULL;
  }
  
  circulo = poligono_circular_crear(parametros[0], parametros[1], parametros[2]);
  
  return circulo;
}

static double grados_a_radianes(int a){
  return a * PI/180.0;
}

poligono_t *leer_geometria_rectangulo(FILE *f){
  int16_t parametros[PAR_GEO_RTG];
  poligono_t *p;
  
  if(PAR_GEO_RTG != fread(parametros, sizeof(int16_t), PAR_GEO_RTG, f)){
    return NULL; 
  }  
  float puntos[][2] = {{parametros[2], parametros[3]},{-parametros[2], parametros[3]},{-parametros[2], -parametros[3]},{parametros[2], -parametros[3]}};
  
  for(size_t i = 0; i < 4; i++){
    puntos[i][0] /= 2.0;
    puntos[i][1] /= 2.0;
  }
  
  p = poligono_crear(puntos, 4);
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

//--------- fin de las funciones de lectura ---------//

static float producto_interno(float ax, float ay, float bx, float by){
  return ax * bx + ay * by;
}

static float distancia(float ax, float ay, float bx, float by){
  float x = bx - ax;
  float y = by - ay;
  
  return sqrtf((x * x) + (y * y));
}

void punto_mas_cercano(float x0, float y0, float x1, float y1, float xp, float yp, float *x, float *y) {
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

void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy) {
  float proy = producto_interno(norm_x, norm_y, *vx, *vy);

  if(proy >= 0){
    return;
  }
  
  *vx -= 2 * norm_x * proy;
  *vy -= 2 * norm_y * proy;

    // Además empujamos a la bola hacia afuera para que no se pegue
  *cx += norm_x * 0.1;
  *cy += norm_y * 0.1;
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
