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

// Inicializa el dataset dejando el contador de registros en cero.
void dataset_inicializar(void);

// Inserta una copia del registro al final del dataset.
// Retorna el índice donde quedó almacenado o -1 si el dataset está lleno.
int dataset_insertar(RegistroClimatico r);

// Retorna una copia del registro ubicado en la posición indicada.
// Si el índice es inválido devuelve un RegistroClimatico vacío.
RegistroClimatico dataset_get(int index);

// Retorna la cantidad de registros actualmente cargados
int dataset_total(void);

// Elimina todos los registros del dataset.
void dataset_limpiar(void);

// Retorna el promedio de una variable climatica sobre todo el dataset.
// indice_variable: usar las constantes VAR_* definidas en registro.h
double dataset_promedio_variable(int indice_variable);

// Calcula el minimo y el maximo de cada una de las NUM_VARIABLES
// variables climaticas presentes en el dataset. Se usa antes de
// correr FCM para poder normalizar (Modulo 3 trabaja mejor con
// variables en rangos comparables, ej. 0-1).
void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]);

// Filtra registros por rango de anios y los copia a un buffer destino.
// Util para separar "entrenamiento 2015-2016" de "prueba 2017".
// Retorna la cantidad de registros copiados a destino.
int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
    RegistroClimatico* destino, int max_destino);

// Actualiza el campo hubo_ciclon de un registro existente.
// Retorna 1 si la actualización fue exitosa o 0 si el índice es inválido.
int dataset_marcar_ciclon(int index, int valor);


#endif // MODULO1_DATASET_H