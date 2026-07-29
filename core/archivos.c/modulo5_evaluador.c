#include <stdio.h>
#include <string.h>
#include "../archivos.h/modulo5_evaluador.h"
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"

// NOTA: Esta funcion asume que entrenamiento_predecir() ya se ejecuto
// exitosamente desde el Modulo 4, y que los riesgos para los datos de prueba
// de 2017 ya estan disponibles en entrenamiento_obtener_riesgo().
ResultadoEvaluacion evaluador_calcular_metricas(double umbral, const RegistroClimatico* pruebas, int n_pruebas) {
    ResultadoEvaluacion r = {0};

    if (n_pruebas == 0) {
        // Retornamos todo en 0 como indicador de que no hubo datos para evaluar
        return r;
    }

    for (int i = 0; i < n_pruebas; i++) {

        RegistroClimatico registro = pruebas[i];
        double riesgo = entrenamiento_obtener_riesgo(i);
        
        int prediccion = (riesgo >= umbral) ? 1 : 0;
        int realidad = registro.hubo_ciclon;

        if (prediccion == 1 && realidad == 1) {
            r.verdaderos_positivos++;
        } else if (prediccion == 1 && realidad == 0) {
            r.falsos_positivos++;
        } else if (prediccion == 0 && realidad == 0) {
            r.verdaderos_negativos++;
        } else if (prediccion == 0 && realidad == 1) {
            r.falsos_negativos++;
        }
    }

    int total = r.verdaderos_positivos + r.verdaderos_negativos + r.falsos_positivos + r.falsos_negativos;
    
    if (total > 0) {
        r.exactitud = (double)(r.verdaderos_positivos + r.verdaderos_negativos) / total;
    } else {
        r.exactitud = 0.0;
    }

    if ((r.verdaderos_positivos + r.falsos_positivos) > 0) {
        r.precision = (double)r.verdaderos_positivos / (r.verdaderos_positivos + r.falsos_positivos);
    } else {
        r.precision = 0.0;
    }

    if ((r.verdaderos_positivos + r.falsos_negativos) > 0) {
        r.sensibilidad = (double)r.verdaderos_positivos / (r.verdaderos_positivos + r.falsos_negativos);
    } else {
        r.sensibilidad = 0.0;
    }

    if ((r.precision + r.sensibilidad) > 0.0) {
        r.f1_score = 2.0 * (r.precision * r.sensibilidad) / (r.precision + r.sensibilidad);
    } else {
        r.f1_score = 0.0;
    }

    return r;
}

char* evaluador_generar_reporte_json(double umbral, const RegistroClimatico* pruebas, int n_pruebas) {
    static char buffer[1024];
    
    if (n_pruebas == 0) {
        snprintf(buffer, sizeof(buffer), "{\"exito\":0,\"mensaje\":\"No hay datos de prueba\"}");
        return buffer;
    }

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
