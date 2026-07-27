#ifndef MODULO5_EVALUADOR_H
#define MODULO5_EVALUADOR_H

// ==========================================================
// MODULO 5 — EVALUADOR Y VALIDACION
// Responsable: (asignar integrante)
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

// Recorre los registros de prueba ya evaluados por el Modulo 4 y arma
// la matriz de confusion, usando 'umbral' para decidir si
// entrenamiento_obtener_riesgo(i) >= umbral cuenta como "predijo ciclon".
ResultadoEvaluacion evaluador_calcular_metricas(double umbral);

// Genera un reporte en formato JSON con la matriz de confusion y las
// metricas, listo para que el Modulo 6 (API bridge) lo devuelva al frontend.
char* evaluador_generar_reporte_json(double umbral);

#endif // MODULO5_EVALUADOR_H
