#include <stdio.h>
#include <string.h>
#include "../core/archivos.h/modulo2_importador.h"
#include "../core/archivos.h/registro.h"

// ==========================================================
// TEST LOCAL DEL MODULO 2 (importador)
//
// El Modulo 1 (dataset en memoria) todavia esta en TODO:
// dataset_insertar() siempre devuelve -1 ahi. Para poder probar
// el parseo del Modulo 2 sin depender de que otro integrante
// termine su parte, este archivo define una version minima
// ("mock") de las mismas funciones declaradas en
// modulo1_dataset.h. No se compila junto con el modulo1_dataset.c
// real (ver README de esta carpeta): es solo para pruebas
// aisladas de este modulo.
// ==========================================================
#define MOCK_MAX_REGISTROS 200
static RegistroClimatico g_mock_dataset[MOCK_MAX_REGISTROS];
static int g_mock_total = 0;

void dataset_inicializar(void) {
    g_mock_total = 0;
}

void dataset_limpiar(void) {
    g_mock_total = 0;
}

int dataset_insertar(RegistroClimatico r) {
    if (g_mock_total >= MOCK_MAX_REGISTROS) {
        return -1;
    }
    g_mock_dataset[g_mock_total] = r;
    return g_mock_total++;
}

RegistroClimatico dataset_get(int index) {
    RegistroClimatico vacio = {0};
    if (index < 0 || index >= g_mock_total) {
        return vacio;
    }
    return g_mock_dataset[index];
}

int dataset_total(void) {
    return g_mock_total;
}

double dataset_promedio_variable(int indice_variable) {
    (void)indice_variable;
    return 0.0;
}

void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]) {
    (void)min_out;
    (void)max_out;
}

int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
                              RegistroClimatico* destino, int max_destino) {
    (void)anio_inicio;
    (void)anio_fin;
    (void)destino;
    (void)max_destino;
    return 0;
}

// ==========================================================
// Helpers de impresion para inspeccionar resultados a ojo
// ==========================================================
static void imprimir_registro(int i) {
    RegistroClimatico r = dataset_get(i);
    printf("  [%d] %04d-%02d-%02d sst=%.1f presion=%.1f humedad=%.1f viento=%.1f cizalladura=%.1f hubo_ciclon=%d\n",
           i, r.anio, r.mes, r.dia, r.sst, r.presion, r.humedad, r.viento, r.cizalladura, r.hubo_ciclon);
}

int main(void) {
    printf("=== Test Modulo 2 (importador) ===\n");
    printf("(harness listo — todavia no hay funciones implementadas para probar)\n");
    return 0;
}
