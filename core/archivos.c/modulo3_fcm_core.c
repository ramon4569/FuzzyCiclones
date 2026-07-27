#include <math.h>
#include <stdlib.h>
#include "../archivos.h/modulo3_fcm_core.h"

// Estado interno del ultimo calculo (mismo enfoque "memoria estatica"
// que el resto del proyecto)
static double g_membresias[MAX_REGISTROS][MAX_CLUSTERS];
static double g_centroides[MAX_CLUSTERS][NUM_VARIABLES];
static int    g_n_clusters = 0;

void fcm_inicializar_membresias(int n_puntos, int n_clusters) {
    // TODO:
    // 1. guardar n_clusters en g_n_clusters
    // 2. para cada punto i, generar n_clusters valores aleatorios
    //    (rand()) y normalizarlos para que sumen 1.0 (restriccion
    //    fundamental de FCM — ver 3.2 del temario)
    // 3. guardar el resultado en g_membresias[i][*]
}

double fcm_distancia_euclidiana(const double x[NUM_VARIABLES],
                                 const double centroide[NUM_VARIABLES]) {
    // TODO: distancia euclidiana clasica:
    // sqrt( sum_k( (x[k] - centroide[k])^2 ) )
    return 0.0;
}

void fcm_calcular_centroides(const double datos[][NUM_VARIABLES],
                              int n_puntos, int n_clusters, double m) {
    // TODO: para cada cluster j, calcular:
    //   c_j = sum_i( mu_ij^m * x_i ) / sum_i( mu_ij^m )
    // Recorrer todos los puntos, acumular numerador (vector) y
    // denominador (escalar) por cluster, y guardar en g_centroides[j].
}

void fcm_actualizar_membresias(const double datos[][NUM_VARIABLES],
                                int n_puntos, int n_clusters, double m) {
    // TODO: para cada punto i y cada cluster j:
    //   mu_ij = 1 / sum_k( (d_ij / d_ik)^(2/(m-1)) )
    // Cuidado con el caso d_ik == 0 (el punto coincide exactamente
    // con un centroide): en ese caso mu de ese cluster es 1 y el
    // resto 0.
}

int fcm_ejecutar(const double datos[][NUM_VARIABLES], int n_puntos,
                  int n_clusters, double m, int max_iter, double epsilon) {
    // TODO:
    // 1. fcm_inicializar_membresias(n_puntos, n_clusters)
    // 2. loop hasta max_iter:
    //      a. fcm_calcular_centroides(...)
    //      b. copiar g_membresias viejo para comparar despues
    //      c. fcm_actualizar_membresias(...)
    //      d. calcular el cambio maximo |mu_nuevo - mu_viejo|
    //      e. si el cambio maximo < epsilon, cortar el loop
    // 3. retornar la cantidad de iteraciones ejecutadas
    return 0;
}

void fcm_actualizar_membresias_con_centroides_fijos(const double datos[][NUM_VARIABLES],
                                                      int n_puntos, int n_clusters, double m) {
    // TODO: identico a fcm_actualizar_membresias(), pero se llama
    // SOLA (sin fcm_calcular_centroides antes) porque los centroides
    // ya vienen fijados por un entrenamiento previo (Modulo 4).
    // Esta es la funcion clave para "predecir" sobre datos nuevos.
}

double fcm_obtener_membresia(int punto, int cluster) {
    // TODO: validar rangos y retornar g_membresias[punto][cluster]
    return 0.0;
}

void fcm_obtener_centroide(int cluster, double out[NUM_VARIABLES]) {
    // TODO: validar rango y copiar g_centroides[cluster] a out
}

int fcm_obtener_num_clusters(void) {
    return g_n_clusters;
}
