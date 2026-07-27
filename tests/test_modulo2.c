#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

static void probar_deteccion(void) {
    struct { const char* nombre; const char* contenido; FormatoArchivo esperado; } casos[] = {
        { "CSV",          "anio,mes,dia,lat,lon,sst,presion,humedad,viento,cizalladura,hubo_ciclon\n2015,8,10,15.2,-60.1,27.8,1011,68,25,14,0", FORMATO_CSV },
        { "JSON array",   "[{\"anio\":2015}]", FORMATO_JSON },
        { "JSON objeto",  "{\"anio\":2015}", FORMATO_JSON },
        { "HURDAT2",      "AL092017,             IRMA,     39,\n20170830, 0000,  , TD, 16.1N,  26.9W,  30, 1006,", FORMATO_HURDAT2 },
        { "Desconocido",  "esto no es nada reconocible", FORMATO_DESCONOCIDO },
        { "Vacio",        "", FORMATO_DESCONOCIDO },
    };
    int n = (int)(sizeof(casos) / sizeof(casos[0]));

    printf("=== importador_detectar_formato ===\n");
    for (int i = 0; i < n; i++) {
        FormatoArchivo obtenido = importador_detectar_formato(casos[i].contenido);
        const char* resultado = (obtenido == casos[i].esperado) ? "OK" : "FALLO";
        printf("  [%s] %-12s -> obtenido=%d esperado=%d\n", resultado, casos[i].nombre, obtenido, casos[i].esperado);
    }
}

// Lee un archivo completo a un buffer nuevo (el llamador debe hacer free()).
static char* leer_archivo(const char* ruta) {
    FILE* f = fopen(ruta, "rb");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc((size_t)tam + 1);
    if (buffer) {
        size_t leido = fread(buffer, 1, (size_t)tam, f);
        buffer[leido] = '\0';
    }
    fclose(f);
    return buffer;
}

static void probar_csv_valido(void) {
    printf("\n=== importar_csv: archivo de ejemplo real ===\n");
    char* contenido = leer_archivo("data/entrenamiento_2015_2016.csv");
    if (!contenido) {
        printf("  [SKIP] no se encontro data/entrenamiento_2015_2016.csv (correr desde la raiz del repo)\n");
        return;
    }

    dataset_inicializar();
    int importados = importar_csv(contenido);
    printf("  filas importadas: %d (esperado 6)\n", importados);
    for (int i = 0; i < dataset_total(); i++) {
        imprimir_registro(i);
    }

    free(contenido);
}

static void probar_csv_lineas_corruptas(void) {
    printf("\n=== importar_csv: filas invalidas se descartan sin crashear ===\n");
    const char* csv =
        "anio,mes,dia,lat,lon,sst,presion,humedad,viento,cizalladura,hubo_ciclon\n"
        "2015,8,10,15.2,-60.1,27.8,1011,68,25,14,0\n"   // valida
        "2015,8,28,17.4,-62.3\n"                          // faltan columnas
        "2015,x,15,14.8,-58.9,26.9,1013,60,20,18,0\n"     // "mes" no numerico
        "\n"                                              // linea vacia
        "2016,8,20,16.1,-61.0,28.9,1002,80,70,5,1";       // valida, sin \n final

    dataset_inicializar();
    int importados = importar_csv(csv);
    printf("  filas importadas: %d (esperado 2 de 4 lineas de datos)\n", importados);
    printf("  dataset_total(): %d\n", dataset_total());
}

int main(void) {
    printf("=== Test Modulo 2 (importador) ===\n\n");
    probar_deteccion();
    probar_csv_valido();
    probar_csv_lineas_corruptas();
    return 0;
}
