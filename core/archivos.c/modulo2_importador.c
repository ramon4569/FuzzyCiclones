#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../archivos.h/modulo2_importador.h"
#include "../archivos.h/modulo1_dataset.h"

/**
 * @brief Valida la firma del bloque maestro (Header) de la base HURDAT2.
 * 
 * Patrón sintáctico esperado: Código de cuenca oceánica de 2 caracteres
 * alfabéticos, seguido por un identificador escalar de 6 dígitos numéricos
 * (ej. "AL092017" para el Huracán Irma en el Atlántico Norte).
 */
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

/**
 * @brief Rutina heurística para la inferencia de formato de serialización.
 */
FormatoArchivo importador_detectar_formato(const char* contenido) {
    if (!contenido) {
        return FORMATO_DESCONOCIDO;
    }

    const char* p = contenido;
    
    // Depuración de cabeceras binarias: Bypass del Byte Order Mark (BOM) en UTF-8
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    // Filtrado de ruido alfanumérico inicial (espacios, retorno de carro, avance de línea)
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

// Cardinalidad esperada de atributos independientes en el esquema relacional CSV
#define CSV_NUM_COLUMNAS 11

/**
 * @brief Tokenizador y parseador atómico para un vector CSV.
 * 
 * @param linea Puntero mutable al buffer local que contiene la fila (modificado por strtok).
 * @param out Puntero destino a la estructura de volcado relacional.
 * @return int Código booleano: 1 (parseo satisfactorio y completo), 0 (vector corrupto/incompleto).
 */
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
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->mes = (int)strtol(campos[1], &fin, 10);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->dia = (int)strtol(campos[2], &fin, 10);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->latitud = strtod(campos[3], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->longitud = strtod(campos[4], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->sst = strtod(campos[5], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->presion = strtod(campos[6], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->humedad = strtod(campos[7], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->viento = strtod(campos[8], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->cizalladura = strtod(campos[9], &fin);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
    if (*fin != '\0') return 0;
    out->hubo_ciclon = (int)strtol(campos[10], &fin, 10);
    while(*fin == ' ' || *fin == '\t' || *fin == '\r') fin++;
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

    // Bucle de recorrido en memoria para fragmentar el archivo por saltos de línea (\n).
    // Implementación iterativa ad-hoc para eludir colisiones de estado interno en strtok,
    // dado que la capa inferior (parsear_linea_csv) depende también de strtok.
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

/**
 * @brief Extractor léxico de variables escalares en formato JSON.
 * 
 * Implementa una heurística de búsqueda plana de subcadenas ("clave": valor).
 * Funcionalidad acotada estrictamente a estructuras sin anidamiento.
 * 
 * @param objeto Cadena que representa el documento u objeto JSON aislado.
 * @param clave Identificador de la propiedad a extraer.
 * @param valor Puntero donde se depositará el número extraído por referencia.
 * @return int Código de éxito (1) o fallo de parseo (0).
 */
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

/**
 * @brief Mapea un bloque JSON de un solo nivel hacia el registro climático de memoria.
 * 
 * Realiza una validación exhaustiva requiriendo la presencia obligatoria de
 * las 11 covariables (Strict Mapping), análogo al validador CSV.
 */
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

    // Estrategia de segmentación de objetos JSON asumiendo topología plana (flat objects).
    // Localiza pares asimétricos de llaves { } para aislar las instancias sin
    // incurrir en la sobrecarga computacional de un parser AST completo.
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

/**
 * @brief Función de saneamiento de cadenas (Trim in-place).
 * Elimina espacios en blanco y tabulaciones periféricas (Left & Right Trim).
 */
static void recortar(char* s) {
    char* inicio = s;
    while (*inicio == ' ' || *inicio == '\t') {
        inicio++;
    }
    if (inicio != s) {
        memmove(s, inicio, strlen(inicio) + 1);
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }
}

/**
 * @brief Decodificador de coordenadas geoespaciales formato HURDAT2.
 * 
 * Transforma una cadena híbrida (ej. "16.1N") a representación de punto flotante
 * con signo (Hemisferio Norte/Este = Positivo, Sur/Oeste = Negativo).
 */
static int parsear_coordenada_hurdat2(const char* campo, char signo_positivo, double* valor) {
    size_t len = strlen(campo);
    if (len < 2) {
        return 0;
    }
    char letra = (char)toupper((unsigned char)campo[len - 1]);
    if (!isalpha((unsigned char)letra)) {
        return 0;
    }

    char numero[16];
    if (len - 1 >= sizeof(numero)) {
        return 0;
    }
    memcpy(numero, campo, len - 1);
    numero[len - 1] = '\0';

    char* fin;
    double v = strtod(numero, &fin);
    if (fin == numero) {
        return 0;
    }

    *valor = (letra == signo_positivo) ? v : -v;
    return 1;
}

/**
 * @brief Extractor del vector de estado cinemático (Track line) de un ciclón NOAA.
 * 
 * Aísla los componentes espaciotemporales (fecha y geolocalización) obviando
 * las métricas extendidas de radios de viento (quadrants) irrelevantes al modelo.
 */
static int parsear_linea_track_hurdat2(char* linea, int* anio, int* mes, int* dia,
                                        double* lat, double* lon) {
    char* campos[8];
    int n = 0;
    char* tok = strtok(linea, ",");
    while (tok != NULL && n < 8) {
        campos[n++] = tok;
        tok = strtok(NULL, ",");
    }
    if (n < 8) {
        return 0;
    }
    for (int i = 0; i < n; i++) {
        recortar(campos[i]);
    }

    if (strlen(campos[0]) != 8) {
        return 0;
    }
    char buf[5];
    char* fin;

    memcpy(buf, campos[0], 4);
    buf[4] = '\0';
    *anio = (int)strtol(buf, &fin, 10);
    if (*fin != '\0') return 0;

    memcpy(buf, campos[0] + 4, 2);
    buf[2] = '\0';
    *mes = (int)strtol(buf, &fin, 10);
    if (*fin != '\0') return 0;

    memcpy(buf, campos[0] + 6, 2);
    buf[2] = '\0';
    *dia = (int)strtol(buf, &fin, 10);
    if (*fin != '\0') return 0;

    if (!parsear_coordenada_hurdat2(campos[4], 'N', lat)) return 0;
    if (!parsear_coordenada_hurdat2(campos[5], 'E', lon)) return 0;

    return 1;
}

// Tolerancia espacial (radio de vecindad en grados) para el algoritmo de cruce
// (Spatial Join) entre el centroide del ciclón (HURDAT2) y la coordenada
// observacional del dataset local. Actúa como función de umbral (Threshold).
#define HURDAT2_RADIO_ZONA_GRADOS 2.0

/**
 * @brief Motor de búsqueda para el apareamiento (Matching) geo-temporal.
 * 
 * Ejecuta un barrido lineal O(N) de solo lectura sobre el Módulo 1 (dataset).
 * Devuelve el primer índice (Early Return) donde coincida la fecha y el delta
 * de distancia caiga dentro del hipercubo definido por HURDAT2_RADIO_ZONA_GRADOS.
 */
static int buscar_registro_por_fecha_zona(int anio, int mes, int dia, double lat, double lon) {
    int total = dataset_total();
    for (int i = 0; i < total; i++) {
        RegistroClimatico r = dataset_get(i);
        if (r.anio != anio || r.mes != mes || r.dia != dia) {
            continue;
        }
        double dlat = r.latitud - lat;
        double dlon = r.longitud - lon;
        if (dlat < 0) dlat = -dlat;
        if (dlon < 0) dlon = -dlon;
        if (dlat <= HURDAT2_RADIO_ZONA_GRADOS && dlon <= HURDAT2_RADIO_ZONA_GRADOS) {
            return i;
        }
    }
    return -1;
}

int importar_hurdat2(const char* contenido) {
    if (!contenido) {
        return -1;
    }

    int procesadas = 0;
    const char* cursor = contenido;

    while (*cursor != '\0') {
        const char* fin_linea = strchr(cursor, '\n');
        size_t largo = fin_linea ? (size_t)(fin_linea - cursor) : strlen(cursor);
        size_t largo_real = largo;
        if (largo_real > 0 && cursor[largo_real - 1] == '\r') {
            largo_real--;
        }

        if (largo_real > 0) {
            char* linea = (char*)malloc(largo_real + 1);
            if (linea) {
                memcpy(linea, cursor, largo_real);
                linea[largo_real] = '\0';

                // Filtrado por capa de transporte: Ignora registros de Metadatos
                // (encabezados de ciclón) procediendo exclusivamente con los 
                // puntos discretos de la trayectoria (Track points) identificados por fecha.
                if (!es_encabezado_hurdat2(linea)) {
                    int anio, mes, dia;
                    double lat, lon;
                    if (parsear_linea_track_hurdat2(linea, &anio, &mes, &dia, &lat, &lon)) {
                        procesadas++;

                        int indice = buscar_registro_por_fecha_zona(anio, mes, dia, lat, lon);
                        if (indice >= 0) {
                            // Desbloqueado: se actualiza el dataset
                            dataset_marcar_ciclon(indice, 1);
                        }
                    }
                }

                free(linea);
            }
        }

        cursor += largo;
        if (*cursor == '\n') {
            cursor++;
        }
    }

    return procesadas;
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
