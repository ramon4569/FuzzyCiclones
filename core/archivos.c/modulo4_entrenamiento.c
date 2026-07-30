/* ============================================================
 * MÓDULO 4: ENTRENAMIENTO Y PREDICCIÓN
 * ------------------------------------------------------------
 * Este módulo actúa como puente de alto nivel entre los datos 
 * climáticos puros (Módulo 1/2) y el núcleo matemático abstracto (Módulo 3).
 * Se encarga de traducir los Registros Climáticos a vectores que
 * el algoritmo FCM pueda procesar, ejecutar el entrenamiento, 
 * identificar qué cluster representa el "Peligro de Ciclón", 
 * y realizar inferencias sobre datos futuros.
 * ============================================================ */

#include <stddef.h>
#include <stdio.h>
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"
#include "../archivos.h/modulo3_fcm_core.h"

 /* ---- Estado interno del modulo (privado, persistente en RAM) ---- */

static int    g_entrenado = 0;        /* Flag de seguridad: 1 si el modelo ya aprendió patrones */
static int    g_n_clusters = 0;       /* Cantidad de clusters configurados en el último entrenamiento */
static double g_m = 0.0;              /* Nivel de difusidad configurado en el último entrenamiento */
static int    g_cluster_riesgo = -1;  /* ID (0 a N) del cluster identificado como "Huracán", -1 si no se ha buscado aún */
static int    g_n_prueba = 0;         /* Cantidad de registros en la cola actual de predicción (Inferencia) */

/* 
 * Memoria estática (Buffers globales)
 * En lugar de reservar grandes arreglos locales dentro de las funciones (lo cual desborda 
 * fácilmente el 'Stack' de WebAssembly, que es limitado), declaramos arreglos globales
 * fijos que viven en el segmento Data/BSS del módulo. 
 * Tamaño de cada arreglo: MAX_REGISTROS * NUM_VARIABLES * 8 bytes = ~80 KB.
 */
double g_vectores_entrenamiento[MAX_REGISTROS][NUM_VARIABLES];
static double g_vectores_prueba[MAX_REGISTROS][NUM_VARIABLES];
RegistroClimatico g_buffer_filtrado[MAX_REGISTROS];

// -----------------------------------------------------------
// entrenamiento_entrenar
// Ejecuta el flujo completo de aprendizaje automático.
// Toma los datos crudos del rango de años indicado, los vectoriza
// y llama al Módulo 3 para que descubra los patrones subyacentes.
// -----------------------------------------------------------
int entrenamiento_entrenar(int anio_inicio, int anio_fin, int n_clusters,
    double m, int max_iter, double epsilon) {

    /* --- Validación inicial de hiperparámetros --- */
    if (anio_inicio > anio_fin) return -1;
    if (n_clusters <= 0 || n_clusters > MAX_CLUSTERS) return -1;
    if (m <= 1.0) return -1;              /* m <= 1 anula la lógica difusa y provoca división por cero en FCM */
    if (max_iter <= 0) return -1;
    if (epsilon <= 0.0) return -1;

    /* --- 1) Filtrado de Datos (Dataset -> Entrenamiento) --- */
    // Traemos de la base de datos (Módulo 1) solo los registros correspondientes a la ventana de tiempo.
    int n_train = dataset_filtrar_por_anio(anio_inicio, anio_fin,
        g_buffer_filtrado, MAX_REGISTROS);

    if (n_train <= 0) return -1;          /* Error: El usuario no ha cargado datos para estos años */
    if (n_train < n_clusters) return -1;  /* Matemáticamente inválido buscar más clusters que puntos disponibles */

    /* --- 2) Vectorización --- */
    // Traducimos el struct de 'RegistroClimatico' a un array puramente numérico (double[NUM_VARIABLES])
    for (int i = 0; i < n_train; i++) {
        registro_a_vector(&g_buffer_filtrado[i], g_vectores_entrenamiento[i]);
    }

    /* --- 3) Ejecución del Algoritmo Matemático --- */
    // Pasamos la pesada carga computacional al Módulo 3 (FCM)
    int iteraciones = fcm_ejecutar(g_vectores_entrenamiento, n_train,
        n_clusters, m, max_iter, epsilon);

    if (iteraciones < 0) return -1; // Fallo crítico del algoritmo

    /* --- 4) Guardado de estado post-entrenamiento --- */
    // Registramos que el modelo ya es capaz de predecir
    g_n_clusters = n_clusters;
    g_m = m;
    g_entrenado = 1;
    g_cluster_riesgo = -1;   /* Reiniciamos la heurística: el cluster de riesgo se recalcula bajo demanda */
    g_n_prueba = 0;          /* Purgamos cualquier predicción vieja que haya quedado en memoria */

    return iteraciones;
}

// -----------------------------------------------------------
// entrenamiento_predecir
// Pone al modelo a prueba. Toma datos "nuevos" (ej. año 2017)
// y sin modificar lo que ya aprendió, evalúa qué tanto se
// parecen a los patrones descubiertos en el entrenamiento.
// -----------------------------------------------------------
int entrenamiento_predecir(const RegistroClimatico* datos_prueba, int n) {

    if (!g_entrenado) return -1;                 /* Excepción: Se intentó predecir antes de entrenar */
    if (datos_prueba == NULL) return -1;
    if (n <= 0 || n > MAX_REGISTROS) return -1;

    /* 1) Transformamos los nuevos registros de evaluación a vectores (double array) */
    for (int i = 0; i < n; i++) {
        registro_a_vector(&datos_prueba[i], g_vectores_prueba[i]);
    }

    /* 2) Clasificación Difusa sobre Centroides Fijos */
    /* IMPORTANTE: Usamos la variante especial de lectura (sin calcular centroides).
     * Si usamos fcm_ejecutar, los centroides entrenados (2015-2016) se moverían 
     * al ver los datos de 2017, arruinando la generalización del modelo. */
    fcm_actualizar_membresias_con_centroides_fijos(g_vectores_prueba, n,
        g_n_clusters, g_m);

    g_n_prueba = n;
    return n;
}

