#ifndef MODULO3_FCM_CORE_H
#define MODULO3_FCM_CORE_H

#include "registro.h"

// ==========================================================
// MÓDULO 3 — NÚCLEO MATEMÁTICO: FUZZY C-MEANS
//
// Este archivo define la interfaz del corazón matemático del 
// sistema. Aquí se declaran las funciones que ejecutan el 
// agrupamiento difuso (Fuzzy C-Means). 
// Este módulo es agnóstico: NO sabe qué representan los datos
// (si son humedad, presión, etc.), solo procesa vectores
// matemáticos (double[NUM_VARIABLES]) agrupándolos por similitud.
//
// Fórmulas matemáticas implementadas en la estructura (C):
//   1. Normalización de membresía: sum_j( mu_ij ) = 1.0
//   2. Cálculo de centroides:      c_j = sum_i(mu_ij^m * x_i) / sum_i(mu_ij^m)
//   3. Actualización membresías:   mu_ij = 1 / sum_k( (d_ij/d_ik)^(2/(m-1)) )
// ==========================================================

#define MAX_CLUSTERS 100 // Límite estricto de seguridad para evitar desbordar memoria en WebAssembly

// Inicializa la matriz de pertenencia (mu) de tamaño (n_puntos x n_clusters).
// Utiliza distribución aleatoria y garantiza que la suma de las membresías de un 
// punto dado siempre sea exactamente 1.0 (Condición base de la lógica difusa).
void fcm_inicializar_membresias(int n_puntos, int n_clusters);

// Calcula la Distancia Euclidiana (línea recta en hiperespacio n-dimensional)
// entre un vector de datos 'x' y un 'centroide'. A menor distancia, mayor similitud.
double fcm_distancia_euclidiana(const double x[NUM_VARIABLES],
                                 const double centroide[NUM_VARIABLES]);

// Recalcula matemáticamente las coordenadas ideales de los 'c' centroides basándose
// en el grado de pertenencia actual de los puntos a dicho centroide y el exponente difuso 'm'.
void fcm_calcular_centroides(const double datos[][NUM_VARIABLES],
                              int n_puntos, int n_clusters, double m);

// Recalcula el grado de pertenencia (0 a 1) de todos los puntos hacia los clusters
// evaluando las nuevas distancias geométricas contra cada centroide.
void fcm_actualizar_membresias(const double datos[][NUM_VARIABLES],
                                int n_puntos, int n_clusters, double m);

// Orquesta el Bucle Principal (Entrenamiento). 
// Ejecuta cíclicamente fcm_calcular_centroides() y fcm_actualizar_membresias()
// hasta alcanzar un estado de equilibrio matemático (cambio_maximo < epsilon) 
// o un límite de seguridad computacional (max_iter).
// Retorna cuántas iteraciones se usaron en la convergencia.
int fcm_ejecutar(const double datos[][NUM_VARIABLES], int n_puntos,
                  int n_clusters, double m, int max_iter, double epsilon);

// Ejecuta ÚNICAMENTE la función de clasificación geométrica sin mutar
// las posiciones de los centroides entrenados.
// Es la pieza vital para la fase de "Inferencia / Predicción" sobre nuevos datasets.
void fcm_actualizar_membresias_con_centroides_fijos(const double datos[][NUM_VARIABLES],
                                                      int n_puntos, int n_clusters, double m);

// ==========================================================
// GETTERS — Lectura de la memoria interna post-entrenamiento
// ==========================================================

// Retorna un float (0.0 a 1.0) indicando qué tan fuerte es la pertenencia 
// del punto 'i' hacia el cluster 'j'.
double fcm_obtener_membresia(int punto, int cluster);

// Copia las coordenadas vectoriales (NUM_VARIABLES) de un centroide hacia el buffer 'out'.
void   fcm_obtener_centroide(int cluster, double out[NUM_VARIABLES]);

// Devuelve el número total de clusters que están activos en la memoria.
int    fcm_obtener_num_clusters(void);

#endif // MODULO3_FCM_CORE_H
