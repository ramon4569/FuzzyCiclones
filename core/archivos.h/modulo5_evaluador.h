#ifndef MODULO5_EVALUADOR_H
#define MODULO5_EVALUADOR_H

#include "../archivos.h/registro.h"

// ==========================================================
// MODULO 5 — EVALUADOR Y VALIDACION
// Responsable: Andrea
//
// Compara el riesgo predicho por el Modulo 4 contra la realidad
// (campo hubo_ciclon de cada RegistroClimatico, poblado por el
// Modulo 2 al importar HURDAT2) y arma metricas objetivas.
// Esta es la parte que responde directamente a la consigna:
// "si le pasas datos de 2015-2016, que tanto acierta 2017".
// ==========================================================

typedef struct {
    int verdaderos_positivos;   // dijo alto riesgo Y hubo ciclon
    int falsos_positivos;        // dijo alto riesgo Y NO hubo ciclon
    int verdaderos_negativos;     // dijo bajo riesgo Y NO hubo ciclon
    int falsos_negativos;          // dijo bajo riesgo Y SI hubo ciclon

    double exactitud;      // accuracy   = (VP+VN) / total
    double precision;       // precision  = VP / (VP+FP)
    double sensibilidad;     // recall     = VP / (VP+FN)
    double f1_score;          // f1         = 2 * (precision*recall)/(precision+recall)
} ResultadoEvaluacion;

/**
 * @brief Computa la matriz de confusión y derivadas métricas de desempeño.
 * 
 * Itera sobre el conjunto de pruebas (test set) validando la inferencia
 * estocástica (riesgo) frente al Ground Truth (hubo_ciclon) utilizando un
 * umbral de decisión discriminante.
 * 
 * @param umbral Límite de activación [0.0, 1.0] para binarizar el riesgo continuo.
 * @param pruebas Arreglo conteniendo la partición de evaluación.
 * @param n_pruebas Tamaño del lote de pruebas.
 * @return ResultadoEvaluacion Estructura consolidada con VP, FP, VN, FN y métricas (F1-score, Recall, etc).
 */
ResultadoEvaluacion evaluador_calcular_metricas(double umbral, const RegistroClimatico* pruebas, int n_pruebas);

/**
 * @brief Serializa los resultados de la evaluación a notación de objetos (JSON).
 * 
 * Prepara el paquete de red (Payload) para ser transmitido hacia el frontend
 * a través de la capa API (Módulo 6).
 * 
 * @param umbral Umbral de corte empleado en la clasificación binaria.
 * @param pruebas Arreglo con la partición de evaluación.
 * @param n_pruebas Tamaño del lote de pruebas.
 * @return char* Puntero a buffer estático de memoria (advertencia: no-reentrante ni thread-safe) con el JSON.
 */
char* evaluador_generar_reporte_json(double umbral, const RegistroClimatico* pruebas, int n_pruebas);

#endif // MODULO5_EVALUADOR_H
