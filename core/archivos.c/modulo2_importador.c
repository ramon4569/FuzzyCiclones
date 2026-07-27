#include <string.h>
#include <stdlib.h>
#include "../archivos.h/modulo2_importador.h"
#include "../archivos.h/modulo1_dataset.h"

FormatoArchivo importador_detectar_formato(const char* contenido) {
    // TODO: mirar los primeros caracteres no-espacio de 'contenido':
    //   - si empieza con '[' o '{'                -> FORMATO_JSON
    //   - si la primera linea tiene comas y un encabezado tipo
    //     "anio,mes,dia,..."                        -> FORMATO_CSV
    //   - si el patron coincide con lineas HURDAT2
    //     (ej. "AL092017,IRMA,...")                 -> FORMATO_HURDAT2
    //   - si no calza con nada                        -> FORMATO_DESCONOCIDO
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
