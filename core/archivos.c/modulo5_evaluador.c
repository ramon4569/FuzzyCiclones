#include <stdio.h>
#include <string.h>
#include "../archivos.h/modulo5_evaluador.h"
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"

/**
 * @brief Evalúa empíricamente la robustez predictiva del modelo de inferencia.
 * 
 * PRECONDICIÓN: La rutina 'entrenamiento_predecir()' (Módulo 4) debió haberse
 * ejecutado a priori, dejando en memoria los scores de riesgo asignados a
 * cada vector del conjunto de validación cruzada.
 */
ResultadoEvaluacion evaluador_calcular_metricas(double umbral, const RegistroClimatico* pruebas, int n_pruebas) {
    // Inicialización a cero de todos los campos de la estructura de métricas
    ResultadoEvaluacion r = {0};

    // Estrategia de guardia: Fallo seguro (Fail-safe) ante entrada nula
    if (n_pruebas == 0) {
        return r;
    }

    // Iteración O(N) analizando la calidad de clasificación de cada tupla
    for (int i = 0; i < n_pruebas; i++) {
        RegistroClimatico registro = pruebas[i];
        
        // Extracción del score difuso generado previamente
        double riesgo = entrenamiento_obtener_riesgo(i);
        
        // Función escalón (Step function): Binarización de probabilidad a clase discreta
        int prediccion = (riesgo >= umbral) ? 1 : 0;
        
        // Consulta de la variable dependiente de control (Ground Truth)
        int realidad = registro.hubo_ciclon;

        // Distribución categórica dentro de la Matriz de Confusión
        if (prediccion == 1 && realidad == 1) {
            r.verdaderos_positivos++;   // Hit / Acierto de riesgo
        } else if (prediccion == 1 && realidad == 0) {
            r.falsos_positivos++;        // Falsa alarma (Type I Error)
        } else if (prediccion == 0 && realidad == 0) {
            r.verdaderos_negativos++;    // Rechazo correcto
        } else if (prediccion == 0 && realidad == 1) {
            r.falsos_negativos++;        // Omisión / Miss (Type II Error)
        }
    }

    // Población estática total (Cardinalidad del subconjunto evaluado)
    int total = r.verdaderos_positivos + r.verdaderos_negativos + r.falsos_positivos + r.falsos_negativos;
    
    // Cálculo de Exactitud (Accuracy): Razón de predicciones globalmente correctas
    if (total > 0) {
        r.exactitud = (double)(r.verdaderos_positivos + r.verdaderos_negativos) / total;
    } else {
        r.exactitud = 0.0;
    }

    // Cálculo de Precisión (Positive Predictive Value): Mitigación del Error Tipo I
    if ((r.verdaderos_positivos + r.falsos_positivos) > 0) {
        r.precision = (double)r.verdaderos_positivos / (r.verdaderos_positivos + r.falsos_positivos);
    } else {
        r.precision = 0.0;
    }

    // Cálculo de Sensibilidad / Recall (True Positive Rate): Mitigación del Error Tipo II
    if ((r.verdaderos_positivos + r.falsos_negativos) > 0) {
        r.sensibilidad = (double)r.verdaderos_positivos / (r.verdaderos_positivos + r.falsos_negativos);
    } else {
        r.sensibilidad = 0.0;
    }

    // Cálculo de F1-Score: Media armónica combinando precisión y exhaustividad
    if ((r.precision + r.sensibilidad) > 0.0) {
        r.f1_score = 2.0 * (r.precision * r.sensibilidad) / (r.precision + r.sensibilidad);
    } else {
        r.f1_score = 0.0;
    }

    return r;
}

/**
 * @brief Implementa el motor de serialización hacia una cadena JSON conformada.
 */
char* evaluador_generar_reporte_json(double umbral, const RegistroClimatico* pruebas, int n_pruebas) {
    // Almacenamiento persistente en segmento de datos (static buffer pool).
    // Precaución: Inadecuado para esquemas multi-hilo concurrentes.
    static char buffer[1024];
    
    // Tratamiento robusto de anomalías (Data Deprivation)
    if (n_pruebas == 0) {
        snprintf(buffer, sizeof(buffer), "{\"exito\":0,\"mensaje\":\"No hay datos de prueba\"}");
        return buffer;
    }

    // Obtención delegada de la matriz analítica principal
    ResultadoEvaluacion r = evaluador_calcular_metricas(umbral, pruebas, n_pruebas);

    snprintf(buffer, sizeof(buffer),
        "{\"exito\":1,\"umbral\":%.2f,"
        "\"matriz_confusion\":{\"verdaderos_positivos\":%d,\"falsos_positivos\":%d,\"verdaderos_negativos\":%d,\"falsos_negativos\":%d},"
        "\"metricas\":{\"accuracy\":%.4f,\"precision\":%.4f,"
        "\"recall\":%.4f,\"f1_score\":%.4f}}",
        umbral,
        r.verdaderos_positivos, r.falsos_positivos,
        r.verdaderos_negativos, r.falsos_negativos,
        r.exactitud, r.precision, r.sensibilidad, r.f1_score);

    return buffer;
}
