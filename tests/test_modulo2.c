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

static void probar_json_valido(void) {
    printf("\n=== importar_json: array de objetos valido ===\n");
    const char* json =
        "[\n"
        "  {\"anio\":2015,\"mes\":8,\"dia\":10,\"lat\":15.2,\"lon\":-60.1,"
        "\"sst\":27.8,\"presion\":1011,\"humedad\":68,\"viento\":25,"
        "\"cizalladura\":14,\"hubo_ciclon\":0},\n"
        "  {\"anio\":2015,\"mes\":8,\"dia\":28,\"lat\":17.4,\"lon\":-62.3,"
        "\"sst\":28.6,\"presion\":1004,\"humedad\":78,\"viento\":55,"
        "\"cizalladura\":7,\"hubo_ciclon\":1}\n"
        "]";

    dataset_inicializar();
    int importados = importar_json(json);
    printf("  objetos importados: %d (esperado 2)\n", importados);
    for (int i = 0; i < dataset_total(); i++) {
        imprimir_registro(i);
    }
}

static void probar_json_objeto_incompleto(void) {
    printf("\n=== importar_json: objeto con clave faltante se descarta ===\n");
    const char* json =
        "[{\"anio\":2015,\"mes\":8,\"dia\":10,\"lat\":15.2,\"lon\":-60.1,"
        "\"sst\":27.8,\"presion\":1011,\"humedad\":68,\"viento\":25,"
        "\"cizalladura\":14,\"hubo_ciclon\":0},"
        "{\"anio\":2016,\"mes\":9,\"dia\":5}]"; // falta la mayoria de las claves

    dataset_inicializar();
    int importados = importar_json(json);
    printf("  objetos importados: %d (esperado 1 de 2)\n", importados);
}

static void probar_hurdat2(void) {
    printf("\n=== importar_hurdat2: parseo y matching por fecha/zona ===\n");

    // Simulamos que el CSV/JSON de 2017 ya se importo antes: dos
    // registros con hubo_ciclon=0 (todavia no se sabe si hubo ciclon).
    dataset_inicializar();
    RegistroClimatico r1 = {2017, 8, 30, 16.8, -61.5, 28.7, 1003, 79, 60, 6, 0};
    RegistroClimatico r2 = {2017, 9, 6, 18.4, -63.1, 29.1, 995, 85, 120, 4, 0};
    dataset_insertar(r1);
    dataset_insertar(r2);

    const char* hurdat2 =
        "AL092017,             IRMA,     39,\n"
        // coincide en fecha y zona con r1 (16.8N/61.5W vs registro en 16.8/-61.5)
        "20170830, 0000,  , TD, 16.8N,  61.5W,  30, 1006, -999, -999, -999,\n"
        // coincide con r2 (18.4N/63.1W vs registro en 18.4/-63.1)
        "20170906, 1200,  , HU, 18.4N,  63.1W, 100,  950, -999, -999, -999,\n"
        // no coincide con ningun registro cargado (otra fecha/zona)
        "20170701, 0000,  , TD,  5.0N,  10.0W,  25, 1010, -999, -999, -999,\n";

    int procesadas = importar_hurdat2(hurdat2);
    printf("  lineas de track procesadas: %d (esperado 3)\n", procesadas);

    // IMPORTANTE (ver TODO en importar_hurdat2): el match se detecta
    // internamente pero todavia no se puede persistir porque Modulo 1
    // no expone forma de actualizar un registro ya insertado. Por eso
    // hubo_ciclon sigue en 0 aca — es el comportamiento esperado HOY,
    // no un bug de este modulo.
    printf("  r1.hubo_ciclon = %d (sigue en 0: bloqueado por falta de setter en Modulo 1)\n",
           dataset_get(0).hubo_ciclon);
    printf("  r2.hubo_ciclon = %d (sigue en 0: bloqueado por falta de setter en Modulo 1)\n",
           dataset_get(1).hubo_ciclon);
}

int main(void) {
    printf("=== Test Modulo 2 (importador) ===\n\n");
    probar_deteccion();
    probar_csv_valido();
    probar_csv_lineas_corruptas();
    probar_json_valido();
    probar_json_objeto_incompleto();
    probar_hurdat2();
    return 0;
}
