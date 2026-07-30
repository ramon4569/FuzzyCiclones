# Módulo 4: Entrenamiento y Predicción

## 1. Propósito y Alcance
El Módulo 4 es la **Capa de Lógica de Negocio** en la Arquitectura. Actúa como el puente interpretativo entre los datos climáticos en bruto (Módulo 1) y el corazón matemático de Machine Learning (Módulo 3). 
Se encarga de inyectarle contexto humano a los vectores, identificando automáticamente cuál de los clusters descubiertos por la inteligencia artificial es efectivamente un Ciclón Tropical/Huracán, usando meteorología.

---

## 2. Descripción General y Arquitectura
Este módulo controla los flujos de "Train" (aprender del pasado) y "Predict" (evaluar el futuro). 

**Flujo de Ejecución:**
1. Solicita al Dataset (M1) un segmento de años para entrenar.
2. Desempaqueta y envía las variables climáticas de esos años a las turbinas matemáticas de FCM (M3).
3. Analiza los resultados: De los 'n' clusters que se encontraron, aplica **heurística climatológica** para identificar el peligroso.
4. Queda "armado" en RAM para recibir datos nuevos y dictaminar su nivel de riesgo.

---

## 3. Especificaciones y Explicación del Código

### 3.1 Transcripción de Registros a Vectores
```c
int n_train = dataset_filtrar_por_anio(anio_inicio, anio_fin, g_buffer_filtrado, MAX_REGISTROS);
for (int i = 0; i < n_train; i++) {
    registro_a_vector(&g_buffer_filtrado[i], g_vectores_entrenamiento[i]);
}
```
**Descripción:** La capa FCM solo entiende matemáticas (`double[]`). Aquí desempaquetamos los `struct` de C (que contienen metadatos como el String de la fecha) y los destilamos a su más pura expresión flotante multidimensional para su procesamiento.

### 3.2 Heurística Meteorológica de Huracanes
```c
int entrenamiento_identificar_cluster_riesgo(void) {
    // ...
    double mejor_score = centroide[VAR_SST] - centroide[VAR_PRESION];
    for (int c = 1; c < g_n_clusters; c++) {
        double score = centroide[c][VAR_SST] - centroide[c][VAR_PRESION];
        if (score > mejor_score) mejor_cluster = c;
    }
    g_cluster_riesgo = mejor_cluster;
}
```
**Descripción:** De forma autónoma, el sistema determina qué cluster descubrió la IA que representa huracanes. Basado en las leyes físicas: *La temperatura superficial del mar (SST) altísima calienta el aire, lo hace subir y provoca una caída violenta en la Presión Atmosférica*. Al restar Presión a SST, buscamos matemáticamente esa anomalía térmica y barométrica.

### 3.3 Explicabilidad (XAI - Explainable AI)
```c
for (int k = 0; k < NUM_VARIABLES; k++) {
    double diff = g_vectores_prueba[index][k] - centroide[k];
    distancias_parciales[k] = diff * diff;
    afinidades_parciales[k] = 1.0 / (distancias_parciales[k] + 1e-10);
}
// normalizar a 100%
```
**Descripción:** No basta con decirle al usuario "Hay 80% de riesgo". El Módulo 4 provee una función detallada que descompone el riesgo por variable (ej. 40% culpa del viento, 10% lluvia). Mide la distancia fragmentada de cada variable al núcleo del huracán y la invierte para sacar "Afinidades de Responsabilidad".

---

## 4. Justificación Técnica: ¿Por qué debe ser así?

> [!WARNING]
> **Decisión Arquitectónica: Estado Interno "Lazy Evaluation"**

En el código vemos constructos como `if (g_cluster_riesgo < 0) { entrenamiento_identificar_cluster_riesgo(); }` cuando alguien intenta pedir un riesgo por primera vez.

**El Por qué:**
Esto se llama evaluación perezosa. El cálculo de heurísticas puede requerir recorrer docenas de centroides y hacer cruces flotantes. Al dejarlo como `lazy`, no castigamos el CPU de WebAssembly (provocando lags en la pantalla de carga del Frontend) a menos que el usuario *específicamente haga click* en una gráfica o pida evaluar un día. 
Garantizamos un renderizado inicial ultra rápido en JS.
