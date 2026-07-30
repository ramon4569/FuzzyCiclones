#include <stdio.h>
#include "core/archivos.h/modulo1_dataset.h"

/**
 * @brief Punto de entrada principal (Entry Point) - Fase de inicialización.
 * 
 * Arquitectura basada en WebAssembly: La subrutina main actúa exclusivamente
 * como Bootstrapper de memoria. Toda la delegación de eventos y orquestación
 * del ciclo de vida asíncrono se delega al Módulo 6 (API Bridge) invocado
 * mediante ccall/cwrap por el entorno de ejecución de JavaScript.
 */

int main() {
    dataset_inicializar();
    return 0;
}
