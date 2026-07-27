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

int importar_csv(const char* contenido) {
    // TODO:
    // 1. saltar la linea de encabezado
    // 2. por cada linea: separar por comas (strtok o similar)
    // 3. armar un RegistroClimatico y llamar a dataset_insertar()
    // 4. contar cuantas filas se importaron con exito y retornarlo
    // Cuidado con lineas vacias al final del archivo.
    return -1;
}

int importar_json(const char* contenido) {
    // TODO: parsear un array de objetos JSON con las mismas claves
    // que las columnas del CSV. Se puede reutilizar/adaptar el
    // parser simple que ya existe en modulo3_importar.c de
    // ProyectoBisect como punto de partida (busqueda de claves con
    // strstr, sin libreria externa de JSON).
    return -1;
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
