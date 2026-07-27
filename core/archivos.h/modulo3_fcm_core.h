#ifndef MODULO3_FCM_CORE_H
#define MODULO3_FCM_CORE_H

#include "registro.h"

// ==========================================================
// MODULO 3 — NUCLEO MATEMATICO: FUZZY C-MEANS
// Responsable: (asignar integrante) — el corazon matematico del proyecto
//
// Implementa el algoritmo puro (ver Parte 3 del temario: 3.2 grado de
// pertenencia, 3.4 el algoritmo). No sabe de donde vienen los datos
// climaticos ni que significan sst/presion/etc: solo recibe vectores
// double[NUM_VARIABLES] y trabaja con ellos.
//
// Formulas de referencia:
//   Membresia:   sum_j( mu_ij ) = 1           para cada punto i
//   Centroide:   c_j = sum_i(mu_ij^m * x_i) / sum_i(mu_ij^m)
//   Actualizar:  mu_ij = 1 / sum_k( (d_ij/d_ik)^(2/(m-1)) )
// ==========================================================

#define MAX_CLUSTERS 6

// Inicializa la matriz de pertenencia (mu) de tamano n_puntos x n_clusters
// con valores aleatorios, normalizando cada fila para que sume 1.0
// (restriccion fundamental de FCM, ver 3.2 del temario).
void fcm_inicializar_membresias(int n_puntos, int n_clusters);

// Distancia euclidiana entre un vector de datos x y un centroide,
// ambos de dimension NUM_VARIABLES.
double fcm_distancia_euclidiana(const double x[NUM_VARIABLES],
                                 const double centroide[NUM_VARIABLES]);

// Recalcula los c centroides en base a la matriz de membresia actual.
void fcm_calcular_centroides(const double datos[][NUM_VARIABLES],
                              int n_puntos, int n_clusters, double m);

// Recalcula la matriz de pertenencia (mu) en base a los centroides actuales.
void fcm_actualizar_membresias(const double datos[][NUM_VARIABLES],
                                int n_puntos, int n_clusters, double m);

// Ejecuta el ciclo completo (calcular centroides + actualizar membresias)
// repitiendo hasta que el cambio maximo en mu sea menor a epsilon, o se
// llegue a max_iter. Retorna la cantidad de iteraciones realmente ejecutadas.
int fcm_ejecutar(const double datos[][NUM_VARIABLES], int n_puntos,
                  int n_clusters, double m, int max_iter, double epsilon);

// Corre UNICAMENTE el paso de actualizar_membresias, sin tocar los
// centroides. Es la pieza clave para "predecir" sobre datos nuevos
// (ej. 2017) usando centroides ya fijados por el entrenamiento (Modulo 4).
void fcm_actualizar_membresias_con_centroides_fijos(const double datos[][NUM_VARIABLES],
                                                      int n_puntos, int n_clusters, double m);

// GETTERS — estado interno del ultimo calculo
double fcm_obtener_membresia(int punto, int cluster);
void   fcm_obtener_centroide(int cluster, double out[NUM_VARIABLES]);
int    fcm_obtener_num_clusters(void);

#endif // MODULO3_FCM_CORE_H
