# Módulo 5: Evaluador (Matriz de Confusión)

## 1. Propósito y Alcance
El Módulo 5 tiene la responsabilidad exclusiva de cuantificar el rendimiento y la precisión de la IA. Compara las predicciones teóricas del sistema (¿hubo huracán matemáticamente hablando?) contra la realidad climática de lo que verdaderamente ocurrió, arrojando métricas de Exactitud, Precisión y Recall.

---

## 2. Descripción General y Arquitectura
Su operación es estática y post-entrenamiento. 
1. Recibe un lote de pruebas evaluado por el Módulo 4.
2. Analiza los registros históricos reales, midiendo un parámetro duro (ej. Velocidad Máxima del Viento sostenida > Umbral) para saber si ese día de la historia hubo realmente un ciclón.
3. Lo contrasta con el `riesgo_difuso` que predijo la red (evaluado contra un Umbral de Peligro, ej. > 70%).
4. Rellena una **Matriz de Confusión** (Verdaderos Positivos, Falsos Negativos, etc.).

---

## 3. Especificaciones y Explicación del Código

### 3.1 Criterio de Verdad (Ground Truth)
```c
int evaluador_hubo_ciclon_real(const RegistroClimatico* reg) {
    if (reg->viento_max >= UMBRAL_VIENTO_REAL_KMH || reg->precip_suma >= UMBRAL_LLUVIA_REAL_MM) {
        return 1;
    }
    return 0;
}
```
**Descripción:** Esta función define qué es un ciclón *en el mundo real*. Si el viento documentado superó un umbral devastador (ej. 118 km/h para categoría huracán), entonces se marca con un `1` (Verdadero). Esta es la vara con la que mediremos si nuestra IA miente o no.

### 3.2 Construcción de la Matriz Cuadrante
```c
for (int i = 0; i < n_prueba; i++) {
    int real = evaluador_hubo_ciclon_real(&datos_prueba[i]);
    
    double riesgo = entrenamiento_obtener_riesgo(i);
    int predicho = (riesgo >= umbral_fcm) ? 1 : 0;
    
    if (real == 1 && predicho == 1) matriz->tp++;
    else if (real == 0 && predicho == 0) matriz->tn++;
    else if (real == 0 && predicho == 1) matriz->fp++;
    else if (real == 1 && predicho == 0) matriz->fn++;
}
```
**Descripción:** El núcleo de Data Science del módulo. Para cada día probado, consulta el mundo real y el mundo matemático. Si ambos coinciden en "Peligro" se registra un `True Positive` (TP). Si la IA dio falsas alarmas, un `False Positive` (FP). Si la IA no vio venir la tragedia, un temido `False Negative` (FN).

### 3.3 Extracción de Métricas Derivadas
```c
double evaluador_accuracy(const MatrizConfusion* m) {
    int total = m->tp + m->tn + m->fp + m->fn;
    return (double)(m->tp + m->tn) / total;
}
```
**Descripción:** Procesa los cuadrantes para generar porcentajes digeribles para interfaces de usuario (Exactitud global del modelo, Precisión, Tasa de falsas alarmas, etc.).

---

## 4. Justificación Técnica: ¿Por qué debe ser así?

> [!TIP]
> **Decisión Matemática: Separar el Umbral de FCM del Umbral Real**

**El Por qué:**
Notarás que el viento se evalúa estáticamente (`UMBRAL_VIENTO_REAL_KMH`), pero la alerta de IA se evalúa mediante un umbral dinámico o paramétrico (`umbral_fcm`). 
Esto se diseñó así porque la Inteligencia Artificial (FCM) es un sistema probabilístico. Un riesgo matemático de `0.6 (60%)` puede ser suficiente para alertar a la población si priorizamos salvar vidas (aumentando falsos positivos), o podemos exigir `0.9 (90%)` si no queremos causar pánico innecesario (bajando falsos positivos pero corriendo el riesgo de omisión). 
Mantenerlos separados y paramétricos permite dibujar "Curvas ROC" completas en la aplicación Web.
