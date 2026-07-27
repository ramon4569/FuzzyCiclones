#ifndef MODULO2_IMPORTADOR_H
#define MODULO2_IMPORTADOR_H

#include "registro.h"

// ==========================================================
// MODULO 2 — IMPORTADOR MULTI-FORMATO
// Responsable: (asignar integrante)
// Equivalente a modulo3_importar.c en ProyectoBisect, pero
// ampliado para soportar varios formatos de entrada.
//
// Este modulo NO decide donde se guardan los datos: parsea el
// texto recibido y llama a dataset_insertar() (Modulo 1) por
// cada registro valido que encuentra.
// ==========================================================

typedef enum {
    FORMATO_DESCONOCIDO = 0,
    FORMATO_CSV         = 1,
    FORMATO_JSON         = 2,
    FORMATO_HURDAT2       = 3   // formato oficial NOAA de huracanes historicos
} FormatoArchivo;

// Analiza el contenido recibido (las primeras lineas) y adivina el formato
FormatoArchivo importador_detectar_formato(const char* contenido);

// Importa registros climaticos desde texto CSV.
// Columnas esperadas (con encabezado):
// anio,mes,dia,lat,lon,sst,presion,humedad,viento,cizalladura,hubo_ciclon
// Retorna la cantidad de filas importadas, -1 si el formato es invalido.
int importar_csv(const char* contenido);

// Importa registros climaticos desde un string JSON (array de objetos)
// con las mismas claves que las columnas del CSV.
// Retorna la cantidad de objetos importados, -1 si el JSON es invalido.
int importar_json(const char* contenido);

// Importa directamente el formato HURDAT2 de NOAA (best-track de huracanes).
// Este formato no trae variables climaticas de contexto (sst, humedad, etc),
// solo posicion/presion/viento de ciclones YA formados: se usa principalmente
// para poblar la etiqueta hubo_ciclon en el Modulo 5 (evaluador), cruzando
// por fecha con los registros climaticos ya cargados.
// Retorna la cantidad de lineas de ciclon procesadas.
int importar_hurdat2(const char* contenido);

// Punto de entrada unico para el resto del sistema (y para el API bridge):
// detecta el formato automaticamente y delega en la funcion correspondiente.
// Retorna la cantidad de registros importados, -1 si no se pudo determinar
// el formato o el contenido esta vacio/corrupto.
int importar_archivo(const char* contenido);

#endif // MODULO2_IMPORTADOR_H
