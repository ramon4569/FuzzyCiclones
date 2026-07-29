# Módulo 7 — Frontend / Visualizador
## Especificación Técnica IEEE 830
**Proyecto:** Predictor de Riesgo de Ciclones — Fuzzy C-Means
**Responsable:** Rachel
**Rama:** m7
**Fecha:** 28-29 julio 2026

---

### 1. Introducción

**1.1 Objetivo**
Interfaz web que carga el módulo WASM (compilado desde los módulos 1-5 en C) y permite al usuario importar datos climáticos, entrenar el modelo Fuzzy C-Means, ejecutar predicciones y visualizar resultados de evaluación contra ciclones reales.

**1.2 Alcance**
El Módulo 7 no implementa ninguna lógica de negocio ni algoritmo. Su única responsabilidad es: recibir input del usuario, pasarlo a las funciones exportadas por el Módulo 6 (API Bridge) vía `M.ccall()`, y renderizar la respuesta JSON en la interfaz.

**1.3 Restricciones**
- Debe servirse por HTTP (no funciona con `file://` por restricciones de WASM)
- Requiere que `frontend/fcm.js` y `frontend/fcm.wasm` existan (generados por `compilar.bat` vía Emscripten)

---

### 2. Descripción General del Sistema

**2.1 Perspectiva del producto**

```
Usuario (navegador) → HTML/JS → M.ccall() → WASM (C) → Módulos 1-5
```

**2.2 Funciones principales implementadas**
1. Cargar archivo de entrenamiento (CSV/JSON/HURDAT2)
2. Cargar archivo de prueba y filtrarlo automáticamente por año
3. Configurar y ejecutar entrenamiento del modelo (clusters, m, iteraciones)
4. Visualizar centroides entrenados en tabla
5. Ejecutar predicción sobre datos de prueba
6. Visualizar mapa de calor de riesgo por fecha (bajo/moderado/alto)
7. Evaluar el modelo contra ciclones reales (matriz de confusión + métricas)

**2.3 Características del usuario**
Sin conocimiento técnico de WASM o C. Interactúa únicamente con formularios, botones y tablas.

---

### 3. Requisitos Específicos

**3.1 Requisitos funcionales implementados**

| ID | Requisito |
|----|-----------|
| RF-M7-01 | Cargar módulo WASM al iniciar, con pantalla de carga |
| RF-M7-02 | Importar archivo de entrenamiento vía `api_importar_datos()` |
| RF-M7-03 | Importar archivo de prueba vía `api_cargar_prueba()`, filtrado por año |
| RF-M7-04 | Configurar parámetros (clusters, m, iteraciones) y entrenar vía `api_entrenar()` |
| RF-M7-05 | Mostrar centroides entrenados en tabla |
| RF-M7-06 | Ejecutar predicción vía `api_predecir()` |
| RF-M7-07 | Mapa de calor con color por nivel de riesgo (verde/naranja/rojo) |
| RF-M7-08 | Evaluar modelo vía `api_evaluar()`, mostrar matriz de confusión y métricas |

**3.2 Requisitos no funcionales**

| ID | Requisito |
|----|-----------|
| RNF-M7-01 | Manejo de errores sin crashear (try/catch en cada llamada a WASM) |
| RNF-M7-02 | Mensajes claros al usuario ante fallos |
| RNF-M7-03 | Diseño responsivo básico (mobile) |
| RNF-M7-04 | Paleta de colores: fondo claro, texto oscuro, acentos para comparaciones/resultados |

---

### 4. Interfaz con el Módulo 6 (API Bridge)

Funciones WASM consumidas, todas vía `M.ccall()`:

```javascript
api_importar_datos(contenido)                          → string (JSON)
api_cargar_prueba(contenido, anio_inicio, anio_fin)     → string (JSON)
api_entrenar(anio_i, anio_f, n_clusters, m, iter, eps)  → string (JSON)
api_predecir()                                          → string (JSON, array)
api_evaluar(umbral)                                     → string (JSON)
```

---

### 5. Decisiones de diseño

- **DA-M7-01**: Sin frameworks — JavaScript vanilla + `M.ccall()` directo, siguiendo el patrón del proyecto de referencia (ProyectoBisect)
- **DA-M7-02**: Cada función de vista maneja su propio estado de error de forma independiente
- **DA-M7-03**: El color en la interfaz se reserva exclusivamente para resultados y comparaciones (mapa de calor, matriz de confusión); el resto de la UI usa fondo claro y texto oscuro
- **DA-M7-04**: Tarjetas de carga de archivo separadas visualmente (una por dataset) para diferenciar entrenamiento y prueba

---

### 6. Flujo de uso

1. El usuario abre la página y espera la carga del módulo WASM
2. Importa el archivo de entrenamiento (2015-2016) y el archivo de prueba (2017)
3. Ajusta los parámetros del modelo (clusters, exponente difuso, iteraciones) y entrena
4. Revisa los centroides resultantes
5. Ejecuta la predicción sobre los datos de prueba y observa el mapa de calor de riesgo por fecha
6. Define un umbral de riesgo y evalúa el modelo, obteniendo la matriz de confusión y las métricas de exactitud, precisión, sensibilidad y F1-score

---

### 7. Conclusión

El Módulo 7 cumple su función según lo especificado: importa datos, dispara entrenamiento, predicción y evaluación, y visualiza los resultados devueltos por el backend a través de las cuatro vistas de la interfaz.
