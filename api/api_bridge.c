#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/archivos.h/registro.h"
#include "../core/archivos.h/modulo1_dataset.h"
#include "../core/archivos.h/modulo2_importador.h"
#include "../core/archivos.h/modulo3_fcm_core.h"
#include "../core/archivos.h/modulo4_entrenamiento.h"
#include "../core/archivos.h/modulo5_evaluador.h"

// ==========================================================
// MODULO 6 — API BRIDGE (WebAssembly)
// Responsable: Joseidy
//
// Puente entre la logica en C (Modulos 1-5) y el frontend en
// JavaScript (Modulo 7). Cada funcion EMSCRIPTEN_KEEPALIVE
// queda exportada y se puede invocar desde JS con M.ccall().
// Ninguna funcion de aca implementa logica propia: solo arma
// el JSON de entrada/salida y delega en los otros modulos.
// ==========================================================

// Buffer estatico auxiliar para armar arrays de registros de prueba
// antes de pasarlos al Modulo 4 (no se toca el dataset global del
// Modulo 1 para no mezclar entrenamiento con prueba).
static RegistroClimatico g_buffer_prueba[MAX_REGISTROS];
static int g_total_prueba = 0;

// -----------------------------------------------------------
// Helper interno: convierte el enum FormatoArchivo (Modulo 2)
// a un string legible para el JSON. No es logica de negocio,
// solo texto de presentacion, por eso vive aca y no en Modulo 2.
// -----------------------------------------------------------
static const char* formato_a_texto(FormatoArchivo formato) {
    switch (formato) {
    case FORMATO_CSV:     return "csv";
    case FORMATO_JSON:    return "json";
    case FORMATO_HURDAT2: return "hurdat2";
    default:              return "desconocido";
    }
}

// -----------------------------------------------------------
// Carga de datos (Modulo 1 + Modulo 2)
// -----------------------------------------------------------

// Recibe un string (CSV, JSON o HURDAT2) desde el navegador (por ejemplo,
// leido de un <input type="file">) y lo importa al dataset en memoria.
EMSCRIPTEN_KEEPALIVE
char* api_importar_datos(char* contenido) {
    static char buffer[512];

    FormatoArchivo formato = importador_detectar_formato(contenido);
    int importados = importar_archivo(contenido);

    if (importados < 0) {
        snprintf(buffer, sizeof(buffer),
            "{\"exito\":0,\"formato_detectado\":\"%s\",\"importados\":0}",
            formato_a_texto(formato));
    }
    else {
        snprintf(buffer, sizeof(buffer),
            "{\"exito\":1,\"formato_detectado\":\"%s\",\"importados\":%d}",
            formato_a_texto(formato), importados);
    }

    return buffer;
}

EMSCRIPTEN_KEEPALIVE
int api_dataset_total(void) {
    return dataset_total();
}


EMSCRIPTEN_KEEPALIVE
void api_dataset_limpiar(void) {
    dataset_limpiar();
}

// Recibe el contenido crudo del archivo de PRUEBA (ej. el CSV de 2017)
// desde el navegador, lo importa al dataset del Modulo 1 (via Modulo 2,
// igual que cualquier otro archivo) y de una vez filtra ese rango de
// anios hacia g_buffer_prueba, dejando todo listo para api_predecir().
// Retorna JSON con cuantos registros se importaron y cuantos quedaron
// disponibles en el buffer de prueba.
EMSCRIPTEN_KEEPALIVE
char* api_cargar_prueba(char* contenido, int anio_inicio, int anio_fin) {
    static char buffer[512];

    int importados = importar_archivo(contenido);
    if (importados < 0) {
        snprintf(buffer, sizeof(buffer),
            "{\"exito\":0,\"importados\":0,\"total_prueba\":0,"
            "\"mensaje\":\"no se pudo importar el archivo de prueba\"}");
        return buffer;
    }

    g_total_prueba = dataset_filtrar_por_anio(anio_inicio, anio_fin,
        g_buffer_prueba, MAX_REGISTROS);

    snprintf(buffer, sizeof(buffer),
        "{\"exito\":1,\"importados\":%d,\"total_prueba\":%d}",
        importados, g_total_prueba);

    return buffer;
}

// -----------------------------------------------------------
// Entrenamiento y prediccion (Modulo 3 + Modulo 4)
// -----------------------------------------------------------

