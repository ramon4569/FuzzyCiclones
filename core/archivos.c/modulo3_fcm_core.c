#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../archivos.h/modulo3_fcm_core.h"

// ==========================================================
// MODULO 3 — NUCLEO MATEMATICO: FUZZY C-MEANS
// Implementacion completa.
// ==========================================================

// Estado interno del ultimo calculo (memoria estatica, mismo
// enfoque que el resto del proyecto)
static double g_membresias[MAX_REGISTROS][MAX_CLUSTERS];
static double g_centroides[MAX_CLUSTERS][NUM_VARIABLES];
static int    g_n_clusters = 0;
static int    g_semilla_inicializada = 0;

// -----------------------------------------------------------
// Inicializacion de membresias (aleatoria, normalizada)
// -----------------------------------------------------------
void fcm_inicializar_membresias(int n_puntos, int n_clusters) {
    if (!g_semilla_inicializada) {
        srand((unsigned int)time(NULL));
        g_semilla_inicializada = 1;
    }

    g_n_clusters = n_clusters;

    for (int i = 0; i < n_puntos; i++) {
        double suma = 0.0;
        double valores[MAX_CLUSTERS];

        // Generar un valor aleatorio positivo por cluster
        for (int j = 0; j < n_clusters; j++) {
            valores[j] = (double)rand() / (double)RAND_MAX + 0.0001; // evitar exactos 0
            suma += valores[j];
        }

        // Normalizar para que la fila sume 1.0 (restriccion de FCM, 3.2)
        for (int j = 0; j < n_clusters; j++) {
            g_membresias[i][j] = valores[j] / suma;
        }
    }
}

// -----------------------------------------------------------
// Distancia euclidiana entre un vector de datos y un centroide
// -----------------------------------------------------------
double fcm_distancia_euclidiana(const double x[NUM_VARIABLES],
    const double centroide[NUM_VARIABLES]) {
    double suma_cuadrados = 0.0;
    for (int k = 0; k < NUM_VARIABLES; k++) {
        double diff = x[k] - centroide[k];
        suma_cuadrados += diff * diff;
    }
    return sqrt(suma_cuadrados);
}

// -----------------------------------------------------------
// Recalculo de centroides:
//   c_j = sum_i( mu_ij^m * x_i ) / sum_i( mu_ij^m )
// -----------------------------------------------------------
void fcm_calcular_centroides(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    for (int j = 0; j < n_clusters; j++) {
        double numerador[NUM_VARIABLES] = { 0 };
        double denominador = 0.0;

        for (int i = 0; i < n_puntos; i++) {
            double peso = pow(g_membresias[i][j], m);
            denominador += peso;
            for (int k = 0; k < NUM_VARIABLES; k++) {
                numerador[k] += peso * datos[i][k];
            }
        }

        if (denominador > 1e-12) {
            for (int k = 0; k < NUM_VARIABLES; k++) {
                g_centroides[j][k] = numerador[k] / denominador;
            }
        }
        // Si denominador ~ 0 (ningun punto pertenece de verdad a este
        // cluster), dejamos el centroide como estaba antes en vez de
        // dividir por cero o producir NaN.
    }
}

// -----------------------------------------------------------
// Actualizacion de membresias:
//   mu_ij = 1 / sum_k( (d_ij / d_ik)^(2/(m-1)) )
// -----------------------------------------------------------
void fcm_actualizar_membresias(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    double distancias[MAX_CLUSTERS];
    const double exponente = 2.0 / (m - 1.0);

    for (int i = 0; i < n_puntos; i++) {
        int cluster_exacto = -1;

        // Calcular distancia del punto i a cada centroide
        for (int j = 0; j < n_clusters; j++) {
            distancias[j] = fcm_distancia_euclidiana(datos[i], g_centroides[j]);
            if (distancias[j] < 1e-10) {
                cluster_exacto = j; // el punto coincide con este centroide
            }
        }

        if (cluster_exacto != -1) {
            // Caso especial: pertenencia total a un solo cluster
            for (int j = 0; j < n_clusters; j++) {
                g_membresias[i][j] = (j == cluster_exacto) ? 1.0 : 0.0;
            }
            continue;
        }

        // Caso general
        for (int j = 0; j < n_clusters; j++) {
            double suma = 0.0;
            for (int k = 0; k < n_clusters; k++) {
                suma += pow(distancias[j] / distancias[k], exponente);
            }
            g_membresias[i][j] = 1.0 / suma;
        }
    }
}

// -----------------------------------------------------------
// Ciclo completo hasta convergencia
// -----------------------------------------------------------
int fcm_ejecutar(const double datos[][NUM_VARIABLES], int n_puntos,
    int n_clusters, double m, int max_iter, double epsilon) {
    fcm_inicializar_membresias(n_puntos, n_clusters);

    double membresias_previas[MAX_REGISTROS][MAX_CLUSTERS];
    int iteracion;

    for (iteracion = 0; iteracion < max_iter; iteracion++) {
        // Guardar el estado anterior para poder medir el cambio
        memcpy(membresias_previas, g_membresias, sizeof(g_membresias));

        fcm_calcular_centroides(datos, n_puntos, n_clusters, m);
        fcm_actualizar_membresias(datos, n_puntos, n_clusters, m);

        // Cambio maximo entre esta iteracion y la anterior
        double cambio_maximo = 0.0;
        for (int i = 0; i < n_puntos; i++) {
            for (int j = 0; j < n_clusters; j++) {
                double diff = fabs(g_membresias[i][j] - membresias_previas[i][j]);
                if (diff > cambio_maximo) {
                    cambio_maximo = diff;
                }
            }
        }

        if (cambio_maximo < epsilon) {
            iteracion++; // contar esta iteracion antes de cortar
            break;
        }
    }

    return iteracion;
}

// -----------------------------------------------------------
// Prediccion: recalcular membresias SIN mover los centroides
// -----------------------------------------------------------
void fcm_actualizar_membresias_con_centroides_fijos(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    // Es la misma cuenta que fcm_actualizar_membresias(): esta funcion
    // NUNCA llama a fcm_calcular_centroides antes, por eso los
    // centroides (entrenados previamente) quedan fijos. Se deja como
    // funcion separada para que el Modulo 4 no tenga forma de
    // "equivocarse" y mover los centroides sin darse cuenta.
    fcm_actualizar_membresias(datos, n_puntos, n_clusters, m);
}

// -----------------------------------------------------------
// Getters
// -----------------------------------------------------------
double fcm_obtener_membresia(int punto, int cluster) {
    if (punto < 0 || punto >= MAX_REGISTROS || cluster < 0 || cluster >= g_n_clusters) {
        return 0.0;
    }
    return g_membresias[punto][cluster];
}

void fcm_obtener_centroide(int cluster, double out[NUM_VARIABLES]) {
    if (cluster < 0 || cluster >= g_n_clusters) {
        memset(out, 0, sizeof(double) * NUM_VARIABLES);
        return;
    }
    memcpy(out, g_centroides[cluster], sizeof(double) * NUM_VARIABLES);
}

int fcm_obtener_num_clusters(void) {
    return g_n_clusters;
}