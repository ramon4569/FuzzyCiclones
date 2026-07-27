#include <stdio.h>
#include "core/archivos.h/modulo1_dataset.h"

// Punto de entrada — solo inicializacion.
// Toda la logica real se invoca desde api_bridge.c via Emscripten
// (ver Modulo 6), igual que en ProyectoBisect.

int main() {
    dataset_inicializar();
    return 0;
}
