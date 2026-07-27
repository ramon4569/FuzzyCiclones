#include <string.h>
#include "../archivos.h/modulo1_dataset.h"

// Almacen estatico en memoria (mismo enfoque que modulo1_historial.c
// en ProyectoBisect: un array fijo + un contador de "ocupados")
static RegistroClimatico g_dataset[MAX_REGISTROS];
static int g_total = 0;

void dataset_inicializar(void) {
    // TODO: poner g_total en 0 (no hace falta borrar el array,
    // los valores viejos quedan "invisibles" mientras g_total no crezca)
}

int dataset_insertar(RegistroClimatico r) {
    // TODO:
    // 1. validar que g_total < MAX_REGISTROS (si no, retornar -1)
    // 2. copiar r a g_dataset[g_total]
    // 3. incrementar g_total
    // 4. retornar el indice insertado
    return -1;
}

RegistroClimatico dataset_get(int index) {
    // TODO: validar 0 <= index < g_total antes de acceder.
    // Si el indice es invalido, decidir un valor de retorno seguro
    // (ej. registro con todos los campos en 0).
    RegistroClimatico vacio = {0};
    return vacio;
}

int dataset_total(void) {
    return g_total;
}

void dataset_limpiar(void) {
    // TODO: igual que dataset_inicializar()
}

double dataset_promedio_variable(int indice_variable) {
    // TODO: recorrer g_dataset[0..g_total-1], usar registro_a_vector()
    // o acceder directo al campo segun indice_variable, y promediar.
    return 0.0;
}

void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]) {
    // TODO: recorrer todo el dataset y, para cada una de las
    // NUM_VARIABLES variables, guardar el minimo y el maximo
    // encontrados en min_out[] y max_out[].
}

int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
                              RegistroClimatico* destino, int max_destino) {
    // TODO: recorrer g_dataset, copiar a 'destino' los registros cuyo
    // r.anio este entre anio_inicio y anio_fin (inclusive), respetando
    // el limite max_destino. Retornar cuantos se copiaron.
    return 0;
}
