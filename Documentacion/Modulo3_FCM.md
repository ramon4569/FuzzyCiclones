# Módulo 3: Núcleo Matemático (Fuzzy C-Means)

## 1. Propósito y Alcance
El Módulo 3 es el "Cerebro Algorítmico" de todo el sistema. Implementa exclusivamente la matemática del *Fuzzy C-Means Clustering* (FCM). 
Es un módulo agnóstico: no sabe si está analizando huracanes, píxeles de una imagen o clientes de un banco. Su único alcance es recibir un grupo de coordenadas (matrices multidimensionales), iterar sobre ellas y agruparlas basándose en su densidad y distancia métrica.

---

## 2. Descripción General y Arquitectura
El algoritmo FCM es una variante del aprendizaje automático no supervisado (Machine Learning Unsupervised). A diferencia del clustering rígido (Hard C-Means / K-Means) donde un registro pertenece solo a un grupo, en FCM cada registro tiene un "Porcentaje de Pertenencia" distribuido entre todos los grupos.

**Flujo de Ejecución (Bucle Matemático):**
1. **Inicialización:** Matrices difusas al azar.
2. **Cálculo de Centros:** El centro de gravedad de cada cluster es movido hacia donde haya puntos con mayor membresía.
3. **Cálculo de Membresía:** Las membresías de cada punto se ajustan basadas en qué tan lejos están de los nuevos centros.
4. **Convergencia:** Regresar al paso 2 hasta que los centros de gravedad dejen de moverse.

---

## 3. Especificaciones y Explicación del Código

### 3.1 Matrices Globales de Estado
```c
static double g_membresias[MAX_REGISTROS][MAX_CLUSTERS];
static double g_centroides[MAX_CLUSTERS][NUM_VARIABLES];
```
**Descripción:** La Memoria Artificial. `g_membresias` es la Matriz U (de dimensiones $N \times C$) que guarda los porcentajes (ej. el día 1 es 90% huracán, 10% despejado). `g_centroides` es la Matriz V que almacena las coordenadas físicas del "Torbellino Perfecto" que descubrió la IA.

### 3.2 Paso 1: Mover los Centroides
```c
double peso = pow(g_membresias[i][j], m);
denominador += peso;
numerador[k] += peso * datos[i][k];
g_centroides[j][k] = numerador[k] / denominador;
```
**Descripción:** Fórmula matemática real de centroides FCM: $v_j = \frac{\sum (u_{ij}^m \cdot x_i)}{\sum u_{ij}^m}$. Aquí, `m` es el exponente difuso. Actúa como un atractor gravitacional: los puntos con una membresía altísima arrastran poderosamente al centro de gravedad del cluster hacia ellos.

### 3.3 Paso 2: Actualizar las Membresías
```c
double dist = fcm_distancia_euclidiana(datos[i], g_centroides[j]);
// ...
for (int k = 0; k < n_clusters; k++) {
    suma += pow(distancias[j] / distancias[k], exponente);
}
g_membresias[i][j] = 1.0 / suma;
```
**Descripción:** Fórmula de actualización: $u_{ij} = \frac{1}{\sum (d_{ij}/d_{ik})^{\frac{2}{m-1}}}$. Es el Teorema Inverso a la Distancia. Si un día `i` (datos[i]) está muy cerca del centroide de huracanes `j`, la fracción $d_{ij}$ será pequeñísima, el denominador total encogerá y la membresía estallará hacia `1.0` (100%).

### 3.4 Inferencia sin alterar memoria
```c
void fcm_actualizar_membresias_con_centroides_fijos(...) {
    fcm_actualizar_membresias(datos, n_puntos, n_clusters, m);
}
```
**Descripción:** Se salta intencionalmente el Paso 1. Permite medir datos "futuros" contra el modelo entrenado sin alterar la memoria de los huracanes históricos ya documentados.

---

## 4. Justificación Técnica: ¿Por qué debe ser así?

> [!CAUTION]
> **Decisión Matemática: El uso del Exponente 'm' (Fuzziness factor) y pow()**

El código hace uso constante de la pesada instrucción `pow()` del CPU (para elevar bases a potencias flotantes).

**El Por qué:** En una IA rígida (`m=1`), la distancia no importa: si cruzas el umbral, eres 100% huracán o 0%. El exponente `m` (cuyo valor estándar global en la literatura es `2.0`) "suaviza" la línea divisoria. Permite que el sistema alerte al usuario con un "Cuidado, el clima tiene un 65% de similitud con un huracán". Sin `pow()`, la IA pierde los grises y es inútil para la meteorología (la cual es caótica por naturaleza). 

> [!TIP]
> **Condición de Singularidad (If divisor < 1e-10)**
El código posee prevenciones contra `división por cero` si un día coincide matemáticamente exacto (100%) con un centroide. Esta es una mitigación de seguridad obligatoria recomendada por los papers matemáticos originales del algoritmo FCM.
