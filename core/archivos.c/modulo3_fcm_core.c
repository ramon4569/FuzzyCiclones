#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../archivos.h/modulo3_fcm_core.h"

// ==========================================================
// MÓDULO 3 — NÚCLEO MATEMÁTICO: FUZZY C-MEANS (FCM)
//
// Este archivo contiene la implementación pura del algoritmo
// de clustering difuso (Fuzzy C-Means). A diferencia del K-Means 
// tradicional donde un punto pertenece a un solo grupo, en FCM
// cada punto pertenece a TODOS los grupos con un cierto "grado 
// de membresía" (entre 0 y 1).
// ==========================================================

// Estado interno del cálculo (memoria estática para evitar
// desbordar el heap o stack en WebAssembly).
// g_membresias almacena la matriz U (grado de pertenencia de cada punto a cada cluster)
static double g_membresias[MAX_REGISTROS][MAX_CLUSTERS];
// g_centroides almacena la matriz V (coordenadas del centro de cada cluster)
static double g_centroides[MAX_CLUSTERS][NUM_VARIABLES];
static int    g_n_clusters = 0;
static int    g_semilla_inicializada = 0;

// -----------------------------------------------------------
// fcm_inicializar_membresias
// Inicializa la matriz de membresías (U) con valores aleatorios.
// Es el Paso 1 del algoritmo FCM.
// -----------------------------------------------------------
void fcm_inicializar_membresias(int n_puntos, int n_clusters) {
    if (!g_semilla_inicializada) {
        srand((unsigned int)time(NULL)); // Semilla aleatoria inicial
        g_semilla_inicializada = 1;
    }

    g_n_clusters = n_clusters;

    // Iteramos sobre todos los puntos de datos (ej. días registrados)
    for (int i = 0; i < n_puntos; i++) {
        double suma = 0.0;
        double valores[MAX_CLUSTERS];

        // Asignamos un grado aleatorio inicial al punto 'i' para cada cluster 'j'
        for (int j = 0; j < n_clusters; j++) {
            valores[j] = (double)rand() / (double)RAND_MAX + 0.0001; // Se evita el 0 absoluto
            suma += valores[j];
        }

        // Restricción fundamental de FCM: La suma de las membresías de un punto
        // hacia todos los clusters debe ser exactamente 1.0 (100%).
        // Aquí normalizamos dividiendo entre la suma total de aleatorios.
        for (int j = 0; j < n_clusters; j++) {
            g_membresias[i][j] = valores[j] / suma;
        }
    }
}

// -----------------------------------------------------------
// fcm_distancia_euclidiana
// Calcula la distancia lineal geométrica (Euclidiana) entre
// un registro de clima (x) y el centro de un cluster (v).
// Fórmula: d = sqrt( sum( (x_k - v_k)^2 ) )
// -----------------------------------------------------------
double fcm_distancia_euclidiana(const double x[NUM_VARIABLES],
    const double centroide[NUM_VARIABLES]) {
    double suma_cuadrados = 0.0;
    // Dimensiones = número de variables climáticas normalizadas (SST, presión, viento, etc.)
    for (int k = 0; k < NUM_VARIABLES; k++) {
        double diff = x[k] - centroide[k];
        suma_cuadrados += diff * diff;
    }
    return sqrt(suma_cuadrados);
}

// -----------------------------------------------------------
// fcm_calcular_centroides
// Recalcula la posición de cada centroide (cluster) basándose en 
// qué tanto le "pertenecen" los puntos.
// Es el Paso 2 del algoritmo FCM.
//
// Fórmula matemática:
//   v_j = sum_i( (u_ij ^ m) * x_i ) / sum_i( u_ij ^ m )
// Donde:
//   v_j = Nuevo centroide j
//   u_ij = Membresía del punto i en el cluster j
//   m = Exponente difuso (controla el grado de "difusidad")
//   x_i = Punto de datos i
// -----------------------------------------------------------
void fcm_calcular_centroides(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    for (int j = 0; j < n_clusters; j++) {
        double numerador[NUM_VARIABLES] = { 0 };
        double denominador = 0.0;

        for (int i = 0; i < n_puntos; i++) {
            // Peso = (u_ij)^m. Al elevar al exponente 'm' (usualmente 2.0),
            // se le resta influencia exponencialmente a los puntos con baja membresía,
            // atrayendo el centroide fuertemente hacia los puntos con alta membresía.
            double peso = pow(g_membresias[i][j], m);
            denominador += peso;
            
            // Sumatoria ponderada de cada variable climática para este centroide
            for (int k = 0; k < NUM_VARIABLES; k++) {
                numerador[k] += peso * datos[i][k];
            }
        }

        // Se reasignan las coordenadas del centroide dividiendo por la suma de pesos
        if (denominador > 1e-12) {
            for (int k = 0; k < NUM_VARIABLES; k++) {
                g_centroides[j][k] = numerador[k] / denominador;
            }
        }
        // Si el denominador es ~0, el cluster quedó "vacío" o sin influencia, 
        // por lo que se mantiene en su posición actual para evitar división por cero o NaN.
    }
}

