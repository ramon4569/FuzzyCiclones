# Predictor de Riesgo de Ciclones — Fuzzy C-Means

Sistema que usa Fuzzy C-Means para estimar el riesgo de ciclon en el
Caribe a partir de variables climaticas, y que se valida "prediciendo"
ciclones que ya ocurrieron (se entrena con 2015-2016 y se mide que
tanto acierta 2017).

> Los CSV en `data/` son datos de **ejemplo/placeholder** para probar
> el importador — no son datos climaticos reales verificados. Para el
> entregable final hay que reemplazarlos por datos reales (ver seccion
> "Fuentes de datos" mas abajo).

## Estructura

```
FuzzyCiclones/
├── core/
│   ├── archivos.h/
│   │   ├── registro.h                  <- entidad central (RegistroClimatico)
│   │   ├── modulo1_dataset.h           <- Modulo 1: dataset en memoria
│   │   ├── modulo2_importador.h        <- Modulo 2: importar CSV/JSON/HURDAT2
│   │   ├── modulo3_fcm_core.h          <- Modulo 3: nucleo matematico FCM
│   │   ├── modulo4_entrenamiento.h     <- Modulo 4: entrenar / predecir
│   │   └── modulo5_evaluador.h         <- Modulo 5: metricas y validacion
│   └── archivos.c/
│       ├── registro.c
│       ├── modulo1_dataset.c
│       ├── modulo2_importador.c
│       ├── modulo3_fcm_core.c
│       ├── modulo4_entrenamiento.c
│       └── modulo5_evaluador.c
├── api/
│   └── api_bridge.c              <- Modulo 6: puente C <-> JS (Emscripten)
├── frontend/                     <- Modulo 7: interfaz grafica
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   ├── fcm.js                    <- output WASM (generado al compilar)
│   └── fcm.wasm                  <- output WASM (generado al compilar)
├── data/
│   ├── entrenamiento_2015_2016.csv
│   └── prueba_2017.csv
├── build/                        <- copia de salida alternativa (opcional)
├── Documentacion/
│   └── Reparto_de_modulos.docx
├── compilar.bat
├── main.c
└── README.md
```

## Los 7 modulos

Ver el detalle completo en `Documentacion/Reparto_de_modulos.docx`. Resumen:

1. **Dataset** (`modulo1_dataset`) — memoria en RAM de los registros climaticos.
2. **Importador** (`modulo2_importador`) — parsea CSV / JSON / HURDAT2.
3. **Nucleo Fuzzy C-Means** (`modulo3_fcm_core`) — el algoritmo matematico puro.
4. **Entrenamiento y prediccion** (`modulo4_entrenamiento`) — entrena con 2015-2016, predice 2017.
5. **Evaluador** (`modulo5_evaluador`) — matriz de confusion, accuracy, precision, recall.
6. **API Bridge** (`api_bridge.c`) — puente Emscripten C <-> JS.
7. **Frontend / Visualizador** (`frontend/`) — interfaz HTML, mapa de calor de riesgo.

## Requisitos
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- Node.js o Python (para servir el frontend)

## 1. Activar el entorno de Emscripten
```powershell
C:\emsdk\emsdk_env.ps1
```

## 2. Compilar
Parado en la raiz del proyecto (`FuzzyCiclones/`), correr `compilar.bat`
o el comando equivalente:
```powershell
emcc core/archivos.c/registro.c core/archivos.c/modulo1_dataset.c core/archivos.c/modulo2_importador.c core/archivos.c/modulo3_fcm_core.c core/archivos.c/modulo4_entrenamiento.c core/archivos.c/modulo5_evaluador.c api/api_bridge.c main.c -I core/archivos.h -o frontend/fcm.js -s EXPORTED_FUNCTIONS="['_api_importar_datos','_api_dataset_total','_api_dataset_limpiar','_api_entrenar','_api_predecir','_api_evaluar','_api_obtener_centroides']" -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString']" -s MODULARIZE=1 -s EXPORT_NAME="FuzzyModule" -s ALLOW_MEMORY_GROWTH=1 -O2
```

## 3. Ejecutar
El frontend no funciona abriendo `index.html` directo (protocolo
`file://`); necesita servirse por HTTP.
```powershell
npx serve frontend
```
o
```bash
cd frontend && python -m http.server 8080
```

## Fuentes de datos reales sugeridas
- **HURDAT2 (NOAA)**: historico oficial de huracanes del Atlantico
  (fecha, posicion, presion, viento) desde 1851 — usar para poblar
  `hubo_ciclon` con el Modulo 2.
- **NOAA NCEI / NCDC / reanalisis (ERA5, NOAA OISST)**: temperatura
  superficial del mar, presion, humedad, viento por fecha y zona.

## Estado del proyecto
Este zip contiene la arquitectura y las firmas de funciones ya
definidas (headers completos) pero las implementaciones en los `.c`
estan vacias con comentarios `// TODO` explicando paso a paso que
hay que hacer. Cada modulo compila de forma independiente (probado
con `gcc -c`, sin Emscripten) para que cada integrante pueda arrancar
sin bloquear a los demas.
