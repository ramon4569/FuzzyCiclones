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

/**
 * @brief Inspecciona heurísticamente el flujo de texto para inferir su esquema.
 * @param contenido Cadena de caracteres cruda (raw string) del archivo de entrada.
 * @return FormatoArchivo El identificador del tipo detectado (CSV, JSON, HURDAT2 o Desconocido).
 */
FormatoArchivo importador_detectar_formato(const char* contenido);

/**
 * @brief Rutina de deserialización para esquemas tabulares separados por comas (CSV).
 * 
 * Columnas requeridas estrictamente (con encabezado): 
 * anio, mes, dia, lat, lon, sst, presion, humedad, viento, cizalladura, hubo_ciclon
 * 
 * @param contenido Cadena de texto que contiene el archivo CSV completo.
 * @return int Número de tuplas procesadas e insertadas en memoria, o -1 ante errores.
 */
int importar_csv(const char* contenido);

/**
 * @brief Analizador sintáctico para colecciones planas de notación de objetos (JSON).
 * 
 * Extrae iterativamente las entidades del arreglo JSON. Las claves requeridas
 * coinciden uno-a-uno con los atributos del modelo relacional definido en CSV.
 * 
 * @param contenido Cadena de texto codificada en JSON.
 * @return int Conteo de los objetos instanciados, o -1 en caso de formato malformado.
 */
int importar_json(const char* contenido);

/**
 * @brief Ingesta de datos meteorológicos históricos del corpus oficial HURDAT2 de NOAA.
 * 
 * Operación asimétrica: El dataset HURDAT2 no provee variables climáticas basales, 
 * sino el best-track de huracanes maduros. Se emplea primariamente para inyectar 
 * la variable objetivo ('hubo_ciclon' = 1) en el Módulo de Evaluación (Módulo 5), 
 * aplicando un algoritmo de cruce espaciotemporal (Join) con los datos cargados.
 * 
 * @param contenido Texto crudo estructurado según estándar NOAA.
 * @return int Volumen de nodos del trayecto procesados.
 */
int importar_hurdat2(const char* contenido);

/**
 * @brief Controlador maestro para el flujo de ingesta de datos.
 * 
 * Interfaz de entrada (Facade) para la API bridge. Realiza el descubrimiento
 * automático de esquema (Auto-Detect) y actúa como multiplexor, delegando el
 * parsing a la rutina especializada correspondiente.
 * 
 * @param contenido Cadena a importar desde el sistema de archivos del usuario.
 * @return int Sumatoria total de los registros subidos a memoria (-1 si falla la capa proxy).
 */
int importar_archivo(const char* contenido);

#endif // MODULO2_IMPORTADOR_H
