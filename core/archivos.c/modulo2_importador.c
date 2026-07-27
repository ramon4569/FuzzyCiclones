#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../archivos.h/modulo2_importador.h"
#include "../archivos.h/modulo1_dataset.h"

// Un encabezado HURDAT2 arranca con el codigo de cuenca del ciclon:
// dos letras (ej. "AL" de Atlantico) seguidas de 6 digitos
// (2 de numero de ciclon + 4 de anio), ej. "AL092017".
static int es_encabezado_hurdat2(const char* p) {
    if (!(isalpha((unsigned char)p[0]) && isalpha((unsigned char)p[1]))) {
        return 0;
    }
    for (int i = 2; i < 8; i++) {
        if (!isdigit((unsigned char)p[i])) {
            return 0;
        }
    }
    return 1;
}

FormatoArchivo importador_detectar_formato(const char* contenido) {
    if (!contenido) {
        return FORMATO_DESCONOCIDO;
    }

    // Saltar espacios/saltos de linea iniciales para no confundirnos
    // con archivos que empiezan con espacios en blanco.
    const char* p = contenido;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }

    if (*p == '[' || *p == '{') {
        return FORMATO_JSON;
    }

    if (es_encabezado_hurdat2(p)) {
        return FORMATO_HURDAT2;
    }

    // CSV: la primera linea debe ser el encabezado esperado, empezando
    // por la columna "anio" (ver contrato en el .h).
    if (strncmp(p, "anio", 4) == 0) {
        return FORMATO_CSV;
    }

    return FORMATO_DESCONOCIDO;
}

// Cantidad de columnas esperadas en cada linea de datos del CSV:
// anio,mes,dia,lat,lon,sst,presion,humedad,viento,cizalladura,hubo_ciclon
#define CSV_NUM_COLUMNAS 11

// Interpreta una sola linea de datos (sin el encabezado) y llena 'out'.
// 'linea' se modifica in-place (strtok corta la cadena). Devuelve 1 si
// las 11 columnas estaban presentes y eran numericas, 0 si la linea
// esta corrupta (se debe descartar sin frenar el resto del archivo).
static int parsear_linea_csv(char* linea, RegistroClimatico* out) {
    char* campos[CSV_NUM_COLUMNAS];
    int n = 0;

    char* tok = strtok(linea, ",");
    while (tok != NULL && n < CSV_NUM_COLUMNAS) {
        campos[n++] = tok;
        tok = strtok(NULL, ",");
    }
    if (n < CSV_NUM_COLUMNAS) {
        return 0;
    }

    char* fin;
    out->anio = (int)strtol(campos[0], &fin, 10);
    if (*fin != '\0') return 0;
    out->mes = (int)strtol(campos[1], &fin, 10);
    if (*fin != '\0') return 0;
    out->dia = (int)strtol(campos[2], &fin, 10);
    if (*fin != '\0') return 0;
    out->latitud = strtod(campos[3], &fin);
    if (*fin != '\0') return 0;
    out->longitud = strtod(campos[4], &fin);
    if (*fin != '\0') return 0;
    out->sst = strtod(campos[5], &fin);
    if (*fin != '\0') return 0;
    out->presion = strtod(campos[6], &fin);
    if (*fin != '\0') return 0;
    out->humedad = strtod(campos[7], &fin);
    if (*fin != '\0') return 0;
    out->viento = strtod(campos[8], &fin);
    if (*fin != '\0') return 0;
    out->cizalladura = strtod(campos[9], &fin);
    if (*fin != '\0') return 0;
    out->hubo_ciclon = (int)strtol(campos[10], &fin, 10);
    if (*fin != '\0') return 0;

    return 1;
}

int importar_csv(const char* contenido) {
    if (!contenido) {
        return -1;
    }

    int importados = 0;
    const char* cursor = contenido;
    int es_encabezado = 1;

    // Recorremos linea por linea manualmente (en vez de strtok sobre
    // todo el archivo) porque cada linea se vuelve a tokenizar por
    // comas mas abajo: strtok guarda un unico puntero interno global,
    // asi que anidar dos strtok sobre buffers distintos rompe el de
    // "afuera" a mitad de camino.
    while (*cursor != '\0') {
        const char* fin_linea = strchr(cursor, '\n');
        size_t largo = fin_linea ? (size_t)(fin_linea - cursor) : strlen(cursor);

        // Tolerar saltos de linea estilo Windows (\r\n)
        size_t largo_real = largo;
        if (largo_real > 0 && cursor[largo_real - 1] == '\r') {
            largo_real--;
        }

        if (es_encabezado) {
            es_encabezado = 0;
        } else if (largo_real > 0) {
            char* linea = (char*)malloc(largo_real + 1);
            if (linea) {
                memcpy(linea, cursor, largo_real);
                linea[largo_real] = '\0';

                RegistroClimatico r;
                if (parsear_linea_csv(linea, &r) && dataset_insertar(r) >= 0) {
                    importados++;
                }
                free(linea);
            }
        }
        // linea vacia (largo_real == 0, ej. ultima linea del archivo): se ignora

        cursor += largo;
        if (*cursor == '\n') {
            cursor++;
        }
    }

    return importados;
}