// -----------------------------------------------------------
// fcm_actualizar_membresias
// Recalcula a qué cluster pertenece cada punto basándose en 
// las nuevas distancias a los centroides recién actualizados.
// Es el Paso 3 del algoritmo FCM.
//
// Fórmula matemática:
//   u_ij = 1 / sum_k( (d_ij / d_ik) ^ (2 / (m - 1)) )
// Donde:
//   u_ij = Nueva membresía del punto i en el cluster j
//   d_ij = Distancia del punto i al centroide j
//   d_ik = Distancia del punto i al resto de los centroides 'k'
// -----------------------------------------------------------
void fcm_actualizar_membresias(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    double distancias[MAX_CLUSTERS];
    const double exponente = 2.0 / (m - 1.0); // Factor de suavizado

    for (int i = 0; i < n_puntos; i++) {
        int cluster_exacto = -1;

        // Primero calculamos qué tan lejos está el punto 'i' de todos los centroides
        for (int j = 0; j < n_clusters; j++) {
            distancias[j] = fcm_distancia_euclidiana(datos[i], g_centroides[j]);
            // Prevención de división por cero si un punto cae EXACTAMENTE en un centroide
            if (distancias[j] < 1e-10) {
                cluster_exacto = j;
            }
        }

        // Caso especial (Singularidad matemática): 
        // El punto colisionó matemáticamente con el centro del cluster.
        // Se le asigna 100% de membresía a ese cluster y 0% al resto.
        if (cluster_exacto != -1) {
            for (int j = 0; j < n_clusters; j++) {
                g_membresias[i][j] = (j == cluster_exacto) ? 1.0 : 0.0;
            }
            continue;
        }

        // Caso general (Puntos dispersos):
        // La pertenencia es inversamente proporcional a la distancia relativa
        // frente a todos los demás clusters. Si estás muy cerca del cluster j, d_ij será
        // muy pequeño, el denominador (suma) será pequeño y la membresía u_ij será muy cercana a 1.
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
// fcm_ejecutar
// Controla el ciclo iterativo principal (Bucle de Entrenamiento).
// Repite el paso 2 (actualizar centros) y 3 (actualizar membresías)
// hasta que los grupos dejen de moverse (Convergencia) o se agoten 
// los intentos (max_iter).
// -----------------------------------------------------------
int fcm_ejecutar(const double datos[][NUM_VARIABLES], int n_puntos,
    int n_clusters, double m, int max_iter, double epsilon) {
    
    // 1. Inicialización de la matriz difusa
    fcm_inicializar_membresias(n_puntos, n_clusters);

    static double membresias_previas[MAX_REGISTROS][MAX_CLUSTERS];
    int iteracion;

    for (iteracion = 0; iteracion < max_iter; iteracion++) {
        // Guardamos las membresías de la iteración anterior para poder evaluar el progreso
        memcpy(membresias_previas, g_membresias, sizeof(g_membresias));

        // 2. Mover los centroides para que se adapten a la nube de datos difusa
        fcm_calcular_centroides(datos, n_puntos, n_clusters, m);
        // 3. Recalcular quién pertenece a quién, tras el movimiento de los centroides
        fcm_actualizar_membresias(datos, n_puntos, n_clusters, m);

        // 4. Test de convergencia: Se evalúa qué tanto cambió el grado de membresía.
        double cambio_maximo = 0.0;
        for (int i = 0; i < n_puntos; i++) {
            for (int j = 0; j < n_clusters; j++) {
                // Buscamos el diferencial más alto de cambio en toda la matriz
                double diff = fabs(g_membresias[i][j] - membresias_previas[i][j]);
                if (diff > cambio_maximo) {
                    cambio_maximo = diff;
                }
            }
        }

        // Si el mayor movimiento fue menor que el umbral 'epsilon' (ej. 0.0001), 
        // significa que el modelo es estable y se detiene el entrenamiento prematuramente.
        if (cambio_maximo < epsilon) {
            iteracion++; // Contabiliza la última iteración completada exitosamente
            break;
        }
    }

    return iteracion; // Retorna cuántas vueltas requirió para converger
}

// -----------------------------------------------------------
// fcm_actualizar_membresias_con_centroides_fijos
// Usado exclusivamente en PREDICCIÓN (Fase de Inferencia). 
// Cuando el modelo recibe datos totalmente nuevos, no debe modificar
// los patrones climáticos (centroides) que ya aprendió. 
// Simplemente mide a qué patrón se parecen los nuevos datos.
// -----------------------------------------------------------
void fcm_actualizar_membresias_con_centroides_fijos(const double datos[][NUM_VARIABLES],
    int n_puntos, int n_clusters, double m) {
    // Reutiliza la función del paso 3. Al no llamar al paso 2 (calcular centroides), 
    // la red se utiliza en modo "Lectura/Clasificación" sin mutar su memoria.
    fcm_actualizar_membresias(datos, n_puntos, n_clusters, m);
}

// ==========================================================
// GETTERS (Extractores de estado del algoritmo)
// Permiten a otros módulos acceder a los resultados sin 
// exponer directamente los arreglos globales de memoria.
// ==========================================================

// Extrae el grado de pertenencia (0.0 a 1.0) del punto 'i' al cluster 'j'
double fcm_obtener_membresia(int punto, int cluster) {
    if (punto < 0 || punto >= MAX_REGISTROS || cluster < 0 || cluster >= g_n_clusters) {
        return 0.0; // Prevención de desbordamientos de memoria (Out-of-bounds)
    }
    return g_membresias[punto][cluster];
}

// Copia las coordenadas climáticas normalizadas del centroide solicitado
void fcm_obtener_centroide(int cluster, double out[NUM_VARIABLES]) {
    if (cluster < 0 || cluster >= g_n_clusters) {
        memset(out, 0, sizeof(double) * NUM_VARIABLES);
        return;
    }
    memcpy(out, g_centroides[cluster], sizeof(double) * NUM_VARIABLES);
}

// Indica la cantidad de clusters (grupos) con los que fue entrenado el modelo
int fcm_obtener_num_clusters(void) {
    return g_n_clusters;
}