#include <stdio.h>
#include <string.h>
#include "../archivos.h/modulo5_evaluador.h"
#include "../archivos.h/modulo4_entrenamiento.h"
#include "../archivos.h/modulo1_dataset.h"

ResultadoEvaluacion evaluador_calcular_metricas(double umbral) {
    ResultadoEvaluacion r = {0};

    // TODO:
    // 1. recorrer los registros de prueba (mismo orden que se le
    //    paso a entrenamiento_predecir())
    // 2. para cada uno: prediccion = entrenamiento_obtener_riesgo(i) >= umbral
    //    y realidad = registro.hubo_ciclon
    // 3. acumular en r.verdaderos_positivos / falsos_positivos /
    //    verdaderos_negativos / falsos_negativos segun corresponda
    // 4. al final, calcular:
    //    r.exactitud     = (VP+VN) / total
    //    r.precision      = VP / (VP+FP)   (cuidado con division por 0)
    //    r.sensibilidad    = VP / (VP+FN)
    //    r.f1_score         = 2*precision*recall / (precision+recall)

    return r;
}

char* evaluador_generar_reporte_json(double umbral) {
    static char buffer[1024];
    ResultadoEvaluacion r = evaluador_calcular_metricas(umbral);

    // TODO: revisar el formato final acordado con el Modulo 6/7,
    // este es un punto de partida razonable.
    snprintf(buffer, sizeof(buffer),
        "{\"umbral\":%.2f,"
        "\"matriz_confusion\":{\"vp\":%d,\"fp\":%d,\"vn\":%d,\"fn\":%d},"
        "\"metricas\":{\"exactitud\":%.4f,\"precision\":%.4f,"
        "\"sensibilidad\":%.4f,\"f1_score\":%.4f}}",
        umbral,
        r.verdaderos_positivos, r.falsos_positivos,
        r.verdaderos_negativos, r.falsos_negativos,
        r.exactitud, r.precision, r.sensibilidad, r.f1_score);

    return buffer;
}
