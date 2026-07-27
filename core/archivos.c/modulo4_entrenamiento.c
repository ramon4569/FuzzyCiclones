#include <string.h>
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"
#include "../archivos.h/modulo3_fcm_core.h"

static int    g_modelo_entrenado = 0;
static int    g_cluster_riesgo = -1;
static double g_riesgo_prediccion[MAX_REGISTROS];
static int    g_total_prediccion = 0;

int entrenamiento_entrenar(int anio_inicio, int anio_fin,
                            int n_clusters, double m,
                            int max_iter, double epsilon) {
    // TODO:
    // 1. usar dataset_filtrar_por_anio() (Modulo 1) para armar el
    //    subconjunto de entrenamiento (ej. 2015-2016)
    // 2. convertir cada RegistroClimatico a double[NUM_VARIABLES]
    //    con registro_a_vector()
    // 3. llamar a fcm_ejecutar() (Modulo 3) con esos vectores
    // 4. marcar g_modelo_entrenado = 1
    // 5. retornar la cantidad de iteraciones (o -1 si no hay datos)
    return -1;
}

int entrenamiento_predecir(const RegistroClimatico* datos_prueba, int n) {
    // TODO:
    // 1. si !g_modelo_entrenado, retornar -1
    // 2. convertir cada registro de datos_prueba a double[NUM_VARIABLES]
    // 3. llamar a fcm_actualizar_membresias_con_centroides_fijos()
    //    (Modulo 3) — esto NO mueve los centroides entrenados
    // 4. identificar el cluster de riesgo con
    //    entrenamiento_identificar_cluster_riesgo() si aun no se hizo
    // 5. guardar fcm_obtener_membresia(i, cluster_riesgo) en
    //    g_riesgo_prediccion[i] para cada punto
    // 6. guardar g_total_prediccion = n y retornarlo
    return -1;
}

int entrenamiento_identificar_cluster_riesgo(void) {
    // TODO: recorrer los centroides (fcm_obtener_centroide) y
    // elegir el que tenga mayor VAR_SST y menor VAR_PRESION
    // (una heuristica simple: comparar sst - presion_normalizada,
    // o usar un score ponderado que el equipo defina).
    return -1;
}

double entrenamiento_obtener_riesgo(int index) {
    // TODO: validar 0 <= index < g_total_prediccion y retornar
    // g_riesgo_prediccion[index]
    return 0.0;
}
