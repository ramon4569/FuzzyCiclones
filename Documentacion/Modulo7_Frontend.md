# Módulo 7: Frontend / Visualizador SPA

## 1. Propósito y Alcance
El Módulo 7 es la cara pública del sistema, la Capa de Presentación (View). Abarca todos los archivos web (`index.html`, `style.css`, `app.js`). 
Su propósito es facilitar una experiencia de usuario interactiva (User Experience) mediante una Single Page Application (SPA), inyectando datos externos de Open-Meteo, parametrizando los algoritmos en C y dibujando la evaluación de riesgo de manera hermosa e inmersiva.

---

## 2. Descripción General y Arquitectura
El Frontend actúa como el Director de Orquesta asíncrono. Ninguna lógica matemática reside aquí; si requiere evaluar huracanes, empaca los comandos y se los pasa a WebAssembly. 

**Arquitectura de UI (Librerías Híbridas):**
- **WASM (`M.ccall`)**: Comunicación binaria síncrona/asíncrona hacia C.
- **Leaflet.js**: Motor cartográfico.
- **Chart.js**: Renderizado interactivo de la matriz de riesgo y descomposiciones factoriales (XAI).

```mermaid
graph LR
    User[Interacción de Usuario] --> UI[HTML5/CSS3]
    UI --> Fetch[App.js REST API]
    Fetch --> OpenMeteo[archive-api.open-meteo.com]
    OpenMeteo --> JS_JSON[JSON Masivo]
    JS_JSON --> WASM[WebAssembly C-Module]
    WASM --> Cartography[Leaflet & Chart.js rendering]
```

---

## 3. Especificaciones y Explicación del Código

### 3.1 Instanciación Asíncrona del Módulo C
```javascript
let M;
FuzzyModule().then(mod => {
    M = mod;
    console.log("WASM cargado");
    // Oculta la pantalla de carga negra tras 5 segundos
});
```
**Descripción:** WebAssembly no compila al instante, requiere milisegundos y carga asíncrona. La UI encapsula este retardo con un telón de fondo de carga "Cargando Motor Matemático" para garantizar que ningún botón pueda ser pulsado hasta que el cerebro en C esté disponible (`M = mod`).

### 3.2 Invocación Inter-Lenguajes (ccall)
```javascript
let resultadoPtr = M.ccall(
    'api_entrenar',   // Función C a llamar
    'string',         // Tipo de retorno esperado
    ['number', 'number', 'number', 'number'], // Firma de tipos 
    [anioIni, anioFin, numClusters, m_val]    // Argumentos vivos
);
let respuesta = JSON.parse(M.UTF8ToString(resultadoPtr));
```
**Descripción:** La maravilla de la computación moderna. Javascript convierte transparentemente objetos y primitivas al modelo de memoria plana de C, ejecuta el código precompilado de muy bajo nivel de la matriz `Fuzzy C-Means`, y recibe un puntero hexadecimal que luego decodifica mediante `UTF8ToString` a una respuesta nativa de JS.

### 3.3 Renderizado Espacial (Leaflet)
```javascript
const iconCiclon = L.divIcon({
    className: 'ciclon-marker',
    html: `<div class="ciclon-pulse" style="background: red;"></div>`
});
L.marker([latitud, longitud], { icon: iconCiclon }).addTo(map);
```
**Descripción:** Cuando el evaluador matemático diagnostica días con huracanes (Umbral TP/FP superado), el frontend proyecta geográficamente el evento sobre las costas de Florida usando marcadores div personalizados por CSS, para proveer una métrica visual y cartográfica del daño modelado.

---

## 4. Justificación Técnica: ¿Por qué debe ser así?

> [!CAUTION]
> **Decisión de UX: Bloqueo Síncrono de Interfaz durante el Entrenamiento**

Durante el click de "Entrenar Modelo", el botón se deshabilita pero el navegador podría congelarse momentáneamente si el FCM se dispara a millones de iteraciones.

**El Por qué:**
Aunque Javascript soporta WebWorkers para ejecutar WebAssembly en un hilo separado (Thread) evitando bloqueos en el UI, se optó intencionalmente por ejecutar el motor matemático en el **Main Thread**. La razón fundamental es la drástica reducción de complejidad en la compilación y despliegue del proyecto universitario. Soportar Pthreads en WASM requeriría que el servidor final tenga habilitados los infames "SharedArrayBuffer" (cabeceras COOP y COEP obligatorias), lo que destruiría la capacidad del profesor y alumnos de correr la aplicación localmente haciendo doble click en el `index.html`. El sacrificio de un mini-congelamiento en pantalla vale completamente la extrema portabilidad y universalidad del sistema.
