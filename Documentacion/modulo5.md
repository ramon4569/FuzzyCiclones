# Documentación del Módulo 5: Evaluador y Validación

**Responsable:** Andrea
**Archivos:** `core/archivos.h/modulo5_evaluador.h`, `core/archivos.c/modulo5_evaluador.c`

## 🎯 Objetivo General
Este módulo responde directamente a la consigna de validación del proyecto. Se encarga de contrastar las predicciones hechas por el modelo difuso (Fuzzy C-Means del Módulo 4) utilizando datos que el modelo nunca vio (año 2017), contra la realidad climática (HURDAT2 provisto por el Módulo 2).

## ⚙️ Funciones y Lógica Interna

### `evaluador_calcular_metricas`
**Firma:** `ResultadoEvaluacion evaluador_calcular_metricas(double umbral);`

1. **Recolección Aislada de Datos:** Solicita al `Módulo 1` exclusivamente los registros de prueba correspondientes al año 2017 mediante la función `dataset_filtrar_por_anio`. Esto aísla por completo la validación del entrenamiento.
2. **Cálculo de Matriz de Confusión:** Realiza una iteración lineal (`O(N)`) a través de los registros de 2017:
   - Extrae el riesgo predicho a través del `Módulo 4` llamando a `entrenamiento_obtener_riesgo(i)`.
   - Lo discretiza utilizando el `umbral` paramétrico (ej. `0.5`).
   - Compara esta predicción contra la verdad absoluta `registro.hubo_ciclon` categorizando el resultado en VP, FP, VN o FN.
3. **Métricas de Rendimiento:** Calcula los índices matemáticos de rendimiento:
   - **Exactitud (Accuracy)**
   - **Precisión**
   - **Sensibilidad (Recall)**
   - **F1-Score**
   - *Nota de seguridad:* Todas las operaciones matemáticas están envueltas en validaciones condicionales `if (denominador > 0)` para proteger al motor WebAssembly contra errores críticos (crashes) ocasionados por divisiones por cero.

### `evaluador_generar_reporte_json`
**Firma:** `char* evaluador_generar_reporte_json(double umbral);`

Convierte de forma ultrarrápida la salida matemática de `evaluador_calcular_metricas` en un objeto textual JSON para que el `Módulo 6 (API Bridge)` lo retorne sin complicaciones al JavaScript del FrontEnd (Módulo 7).

## ⚡ Análisis de Eficiencia (Rendimiento)
La implementación elegida para el Módulo 5 fue diseñada buscando el máximo rendimiento y menor consumo de memoria en un contexto WebAssembly:
- **Paso Único (O(N)):** Los cálculos se efectúan en una única pasada (single-pass iterativo) sobre el set de pruebas, lo cual representa el límite teórico de mayor rapidez (algoritmo lineal puro).
- **Gestión de Memoria en Stack (Pila):** La instanciación del array de prueba se hace a nivel local y el string del JSON en un `static char buffer`, evitando el uso de `malloc()` y `free()`. Esto es importantísimo porque en WebAssembly la fragmentación de la memoria del *Heap* (montículo) y los *memory leaks* pueden ser problemas graves.
- **Construcción Ligera de JSON:** El formateo se realiza de forma nativa en C con una simple y óptima llamada a `snprintf`, evitando importar dependencias o librerías pesadas (como `cJSON`). Esto mantiene el archivo binario `.wasm` sumamente ligero y veloz para el navegador.

## 🔗 Dependencias y Orden de Ejecución
Este módulo actúa en la última etapa del ciclo de vida del flujo de predicción. Su uso **asume estrictamente** que:
1. El Módulo 1 ha sido inicializado.
2. El Módulo 2 ha importado los datos (incluyendo 2017).
3. El Módulo 3 y 4 han entrenado sobre 2015-2016 y ejecutado `entrenamiento_predecir()` sobre los datos de 2017, poblando así las variables requeridas en memoria.
