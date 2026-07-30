# Módulo 6: API Bridge (El Puente WebAssembly)

## 1. Propósito y Alcance
El Módulo 6 (`api_bridge.c`) es el **Controlador (Controller)** en el patrón MVC. Es la única puerta de entrada y salida entre el aislado mundo matemático de C (Backend) y el mundo interactivo de Javascript (Frontend). 

> [!IMPORTANT]
> Este módulo no calcula matemáticas ni dibuja gráficos. Actúa estrictamente como un embajador, traduciendo strings de texto asíncronos y punteros de memoria a formatos seguros que WebAssembly pueda transportar.

---

## 2. Descripción General y Arquitectura
Cuando compilamos el código C mediante Emscripten (`emcc`), la memoria de C queda confinada en un bloque binario estanco. Javascript no puede invocar funciones internas libremente. 
El puente resuelve esto exponiendo funciones públicas mediante la directiva `EMSCRIPTEN_KEEPALIVE`.

**Arquitectura de Interfaz:**
1. Expone `api_importar_datos` para que JS le pase los JSON de Open-Meteo.
2. Expone `api_entrenar` y recibe parámetros (clusters, epsilon) desde los controles deslizantes (Sliders) del HTML.
3. Expone `api_obtener_resultados` para que JS extraiga la matriz de confusión y los riesgos diarios para graficarlos.

---

## 3. Especificaciones y Explicación del Código

### 3.1 La directiva de Retención
```c
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
const char* api_importar_datos(const char* json_str) {
    // ...
}
```
**Descripción:** Durante la compilación, el optimizador agresivo de WebAssembly asume que si una función nunca es llamada dentro de `main.c`, es código muerto y la elimina. `EMSCRIPTEN_KEEPALIVE` le ordena al compilador: *"Obliga a incluir esta función en el binario final porque será invocada en el futuro desde el exterior (Javascript)"*.

### 3.2 Manejo de Respuestas mediante Buffers Seguros
```c
static char g_api_response_buffer[65536]; // 64 KB para respuestas JSON

EMSCRIPTEN_KEEPALIVE
const char* api_entrenar(int anio_ini, int anio_fin, int clusters, double m) {
    int iters = entrenamiento_entrenar(anio_ini, anio_fin, clusters, m, 100, 0.0001);
    
    snprintf(g_api_response_buffer, sizeof(g_api_response_buffer),
             "{\"status\":\"ok\", \"iteraciones\":%d}", iters);
             
    return g_api_response_buffer;
}
```
**Descripción:** Javascript espera respuestas inteligibles, preferentemente objetos. Como C no tiene objetos literales, este puente serializa los resultados matemáticos (ej. las iteraciones que tomó FCM) y los encapsula a mano en un string JSON puro, retornando un puntero hacia el buffer estático preasignado.

---

## 4. Justificación Técnica: ¿Por qué debe ser así?

> [!WARNING]
> **Decisión Arquitectónica: Buffer de Respuesta Estático Global vs Punteros Dinámicos**

Al retornar strings hacia JS, podríamos hacer un `malloc(65536)` y retornar el puntero.

**El Por qué:**
Cruzar la frontera de lenguajes con memoria dinámica es peligroso. Si retornamos un puntero alojado por `malloc`, Javascript (que sí usa Garbage Collector automático) tendría que invocar obligatoriamente a una función `_free()` exportada de WebAssembly al terminar de leer el string, o de lo contrario crearíamos una gravísima Fuga de Memoria (Memory Leak) en el cliente.
Al usar un `static char g_api_response_buffer[]`, sacrificamos 64KB fijos de RAM para siempre, pero ganamos invulnerabilidad: Javascript puede leer el puntero todas las veces que quiera sin temor a corromper la memoria del heap del navegador.
