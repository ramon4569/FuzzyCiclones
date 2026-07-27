#ifndef MODULO4_ENTRENAMIENTO_H
#define MODULO4_ENTRENAMIENTO_H

#include "registro.h"

// ==========================================================
// MODULO 4 — ENTRENAMIENTO Y PREDICCION
// Responsable: (asignar integrante)
//
// Este modulo orquesta el Modulo 3 (nucleo FCM) para cumplir
// el objetivo central del proyecto:
//   1) Entrenar con datos historicos (ej. 2015-2016): esto fija
//      los centroides de "bajo / moderado / alto riesgo".
//   2) Predecir sobre datos que el modelo nunca vio (ej. 2017),
//      SIN mover los centroides — solo se recalculan las
//      membresias (mu) de los puntos nuevos contra los
//      centroides ya aprendidos.
// Esto es lo que permite decir "el sistema predijo Irma sin
// haber sido entrenado con datos de Irma".
// ==========================================================

// Entrena el modelo FCM usando los registros ya cargados en el
// dataset (Modulo 1) que caen dentro del rango de anios de
// entrenamiento. Deja los centroides fijos en memoria (Modulo 3)
// listos para ser reutilizados por entrenamiento_predecir().
// Retorna la cantidad de iteraciones que tardo en converger, -1 si error.
int entrenamiento_entrenar(int anio_inicio, int anio_fin,
                            int n_clusters, double m,
                            int max_iter, double epsilon);

// Corre la prediccion sobre un conjunto de registros de prueba
// (ej. todo 2017), usando UNICAMENTE los centroides ya fijados
// por entrenamiento_entrenar(). Internamente llama a
// fcm_actualizar_membresias_con_centroides_fijos().
// Retorna la cantidad de registros procesados, -1 si no hay
// un modelo entrenado todavia.
int entrenamiento_predecir(const RegistroClimatico* datos_prueba, int n);

// Identifica cual de los n_clusters entrenados corresponde a
// "alto riesgo de ciclon": tipicamente el que tiene mayor SST
// promedio y menor presion promedio en su centroide.
// Retorna el indice de ese cluster (0..n_clusters-1).
int entrenamiento_identificar_cluster_riesgo(void);

// Retorna el grado de pertenencia (0.0 - 1.0) del registro de
// prueba en la posicion 'index' al cluster de alto riesgo.
// Este numero ES el "riesgo de ciclon" que se reporta al usuario.
double entrenamiento_obtener_riesgo(int index);

#endif // MODULO4_ENTRENAMIENTO_H
