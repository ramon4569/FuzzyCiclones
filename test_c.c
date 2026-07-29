#include <stdio.h>
#include <stdlib.h>
#include "core/archivos.h/registro.h"
#include "core/archivos.h/modulo2_importador.h"
#include "core/archivos.h/modulo1_dataset.h"
#include "core/archivos.h/modulo3_fcm_core.h"
#include "core/archivos.h/modulo4_entrenamiento.h"

int main() {
    FILE *f = fopen("data/entrenamiento_2015_2016_real.csv", "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;

    int importados = importar_archivo(string);
    printf("Importados: %d\n", importados);

    RegistroClimatico r = dataset_get(1);
    printf("Registro 1: sst=%f, presion=%f\n", r.sst, r.presion);

    int iter = entrenamiento_entrenar(2015, 2016, 3, 2.0, 100, 0.00001);
    printf("Iteraciones: %d\n", iter);

    double c[NUM_VARIABLES];
    fcm_obtener_centroide(0, c);
    printf("Centroide 0: sst=%f, presion=%f\n", c[VAR_SST], c[VAR_PRESION]);

    return 0;
}