// Entrena el modelo con los registros del dataset dentro del rango de
// anios indicado. Retorna JSON con info del entrenamiento (iteraciones,
// centroides resultantes).
EMSCRIPTEN_KEEPALIVE
char* api_entrenar(int anio_inicio, int anio_fin, int n_clusters, double m,
    int max_iter, double epsilon) {
    static char buffer[16384];

    int iteraciones = entrenamiento_entrenar(anio_inicio, anio_fin, n_clusters,
        m, max_iter, epsilon);

    if (iteraciones < 0) {
        snprintf(buffer, sizeof(buffer),
            "{\"exito\":0,\"iteraciones\":0,"
            "\"mensaje\":\"error al entrenar: revisar rango de anios o datos cargados\"}");
        return buffer;
    }

    int total_clusters = fcm_obtener_num_clusters();
    double centroide[NUM_VARIABLES];
    char centroides_json[12288];
    char item[256];

    centroides_json[0] = '\0';
    strcat(centroides_json, "[");

    for (int i = 0; i < total_clusters; i++) {
        fcm_obtener_centroide(i, centroide);
        snprintf(item, sizeof(item),
            "%s{\"cluster\":%d,\"sst\":%.4f,\"presion\":%.4f,"
            "\"humedad\":%.4f,\"viento\":%.4f,\"cizalladura\":%.4f}",
            (i > 0) ? "," : "",
            i,
            centroide[VAR_SST],
            centroide[VAR_PRESION],
            centroide[VAR_HUMEDAD],
            centroide[VAR_VIENTO],
            centroide[VAR_CIZALLADURA]);
        strncat(centroides_json, item, sizeof(centroides_json) - strlen(centroides_json) - 1);
    }
    strncat(centroides_json, "]", sizeof(centroides_json) - strlen(centroides_json) - 1);

    snprintf(buffer, sizeof(buffer),
        "{\"exito\":1,\"iteraciones\":%d,\"n_clusters\":%d,\"centroides\":%s}",
        iteraciones, total_clusters, centroides_json);

    return buffer;
}

// Corre la prediccion sobre los registros de prueba ya cargados con
// api_cargar_prueba(). Retorna JSON con el riesgo (mu al cluster de
// alto riesgo) de cada registro.
EMSCRIPTEN_KEEPALIVE
char* api_predecir(void) {
    static char buffer[2 * 1024 * 1024];

    if (g_total_prueba <= 0) {
        snprintf(buffer, sizeof(buffer), "[]");
        return buffer;
    }

    int procesados = entrenamiento_predecir(g_buffer_prueba, g_total_prueba);
    if (procesados < 0) {
        snprintf(buffer, sizeof(buffer), "[]");
        return buffer;
    }

    buffer[0] = '\0';
    strcat(buffer, "[");

    char item[512];
    for (int i = 0; i < g_total_prueba; i++) {
        double breakdown[NUM_VARIABLES];
        double riesgo = entrenamiento_obtener_riesgo_detallado(i, breakdown);
        snprintf(item, sizeof(item),
            "%s{\"anio\":%d,\"mes\":%d,\"dia\":%d,\"lat\":%.2f,\"lon\":%.2f,\"riesgo\":%.4f,\"hubo_ciclon\":%d,"
            "\"sst\":%.2f,\"presion\":%.2f,\"humedad\":%.2f,\"viento\":%.2f,"
            "\"inf_sst\":%.4f,\"inf_presion\":%.4f,\"inf_humedad\":%.4f,\"inf_viento\":%.4f}",
            (i > 0) ? "," : "",
            g_buffer_prueba[i].anio,
            g_buffer_prueba[i].mes,
            g_buffer_prueba[i].dia,
            g_buffer_prueba[i].latitud,
            g_buffer_prueba[i].longitud,
            riesgo,
            g_buffer_prueba[i].hubo_ciclon,
            g_buffer_prueba[i].sst,
            g_buffer_prueba[i].presion,
            g_buffer_prueba[i].humedad,
            g_buffer_prueba[i].viento,
            breakdown[VAR_SST],
            breakdown[VAR_PRESION],
            breakdown[VAR_HUMEDAD],
            breakdown[VAR_VIENTO]);
        strncat(buffer, item, sizeof(buffer) - strlen(buffer) - 1);
    }
    strncat(buffer, "]", sizeof(buffer) - strlen(buffer) - 1);

    return buffer;
}

// -----------------------------------------------------------
// Evaluacion (Modulo 5)
// -----------------------------------------------------------

// Calcula la matriz de confusion y las metricas contra los ciclones
// reales, usando el umbral indicado (ej. 0.5).
EMSCRIPTEN_KEEPALIVE
char* api_evaluar(double umbral) {
    return evaluador_generar_reporte_json(umbral, g_buffer_prueba, g_total_prueba);
}

// -----------------------------------------------------------
// Utilidades para el visualizador (Modulo 7)
// -----------------------------------------------------------

// Devuelve los c centroides entrenados en JSON, para pintarlos en el
// grafico/heatmap del frontend.
EMSCRIPTEN_KEEPALIVE
char* api_obtener_centroides(void) {
    static char buffer[2048];

    int total_clusters = fcm_obtener_num_clusters();
    double centroide[NUM_VARIABLES];
    char item[256];

    buffer[0] = '\0';
    strcat(buffer, "[");

    for (int i = 0; i < total_clusters; i++) {
        fcm_obtener_centroide(i, centroide);
        snprintf(item, sizeof(item),
            "%s{\"cluster\":%d,\"sst\":%.4f,\"presion\":%.4f,"
            "\"humedad\":%.4f,\"viento\":%.4f,\"cizalladura\":%.4f}",
            (i > 0) ? "," : "",
            i,
            centroide[VAR_SST],
            centroide[VAR_PRESION],
            centroide[VAR_HUMEDAD],
            centroide[VAR_VIENTO],
            centroide[VAR_CIZALLADURA]);
        strncat(buffer, item, sizeof(buffer) - strlen(buffer) - 1);
    }
    strncat(buffer, "]", sizeof(buffer) - strlen(buffer) - 1);

    return buffer;
}