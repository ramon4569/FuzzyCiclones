# Módulo 1 — Dataset y estructuras de datos

**Archivos:** `core/archivos.h/modulo1_dataset.h`, `core/archivos.c/modulo1_dataset.c`  
**Rama:** `m1`  
**Estado:** completo.

---

## Objetivo

Administrar el almacenamiento en memoria de todos los registros climáticos del sistema.

Este módulo funciona como la "memoria" del proyecto: mantiene un arreglo estático de `RegistroClimatico` y ofrece operaciones para insertar, consultar, filtrar y analizar registros.

No conoce el formato de los archivos ni implementa lógica relacionada con Fuzzy C-Means, entrenamiento o predicción. Su única responsabilidad es administrar los datos cargados por otros módulos.

---

# Estructura interna

El dataset se almacena mediante:

```c
static RegistroClimatico g_dataset[MAX_REGISTROS];
static int g_total;
```

- `g_dataset` contiene todos los registros cargados.
- `g_total` indica cuántos registros están actualmente almacenados.

No se utiliza memoria dinámica.

---

# Funciones implementadas

## `void dataset_inicializar(void)`

Reinicia el dataset estableciendo el contador interno en cero.

No limpia físicamente el arreglo, ya que los registros anteriores quedan inaccesibles al reiniciar el contador.

---

## `void dataset_limpiar(void)`

Vacía completamente el dataset.

Su implementación es equivalente a `dataset_inicializar()`, pero se mantiene como función independiente por claridad semántica.

---

## `int dataset_insertar(RegistroClimatico r)`

Inserta un nuevo registro al final del dataset.

Comportamiento:

- valida que exista espacio disponible (`MAX_REGISTROS`)
- copia el registro recibido
- incrementa el contador
- devuelve el índice donde fue almacenado

Si el dataset está lleno retorna:

```c
-1
```

---

## `RegistroClimatico dataset_get(int index)`

Obtiene una copia del registro ubicado en la posición indicada.

Si el índice recibido no es válido, devuelve un `RegistroClimatico` inicializado completamente en cero.

Nunca produce acceso fuera de rango.

---

## `int dataset_total(void)`

Devuelve la cantidad de registros actualmente almacenados.

---

## `double dataset_promedio_variable(int indice_variable)`

Calcula el promedio de cualquiera de las variables climáticas definidas en `registro.h`:

- `VAR_SST`
- `VAR_PRESION`
- `VAR_HUMEDAD`
- `VAR_VIENTO`
- `VAR_CIZALLADURA`

Si el dataset está vacío retorna `0.0`.

---

## `void dataset_min_max(...)`

Calcula el valor mínimo y máximo de cada variable climática.

Los resultados se almacenan en:

```c
min_out[]
max_out[]
```

Si el dataset está vacío, ambos arreglos se rellenan con `0.0`.

Esta información será utilizada por el Módulo 3 para normalizar los datos antes de ejecutar Fuzzy C-Means.

---

## `int dataset_filtrar_por_anio(...)`

Recorre el dataset y copia únicamente los registros cuyo año pertenezca al rango indicado.

Respeta el límite del arreglo destino (`max_destino`).

Retorna la cantidad de registros copiados.

Este método será utilizado principalmente por el Módulo 4 para separar:

- entrenamiento (2015–2016)
- prueba (2017)

---

## `int dataset_marcar_ciclon(int index, int valor)`

Permite modificar el campo `hubo_ciclon` de un registro ya almacenado.

Fue agregada para permitir la integración con el Módulo 2, ya que `dataset_get()` devuelve una copia del registro y no una referencia modificable.

Comportamiento:

- valida el índice recibido
- actualiza `hubo_ciclon`
- retorna `1` si la operación fue exitosa
- retorna `0` si el índice es inválido

---

# Decisiones de diseño

## Arreglo estático

Se decidió utilizar un arreglo estático para evitar el uso de memoria dinámica y simplificar la compilación con Emscripten.

---

## Validación de índices

Toda operación que accede a un registro verifica previamente que el índice pertenezca al rango válido.

Esto evita accesos fuera de memoria.

---

## Función auxiliar

El módulo incorpora una función privada:

```c
variable_de()
```

que permite acceder a cualquiera de las variables climáticas utilizando las constantes `VAR_*`.

Esto evita duplicar código en:

- promedio
- mínimos
- máximos

---

# Integración con otros módulos

## Módulo 2

Utiliza:

- `dataset_insertar()`
- `dataset_get()`
- `dataset_total()`
- `dataset_marcar_ciclon()`

Esta última función fue incorporada específicamente para permitir que el importador HURDAT2 marque correctamente los registros donde existió un ciclón.

---

## Módulo 3

Utilizará:

- `dataset_min_max()`

para normalizar las variables antes de ejecutar Fuzzy C-Means.

---

## Módulo 4

Utilizará:

- `dataset_filtrar_por_anio()`

para separar los conjuntos de entrenamiento y prueba.

---

# Pruebas realizadas

Se verificó el correcto funcionamiento de:

- inserción de registros
- consulta por índice
- validación de índices inválidos
- conteo de registros
- promedio de variables
- cálculo de mínimos y máximos
- filtrado por año
- actualización del campo `hubo_ciclon`

---

# Historial de implementación (rama `m1`)

1. Implementar almacenamiento estático del dataset.
2. Implementar inserción y consulta de registros.
3. Implementar cálculo de promedios.
4. Implementar cálculo de mínimos y máximos.
5. Implementar filtrado por rango de años.
6. Agregar `dataset_marcar_ciclon()` para integración con el Módulo 2.