// Busca "clave": <numero> dentro de un objeto JSON (ya extraido, sin
// llaves anidadas) y devuelve el numero en 'valor'. No es un parser
// JSON general: alcanza para objetos planos con pares clave-numero,
// que es todo lo que necesita RegistroClimatico. Devuelve 0 si la
// clave no aparece o no la sigue un numero valido.
static int extraer_numero_json(const char* objeto, const char* clave, double* valor) {
    char patron[32];
    snprintf(patron, sizeof(patron), "\"%s\"", clave);

    const char* pos = strstr(objeto, patron);
    if (!pos) {
        return 0;
    }
    pos += strlen(patron);

    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    if (*pos != ':') {
        return 0;
    }
    pos++;
    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }

    char* fin;
    double v = strtod(pos, &fin);
    if (fin == pos) {
        return 0; // no habia un numero despues de los ':'
    }

    *valor = v;
    return 1;
}

// Llena 'out' con los valores de un objeto JSON ya extraido (con sus
// llaves { }). Exige que las 11 claves esten presentes, igual de
// estricto que parsear_linea_csv() con las columnas del CSV.
static int parsear_objeto_json(const char* objeto, RegistroClimatico* out) {
    double v;

    if (!extraer_numero_json(objeto, "anio", &v)) return 0;
    out->anio = (int)v;
    if (!extraer_numero_json(objeto, "mes", &v)) return 0;
    out->mes = (int)v;
    if (!extraer_numero_json(objeto, "dia", &v)) return 0;
    out->dia = (int)v;
    if (!extraer_numero_json(objeto, "lat", &v)) return 0;
    out->latitud = v;
    if (!extraer_numero_json(objeto, "lon", &v)) return 0;
    out->longitud = v;
    if (!extraer_numero_json(objeto, "sst", &v)) return 0;
    out->sst = v;
    if (!extraer_numero_json(objeto, "presion", &v)) return 0;
    out->presion = v;
    if (!extraer_numero_json(objeto, "humedad", &v)) return 0;
    out->humedad = v;
    if (!extraer_numero_json(objeto, "viento", &v)) return 0;
    out->viento = v;
    if (!extraer_numero_json(objeto, "cizalladura", &v)) return 0;
    out->cizalladura = v;
    if (!extraer_numero_json(objeto, "hubo_ciclon", &v)) return 0;
    out->hubo_ciclon = (int)v;

    return 1;
}

int importar_json(const char* contenido) {
    if (!contenido) {
        return -1;
    }

    int importados = 0;
    const char* cursor = contenido;

    // Los objetos de este formato son planos (sin objetos/arrays
    // anidados dentro de cada registro), asi que el primer '}' que
    // aparece despues de un '{' siempre es su cierre.
    while ((cursor = strchr(cursor, '{')) != NULL) {
        const char* fin = strchr(cursor, '}');
        if (!fin) {
            break;
        }

        size_t largo = (size_t)(fin - cursor) + 1;
        char* objeto = (char*)malloc(largo + 1);
        if (objeto) {
            memcpy(objeto, cursor, largo);
            objeto[largo] = '\0';

            RegistroClimatico r;
            if (parsear_objeto_json(objeto, &r) && dataset_insertar(r) >= 0) {
                importados++;
            }
            free(objeto);
        }

        cursor = fin + 1;
    }

    return importados;
}

int importar_hurdat2(const char* contenido) {
    // TODO: parsear el formato oficial de NOAA HURDAT2. Estructura:
    //   - lineas de encabezado por ciclon: AL092017, IRMA, <n_lineas>,
    //   - seguidas de <n_lineas> filas con fecha, hora, lat, lon,
    //     viento maximo, presion minima, etc.
    // Este modulo no necesariamente crea RegistroClimatico nuevos:
    // su tarea principal es marcar hubo_ciclon=1 en los registros
    // del dataset (Modulo 1) que coincidan en fecha/zona con un
    // ciclon real, para que el Modulo 5 pueda validar contra ellos.
    return -1;
}

int importar_archivo(const char* contenido) {
    if (!contenido || strlen(contenido) == 0) {
        return -1;
    }
    FormatoArchivo formato = importador_detectar_formato(contenido);
    switch (formato) {
        case FORMATO_CSV:      return importar_csv(contenido);
        case FORMATO_JSON:      return importar_json(contenido);
        case FORMATO_HURDAT2:    return importar_hurdat2(contenido);
        default:                  return -1;
    }
}
