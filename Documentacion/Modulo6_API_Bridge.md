# Módulo 6 — API Bridge (Emscripten)

**Archivo:** `api/api_bridge.c`
**Responsable:** Joseidy

## Objetivo del módulo

Servir de puente entre la lógica en C (Módulos 1 a 5) y el frontend en JavaScript (Módulo 7). Ninguna función de este archivo implementa lógica matemática, de parseo o de validación propia: cada función arma un JSON de entrada/salida y delega el trabajo real en los otros módulos.

Cada función marcada con `EMSCRIPTEN_KEEPALIVE` queda exportada al compilar con Emscripten y puede invocarse desde JavaScript mediante `M.ccall(...)`.

## Estado interno del módulo

```c
static RegistroClimatico g_buffer_prueba[MAX_REGISTROS];
static int g_total_prueba = 0;
```

Buffer auxiliar donde se guardan los registros de **prueba** (ej. el año 2017), separado del dataset general del Módulo 1, para no mezclar datos de entrenamiento con datos de prueba antes de predecir.

```c
static const char* formato_a_texto(FormatoArchivo formato);
```

Helper interno (no exportado) que convierte el enum `FormatoArchivo` del Módulo 2 (`FORMATO_CSV`, `FORMATO_JSON`, `FORMATO_HURDAT2`, `FORMATO_DESCONOCIDO`) a un string legible para incluir en las respuestas JSON. Es solo texto de presentación, por eso vive en el bridge y no en el Módulo 2.

## Funciones exportadas

### `char* api_importar_datos(char* contenido)`

Recibe como string el contenido de un archivo (CSV, JSON o HURDAT2) leído en el navegador. Detecta el formato con `importador_detectar_formato()` e importa los datos con `importar_archivo()` (ambas del Módulo 2).

**Retorna JSON:**
```json
{"exito":1,"formato_detectado":"csv","importados":123}
```
Si `importar_archivo()` retorna un valor negativo, `exito` queda en `0` e `importados` en `0`.

### `int api_dataset_total(void)`

Delega directo en `dataset_total()` (Módulo 1). Retorna la cantidad de registros actualmente en el dataset.

### `void api_dataset_limpiar(void)`

Delega directo en `dataset_limpiar()` (Módulo 1). Vacía el dataset en memoria.

### `char* api_cargar_prueba(char* contenido, int anio_inicio, int anio_fin)`

**Función agregada durante el desarrollo del módulo**, no listada explícitamente en el reparto original de funciones del proyecto. El código base entregado a Módulo 6 tenía un comentario en `api_predecir()` que mencionaba una función `api_cargar_prueba()` como responsable de dejar los datos de prueba listos, pero esa función nunca estaba declarada ni implementada en ningún header ni en el `.c` base — era un hueco en el contrato original.

Se decidió implementarla en este módulo porque encaja con su responsabilidad (encadenar llamadas a otros módulos sin lógica propia) y resuelve, en un solo paso, lo que antes requería dos llamadas separadas:

1. Importa el contenido crudo del archivo de prueba (ej. el CSV de 2017) al dataset del Módulo 1, usando `importar_archivo()` (Módulo 2) — igual que cualquier otro archivo.
2. Inmediatamente filtra ese mismo rango de años hacia `g_buffer_prueba` usando `dataset_filtrar_por_anio()` (Módulo 1), dejando todo listo para que `api_predecir()` pueda usarse sin pasos adicionales.

**Retorna JSON:**
```json
{"exito":1,"importados":123,"total_prueba":123}
```
Si la importación falla (`importar_archivo()` retorna negativo), se corta ahí y retorna:
```json
{"exito":0,"importados":0,"total_prueba":0,"mensaje":"no se pudo importar el archivo de prueba"}
```

### `char* api_entrenar(int anio_inicio, int anio_fin, int n_clusters, double m, int max_iter, double epsilon)`

Llama a `entrenamiento_entrenar(...)` (Módulo 4), que a su vez usa el rango de años para entrenar el modelo FCM (Módulo 3) y fijar los centroides.

