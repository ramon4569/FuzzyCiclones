#include "../archivos.h/registro.h"

/**
 * @brief Extrae los atributos del registro climático en un arreglo unidimensional.
 *
 * Se encarga de mapear cada uno de los campos de la estructura RegistroClimatico
 * hacia el índice correspondiente dentro del arreglo de salida (vector_out), 
 * garantizando el orden establecido por las macros VAR_*.
 *
 * @param r Puntero de solo lectura al registro climático de origen.
 * @param vector_out Arreglo destino donde se copiarán las variables continuas.
 */
void registro_a_vector(const RegistroClimatico* r, double vector_out[NUM_VARIABLES]) {
    // Asignar el valor de la temperatura superficial del mar (SST) al índice 0
    vector_out[VAR_SST] = r->sst;
    
    // Asignar la medición de la presión atmosférica al índice 1
    vector_out[VAR_PRESION] = r->presion;
    
    // Asignar la humedad relativa medida al índice 2
    vector_out[VAR_HUMEDAD] = r->humedad;
    
    // Asignar la velocidad del viento sostenido al índice 3
    vector_out[VAR_VIENTO] = r->viento;
    
    // Asignar el valor de la cizalladura del viento (wind shear) al índice 4
    vector_out[VAR_CIZALLADURA] = r->cizalladura;
}
