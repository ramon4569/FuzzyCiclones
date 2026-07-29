/* ============================================================
 * modulo4_entrenamiento.c
 * ------------------------------------------------------------
 * Implementacion del Modulo 4 - Entrenamiento y prediccion.
 * Ver modulo4_entrenamiento.h para la explicacion de cada funcion.
 * ============================================================ */

#include <stddef.h>
#include <stdio.h>
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"
#include "../archivos.h/modulo3_fcm_core.h"

 /* ---- Estado interno del modulo (privado, no se expone en el .h) ---- */

static int    g_entrenado = 0;   /* 1 si entrenamiento_entrenar() ya tuvo exito */
static int    g_n_clusters = 0;   /* n_clusters usado en el entrenamiento vigente */
static double g_m = 0.0; /* exponente difuso (fuzziness) usado en el entrenamiento */
static int    g_cluster_riesgo = -1;  /* indice del cluster identificado como "riesgo", -1 = aun no calculado */
static int    g_n_prueba = 0;   /* cantidad de registros de la ultima prediccion */

/* Buffers estaticos en vez de arrays locales grandes en el stack:
 * con MAX_REGISTROS = 2000 y NUM_VARIABLES = 5, cada uno pesa
 * 2000*5*8 bytes = 80 KB, que es mejor no meter en el stack
 * (mas aun pensando en el build final con Emscripten). */
double g_vectores_entrenamiento[MAX_REGISTROS][NUM_VARIABLES];
static double g_vectores_prueba[MAX_REGISTROS][NUM_VARIABLES];
RegistroClimatico g_buffer_filtrado[MAX_REGISTROS];


int entrenamiento_entrenar(int anio_inicio, int anio_fin, int n_clusters,
    double m, int max_iter, double epsilon) {

    /* --- validacion de parametros --- */
    if (anio_inicio > anio_fin) return -1;
    if (n_clusters <= 0 || n_clusters > MAX_CLUSTERS) return -1;
    if (m <= 1.0) return -1;              /* m debe ser > 1 para que FCM tenga sentido */
    if (max_iter <= 0) return -1;
    if (epsilon <= 0.0) return -1;

    /* --- 1) traer del Modulo 1 solo los registros del rango de entrenamiento --- */
    int n_train = dataset_filtrar_por_anio(anio_inicio, anio_fin,
        g_buffer_filtrado, MAX_REGISTROS);

    if (n_train <= 0) return -1;          /* no hay datos para ese rango de anios */
    if (n_train < n_clusters) return -1;  /* no tiene sentido pedir mas clusters que puntos */

    /* --- 2) convertir cada RegistroClimatico a vector double[NUM_VARIABLES] --- */
    for (int i = 0; i < n_train; i++) {
        registro_a_vector(&g_buffer_filtrado[i], g_vectores_entrenamiento[i]);
    }

    /* --- 3) delegar el entrenamiento puro al Modulo 3 --- */
    int iteraciones = fcm_ejecutar(g_vectores_entrenamiento, n_train,
        n_clusters, m, max_iter, epsilon);

    if (iteraciones < 0) return -1;

    /* --- 4) guardar el estado de este entrenamiento --- */
    g_n_clusters = n_clusters;
    g_m = m;
    g_entrenado = 1;
    g_cluster_riesgo = -1;   /* se recalcula la proxima vez que se pida */
    g_n_prueba = 0;    /* invalida cualquier prediccion anterior */

    return iteraciones;
}


int entrenamiento_predecir(const RegistroClimatico* datos_prueba, int n) {

    if (!g_entrenado) return -1;                 /* hay que entrenar antes de predecir */
    if (datos_prueba == NULL) return -1;
    if (n <= 0 || n > MAX_REGISTROS) return -1;

    /* convertir los registros de prueba (ej. el 2017 completo) a vectores */
    for (int i = 0; i < n; i++) {
        registro_a_vector(&datos_prueba[i], g_vectores_prueba[i]);
    }

    /* IMPORTANTE: se llama la version "con centroides fijos", NUNCA
     * fcm_ejecutar. Asi los centroides entrenados con 2015-2016 no
     * se mueven un solo milimetro al ver datos de 2017. */
    fcm_actualizar_membresias_con_centroides_fijos(g_vectores_prueba, n,
        g_n_clusters, g_m);

    g_n_prueba = n;
    return n;
}


