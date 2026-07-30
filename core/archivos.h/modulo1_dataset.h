#ifndef MODULO1_DATASET_H
#define MODULO1_DATASET_H

#include "registro.h"

// ==========================================================
// MODULO 1 — DATASET Y ESTRUCTURAS DE DATOS
// Responsable: (asignar integrante)
// Equivalente a modulo1_historial.c en ProyectoBisect.
//
// Este modulo es la "memoria" del programa: guarda en un array
// estatico todos los RegistroClimatico cargados (entrenamiento
// o prueba) y ofrece operaciones basicas sobre ese array.
// No sabe nada de FCM ni de archivos, solo administra datos.
// ==========================================================

/**
 * @brief Inicializa el conjunto de datos estableciendo su tamaño lógico en cero.
 *
 * Prepara el arreglo interno para aceptar nuevas inserciones reiniciando el
 * contador principal. Esta operación tiene un orden de complejidad O(1).
 */
void dataset_inicializar(void);

/**
 * @brief Inserta un nuevo registro climático al final del conjunto de datos en memoria.
 *
 * @param r Estructura del registro climático que se desea insertar por valor.
 * @return int Índice de inserción dentro del arreglo estático, o -1 en caso de 
 *         overflow (cuando se excede MAX_REGISTROS).
 */
int dataset_insertar(RegistroClimatico r);

/**
 * @brief Obtiene una copia por valor del registro ubicado en el índice solicitado.
 *
 * @param index Índice posicional del registro dentro del dataset interno.
 * @return RegistroClimatico Copia íntegra de los datos. En caso de solicitar un
 *         índice fuera de los límites (out of bounds), retorna una estructura a ceros.
 */
RegistroClimatico dataset_get(int index);

/**
 * @brief Informa la cantidad total de registros meteorológicos activos.
 *
 * @return int Cardinalidad del dataset en tiempo de ejecución (hasta MAX_REGISTROS).
 */
int dataset_total(void);

/**
 * @brief Destruye virtualmente los registros actuales devolviendo el contador a cero.
 *
 * A diferencia de una liberación de memoria dinámica, aquí solo se restablece 
 * el apuntador lógico dado que el arreglo es de asignación estática (BSS/Data).
 */
void dataset_limpiar(void);

/**
 * @brief Calcula el valor esperado (promedio aritmético) de una característica específica.
 *
 * @param indice_variable Indicador escalar mapeado por las constantes VAR_* (e.g. VAR_SST).
 * @return double Media aritmética calculada sobre la totalidad de la muestra disponible.
 */
double dataset_promedio_variable(int indice_variable);

/**
 * @brief Computa iterativamente los límites numéricos (infimo y supremo) de las variables.
 *
 * Fundamental como preprocesamiento estadístico antes de aplicar normalización
 * min-max para la convergencia espacial del algoritmo Fuzzy C-Means en el Módulo 3.
 *
 * @param min_out Arreglo de salida donde se almacenan las cotas inferiores (min).
 * @param max_out Arreglo de salida donde se almacenan las cotas superiores (max).
 */
void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]);

/**
 * @brief Segmenta el dataset extrayendo un subconjunto dentro de una ventana temporal.
 *
 * Actúa como un filtro pasabanda en el dominio del tiempo (años). Útil para
 * el particionamiento Hold-out de datos (Set de entrenamiento vs Set de prueba).
 *
 * @param anio_inicio Límite inferior de la ventana temporal (inclusivo).
 * @param anio_fin Límite superior de la ventana temporal (inclusivo).
 * @param destino Puntero base al arreglo donde se instanciarán los registros copiados.
 * @param max_destino Tamaño de búfer reservado para evitar segment fault por desbordamiento.
 * @return int Conteo real de las observaciones extraídas con éxito.
 */
int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
                              RegistroClimatico* destino, int max_destino);

/**
 * @brief Inyecta la clasificación real (Ground Truth) en un registro preexistente.
 *
 * @param index Índice del registro donde se modificará el atributo de salida.
 * @param valor Etiqueta binaria representativa del acaecimiento de un ciclón tropical (0 o 1).
 * @return int Bandera booleana de confirmación: 1 para mutación exitosa, 0 ante error.
 */
int dataset_marcar_ciclon(int index, int valor);


#endif // MODULO1_DATASET_H
