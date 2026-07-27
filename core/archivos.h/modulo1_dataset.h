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

// Inicializa (limpia) el dataset en memoria
void dataset_inicializar(void);

// Inserta un registro al final del dataset.
// Retorna el indice donde quedo insertado, o -1 si esta lleno.
int dataset_insertar(RegistroClimatico r);

// Retorna el registro en la posicion indicada
RegistroClimatico dataset_get(int index);

// Retorna la cantidad de registros actualmente cargados
int dataset_total(void);

// Vacia el dataset (equivalente a inicializar, se deja separado
// por claridad semantica: "reiniciar" vs "limpiar antes de importar")
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

#endif // MODULO1_DATASET_H
