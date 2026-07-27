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
// Responsable: (asignar integrante)
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
// Carga de datos (Modulo 1 + Modulo 2)
// -----------------------------------------------------------

// Recibe un string (CSV, JSON o HURDAT2) desde el navegador (por ejemplo,
// leido de un <input type="file">) y lo importa al dataset en memoria.
EMSCRIPTEN_KEEPALIVE
char* api_importar_datos(char* contenido) {
    static char buffer[512];

    // TODO: llamar a importar_archivo(contenido) (Modulo 2) y armar
    // un JSON de resultado, ej:
    // {"exito":1,"formato_detectado":"csv","importados":123}
    snprintf(buffer, sizeof(buffer),
        "{\"exito\":0,\"mensaje\":\"pendiente de implementar\",\"importados\":0}");
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

// -----------------------------------------------------------
// Entrenamiento y prediccion (Modulo 3 + Modulo 4)
// -----------------------------------------------------------

// Entrena el modelo con los registros del dataset dentro del rango de
// anios indicado. Retorna JSON con info del entrenamiento (iteraciones,
// centroides resultantes).
EMSCRIPTEN_KEEPALIVE
char* api_entrenar(int anio_inicio, int anio_fin, int n_clusters, double m,
                    int max_iter, double epsilon) {
    static char buffer[1024];

    // TODO: llamar a entrenamiento_entrenar(...) y armar el JSON con
    // el resultado, incluyendo los centroides via fcm_obtener_centroide()
    snprintf(buffer, sizeof(buffer),
        "{\"exito\":0,\"iteraciones\":0,\"mensaje\":\"pendiente de implementar\"}");
    return buffer;
}

// Corre la prediccion sobre los registros de prueba ya cargados con
// api_cargar_prueba(). Retorna JSON con el riesgo (mu al cluster de
// alto riesgo) de cada registro.
EMSCRIPTEN_KEEPALIVE
char* api_predecir(void) {
    static char buffer[2 * 1024 * 1024];

    // TODO:
    // 1. llamar a entrenamiento_predecir(g_buffer_prueba, g_total_prueba)
    // 2. recorrer cada registro y armar un array JSON:
    //    [{"anio":2017,"mes":9,"dia":6,"riesgo":0.87,"hubo_ciclon":1}, ...]
    snprintf(buffer, sizeof(buffer), "[]");
    return buffer;
}

// -----------------------------------------------------------
// Evaluacion (Modulo 5)
// -----------------------------------------------------------

// Calcula la matriz de confusion y las metricas contra los ciclones
// reales, usando el umbral indicado (ej. 0.5).
EMSCRIPTEN_KEEPALIVE
char* api_evaluar(double umbral) {
    return evaluador_generar_reporte_json(umbral);
}

// -----------------------------------------------------------
// Utilidades para el visualizador (Modulo 7)
// -----------------------------------------------------------

// Devuelve los c centroides entrenados en JSON, para pintarlos en el
// grafico/heatmap del frontend.
EMSCRIPTEN_KEEPALIVE
char* api_obtener_centroides(void) {
    static char buffer[2048];

    // TODO: recorrer fcm_obtener_num_clusters() centroides y armar
    // [{"cluster":0,"sst":..,"presion":..,"humedad":..,"viento":..,"cizalladura":..}, ...]
    snprintf(buffer, sizeof(buffer), "[]");
    return buffer;
}