Si `entrenamiento_entrenar` retorna un valor negativo (error), responde:
```json
{"exito":0,"iteraciones":0,"mensaje":"error al entrenar: revisar rango de anios o datos cargados"}
```

Si entrena correctamente, recorre `fcm_obtener_num_clusters()` centroides llamando a `fcm_obtener_centroide()` (Módulo 3) por cada uno, usando los índices `VAR_SST`, `VAR_PRESION`, `VAR_HUMEDAD`, `VAR_VIENTO`, `VAR_CIZALLADURA` de `registro.h` para mantener el orden correcto de variables, y arma:

```json
{
  "exito": 1,
  "iteraciones": 42,
  "n_clusters": 3,
  "centroides": [
    {"cluster":0,"sst":28.4,"presion":1004.2,"humedad":78.1,"viento":32.5,"cizalladura":9.3},
    {"cluster":1,"sst":29.8,"presion":998.7,"humedad":82.3,"viento":54.0,"cizalladura":6.1}
  ]
}
```

### `char* api_predecir(void)`

Corre la predicción sobre los registros ya cargados en `g_buffer_prueba` mediante `api_cargar_prueba()`. Si no hay registros de prueba (`g_total_prueba <= 0`) o si `entrenamiento_predecir()` falla, retorna un array vacío `[]`.

Si hay datos, llama a `entrenamiento_predecir(g_buffer_prueba, g_total_prueba)` (Módulo 4), y por cada registro arma un objeto con la fecha, el riesgo obtenido de `entrenamiento_obtener_riesgo(i)` y la etiqueta real `hubo_ciclon` ya presente en el registro:

```json
[
  {"anio":2017,"mes":9,"dia":6,"riesgo":0.87,"hubo_ciclon":1},
  {"anio":2017,"mes":9,"dia":7,"riesgo":0.12,"hubo_ciclon":0}
]
```

### `char* api_evaluar(double umbral)`

Delega directo, sin transformación adicional, en `evaluador_generar_reporte_json(umbral)` (Módulo 5). Ese módulo ya devuelve el JSON final con la matriz de confusión y las métricas.

### `char* api_obtener_centroides(void)`

Similar a la parte de centroides de `api_entrenar`, pero como endpoint independiente: permite al frontend pedir los centroides ya entrenados (ej. para repintar el heatmap) sin necesidad de volver a entrenar el modelo.

```json
[
  {"cluster":0,"sst":28.4,"presion":1004.2,"humedad":78.1,"viento":32.5,"cizalladura":9.3}
]
```

## Contrato de exportación (Emscripten)

Las siguientes funciones deben estar declaradas en `EXPORTED_FUNCTIONS` dentro de `compilar.bat` para que `M.ccall()` pueda invocarlas desde JavaScript:

```
_api_importar_datos
_api_cargar_prueba
_api_dataset_total
_api_dataset_limpiar
_api_entrenar
_api_predecir
_api_evaluar
_api_obtener_centroides
```

## Notas de diseño

- Todas las funciones que retornan `char*` usan buffers `static` internos, para que el puntero devuelto siga siendo válido cuando JavaScript lo lee a través de `UTF8ToString()`.
- Ninguna función parsea CSV/JSON, hace cálculos de FCM, ni calcula métricas — todo eso se delega en los Módulos 1 a 5, respetando el diseño de "cada módulo trabaja en paralelo sin bloquear a los demás".
- Este archivo **no tiene un `.h` propio**, a diferencia de los Módulos 1-5. Esto es intencional: ningún otro archivo `.c` hace `#include` de `api_bridge.c` ni llama a sus funciones en tiempo de compilación — se invocan exclusivamente desde JavaScript vía `M.ccall()`, usando el nombre de la función como string y la lista de `EXPORTED_FUNCTIONS` de `compilar.bat`. Por la misma razón, `main.c` tampoco tiene un header propio.