int entrenamiento_identificar_cluster_riesgo(void) {

    if (!g_entrenado) return -1;

    /* Heuristica acordada: el cluster de riesgo es el que tiene, en su
     * centroide, la combinacion mas "propicia a ciclon": SST alta y
     * presion baja. Se resume en un solo puntaje por cluster:
     *
     *     score(c) = centroide[c][SST] - centroide[c][PRESION]
     *
     * y se elige el cluster con el score mas alto.
     *
     * OJO: esto asume que SST y PRESION ya estan en escalas comparables
     * (por eso Modulo 1 expone dataset_min_max: para normalizar antes
     * de entrenar). Si el equipo decide no normalizar, esta heuristica
     * hay que ajustarla (por ejemplo estandarizando cada variable antes
     * de compararla). Queda documentado aqui a proposito para discutirlo
     * en la integracion final. */
  
    double centroide[NUM_VARIABLES];

    fcm_obtener_centroide(0, centroide);
    int mejor_cluster = 0;
    double mejor_score = centroide[VAR_SST] - centroide[VAR_PRESION];

    for (int c = 1; c < g_n_clusters; c++) {
        fcm_obtener_centroide(c, centroide);
        double score = centroide[VAR_SST] - centroide[VAR_PRESION];
        if (score > mejor_score) {
            mejor_score = score;
            mejor_cluster = c;
        }
    }

    g_cluster_riesgo = mejor_cluster;
    return g_cluster_riesgo;
}


double entrenamiento_obtener_riesgo(int index) {

    if (!g_entrenado) return -1.0;
    if (index < 0 || index >= g_n_prueba) return -1.0;

    /* si todavia nadie pidio el cluster de riesgo, lo calculamos aqui
     * (perezoso), para que el orden de llamado desde Modulo 6 sea flexible */
    if (g_cluster_riesgo < 0) {
        entrenamiento_identificar_cluster_riesgo();
    }

    return fcm_obtener_membresia(index, g_cluster_riesgo);
}

double entrenamiento_obtener_riesgo_detallado(int index, double breakdown[NUM_VARIABLES]) {
    double riesgo = entrenamiento_obtener_riesgo(index);
    
    // Inicializar breakdown a 0
    for(int i = 0; i < NUM_VARIABLES; i++) breakdown[i] = 0.0;
    
    if (riesgo < 0) return riesgo; // Error de inicializacion/index
    
    // Obtener el centroide de riesgo
    double centroide[NUM_VARIABLES];
    fcm_obtener_centroide(g_cluster_riesgo, centroide);
    
    // Calcular la contribucion de cada variable a la distancia (cuadrada)
    double distancia_total = 0.0;
    double distancias_parciales[NUM_VARIABLES];
    
    for (int k = 0; k < NUM_VARIABLES; k++) {
        double diff = g_vectores_prueba[index][k] - centroide[k];
        distancias_parciales[k] = diff * diff;
        distancia_total += distancias_parciales[k];
    }
    
    // Convertir a porcentajes (influencia relativa en acercar el punto al cluster)
    // Mientras más cerca esté (menor diferencia), mayor afinidad a esa característica.
    // Wait: si la distancia parcial es 0 (igual al centroide), su contribución a la membresía (afinidad) es máxima.
    // Por lo tanto, podemos invertir la distancia para ver la afinidad.
    // O si consideramos "qué variable nos empujó más a este cluster", es un poco ambiguo.
    // Vamos a usar la inversa de la diferencia: 1 / (diff^2 + eps).
    
    double afinidad_total = 0.0;
    double afinidades_parciales[NUM_VARIABLES];
    
    for(int k = 0; k < NUM_VARIABLES; k++) {
        afinidades_parciales[k] = 1.0 / (distancias_parciales[k] + 1e-10);
        afinidad_total += afinidades_parciales[k];
    }
    
    for(int k = 0; k < NUM_VARIABLES; k++) {
        breakdown[k] = afinidades_parciales[k] / afinidad_total;
    }
    
    return riesgo;
}