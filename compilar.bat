@echo off
echo Compilando WebAssembly (Predictor de Ciclones)...
emcc core/archivos.c/registro.c core/archivos.c/modulo1_dataset.c core/archivos.c/modulo2_importador.c core/archivos.c/modulo3_fcm_core.c core/archivos.c/modulo4_entrenamiento.c core/archivos.c/modulo5_evaluador.c api/api_bridge.c main.c -I core/archivos.h -o frontend/fcm.js -s EXPORTED_FUNCTIONS="['_api_importar_datos','_api_dataset_total','_api_dataset_limpiar','_api_entrenar','_api_predecir','_api_evaluar','_api_obtener_centroides']" -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString']" -s MODULARIZE=1 -s EXPORT_NAME="FuzzyModule" -s ALLOW_MEMORY_GROWTH=1 -O2

if %errorlevel% neq 0 (
    echo.
    echo ========================================
    echo ERROR: La compilacion fallo. Lee el mensaje de arriba.
    echo ========================================
) else (
    echo.
    echo ========================================
    echo EXITO: Compilacion completada correctamente.
    echo ========================================
)