// -----------------------------------------------------------
// entrenamiento_identificar_cluster_riesgo
// Heurística climática: De todos los clusters matemáticos que halló el 
// FCM, ¿cuál de ellos representa un huracán/ciclón?
// -----------------------------------------------------------
int entrenamiento_identificar_cluster_riesgo(void) {

    if (!g_entrenado) return -1;

    /* --- HEURÍSTICA DE IDENTIFICACIÓN ---
     * Científicamente, un ciclón tropical se alimenta de aguas cálidas (Alta SST)
     * y presenta una presión atmosférica central muy baja.
     * 
     * Definimos un puntaje rudimentario de riesgo por cluster:
     *     score(c) = Temperatura_Mar(SST) - Presion_Atmosferica
     * 
     * El cluster (centroide) que obtenga la mayor puntuación será marcado
     * de forma automática por el sistema como el "Cluster de Peligro Inminente".
     * 
     * NOTA: Esta resta directa funciona maravillosamente porque los datos
     * ya fueron previamente normalizados [0.0, 1.0] por el importador.
     */
  
    double centroide[NUM_VARIABLES];

    // Asumimos inicialmente que el cluster 0 es el peligroso
    fcm_obtener_centroide(0, centroide);
    int mejor_cluster = 0;
    double mejor_score = centroide[VAR_SST] - centroide[VAR_PRESION];

    // Evaluamos el resto de clusters
    for (int c = 1; c < g_n_clusters; c++) {
        fcm_obtener_centroide(c, centroide);
        double score = centroide[VAR_SST] - centroide[VAR_PRESION];
        
        // Si hallamos uno peor (más cálido y con menor presión), lo coronamos
        if (score > mejor_score) {
            mejor_score = score;
            mejor_cluster = c;
        }
    }

    g_cluster_riesgo = mejor_cluster;
    return g_cluster_riesgo;
}

// -----------------------------------------------------------
// entrenamiento_obtener_riesgo
// Devuelve el grado de alerta (0.0 a 1.0) para un día específico
// de los datos de prueba, basado en qué tanto se parece al
// cluster de "Peligro de Ciclón".
// -----------------------------------------------------------
double entrenamiento_obtener_riesgo(int index) {

    if (!g_entrenado) return -1.0;
    if (index < 0 || index >= g_n_prueba) return -1.0;

    /* Evaluación Perezosa (Lazy Evaluation): 
     * Si nadie le ha pedido al algoritmo que busque el cluster de riesgo
     * después del entrenamiento, se autoejecuta en el momento exacto que se necesita. */
    if (g_cluster_riesgo < 0) {
        entrenamiento_identificar_cluster_riesgo();
    }

    // Le consultamos al módulo matemático qué tanta membresía tiene 
    // este día 'index' exclusivamente con el cluster de peligro.
    return fcm_obtener_membresia(index, g_cluster_riesgo);
}

// -----------------------------------------------------------
// entrenamiento_obtener_riesgo_detallado
// Variante avanzada de la predicción. Además de devolver el nivel de riesgo,
// explica el "Por qué" (Explicabilidad / XAI).
// -----------------------------------------------------------
double entrenamiento_obtener_riesgo_detallado(int index, double breakdown[NUM_VARIABLES]) {
    double riesgo = entrenamiento_obtener_riesgo(index);
    
    // Limpieza inicial del arreglo de porcentajes (0%)
    for(int i = 0; i < NUM_VARIABLES; i++) breakdown[i] = 0.0;
    
    if (riesgo < 0) return riesgo; // Fallo en la lectura
    
    // Obtenemos las coordenadas perfectas del "Peligro"
    double centroide[NUM_VARIABLES];
    fcm_obtener_centroide(g_cluster_riesgo, centroide);
    
    double distancia_total = 0.0;
    double distancias_parciales[NUM_VARIABLES];
    
    /* Calculamos cuánto difiere el día actual de la tormenta perfecta,
     * variable por variable (Temperatura, Viento, etc.) */
    for (int k = 0; k < NUM_VARIABLES; k++) {
        double diff = g_vectores_prueba[index][k] - centroide[k];
        distancias_parciales[k] = diff * diff; // Distancia cuadrada parcial
        distancia_total += distancias_parciales[k];
    }
    
    /* Transformación a Porcentajes de Afinidad:
     * Si la distancia en Viento es casi 0 (es decir, el clima de hoy tiene 
     * vientos idénticos a los del huracán), su "afinidad" tiende a infinito. 
     * Usamos la inversa de la distancia cuadrada (con un pequeño épsilon 
     * para evitar divisiones por cero). */
    double afinidad_total = 0.0;
    double afinidades_parciales[NUM_VARIABLES];
    
    for(int k = 0; k < NUM_VARIABLES; k++) {
        afinidades_parciales[k] = 1.0 / (distancias_parciales[k] + 1e-10);
        afinidad_total += afinidades_parciales[k];
    }
    
    // Normalizamos las afinidades para que la suma total sea exactamente 100% (1.0).
    // Esto rellenará el arreglo 'breakdown' indicando, por ejemplo:
    // [SST: 40%, Viento: 30%, Humedad: 25%, ...]
    for(int k = 0; k < NUM_VARIABLES; k++) {
        breakdown[k] = afinidades_parciales[k] / afinidad_total;
    }
    
    return riesgo;
}