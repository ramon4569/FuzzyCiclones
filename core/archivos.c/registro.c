#include "../archivos.h/registro.h"

void registro_a_vector(const RegistroClimatico* r, double vector_out[NUM_VARIABLES]) {
    vector_out[VAR_SST] = r->sst;
    vector_out[VAR_PRESION] = r->presion;
    vector_out[VAR_HUMEDAD] = r->humedad;
    vector_out[VAR_VIENTO] = r->viento;
    vector_out[VAR_CIZALLADURA] = r->cizalladura;
}
