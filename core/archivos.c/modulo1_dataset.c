#include <string.h>
#include "../archivos.h/modulo1_dataset.h"

// Dataset almacenado completamente en memoria.
// Se utiliza un arreglo estático y un contador para evitar
// asignación dinámica de memoria.

static RegistroClimatico g_dataset[MAX_REGISTROS];
static int g_total = 0;


// Obtiene el valor de una variable climática según el índice
// definido por las constantes VAR_*.
// Devuelve 0.0 si el índice recibido no es válido.
static double variable_de(const RegistroClimatico* r, int indice_variable) {
    switch (indice_variable) {
    case VAR_SST:         return r->sst;
    case VAR_PRESION:     return r->presion;
    case VAR_HUMEDAD:     return r->humedad;
    case VAR_VIENTO:      return r->viento;
    case VAR_CIZALLADURA: return r->cizalladura;
    default:              return 0.0;
    }
}

// Verifica si un índice pertenece al rango válido del dataset.
static int indice_valido(int index) {
    return index >= 0 && index < g_total;
}

// Reinicia el dataset dejando el contador en cero.
// No es necesario limpiar el arreglo porque los registros
// quedan inaccesibles mientras g_total sea 0.
void dataset_inicializar(void) {

    g_total = 0;
}

// Elimina todos los registros del dataset.
void dataset_limpiar(void) {
    g_total = 0;
}

// Inserta un nuevo registro en el dataset.
// Devuelve el índice donde fue almacenado o -1 si el
// arreglo alcanzó su capacidad máxima.
int dataset_insertar(RegistroClimatico r) {

    if (g_total >= MAX_REGISTROS) {
    return -1;
}
    g_dataset[g_total] = r;
    g_total++;
    return g_total - 1;
}



// Obtiene una copia del registro solicitado.
// Si el índice es inválido devuelve un registro vacío.
RegistroClimatico dataset_get(int index) {

    if (!indice_valido(index)) {
        RegistroClimatico vacio;
        memset(&vacio, 0, sizeof(vacio));
    return vacio;
}
    return g_dataset[index];
}

// Devuelve la cantidad de registros almacenados.
int dataset_total(void) {
    return g_total;
}

// Calcula el promedio de una variable climática
// recorriendo todos los registros almacenados.
double dataset_promedio_variable(int indice_variable) {

    if (g_total == 0) {
    return 0.0;
}
    double suma = 0.0;
    for (int i = 0; i < g_total; i++) {
        suma += variable_de(&g_dataset[i], indice_variable);
    }
    return suma / g_total;
}

// Calcula el valor mínimo y máximo de cada variable
// climática presente en el dataset.
void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]) {

    // Si el dataset está vacío, todos los valores
   // mínimos y máximos se inicializan en cero.
    if (g_total == 0) {
        for (int v = 0; v < NUM_VARIABLES; v++) {
            min_out[v] = 0.0;
            max_out[v] = 0.0;
        }
        return;
    }

    // Inicializa los mínimos y máximos con el primer registro.
    for (int v = 0; v < NUM_VARIABLES; v++) {
        double val0 = variable_de(&g_dataset[0], v);
        min_out[v] = val0;
        max_out[v] = val0;
    }

    // Recorre el resto de los registros actualizando
    // los valores mínimos y máximos encontrados.
    for (int i = 1; i < g_total; i++) {
        for (int v = 0; v < NUM_VARIABLES; v++) {
            double val = variable_de(&g_dataset[i], v);
            if (val < min_out[v]) min_out[v] = val;
            if (val > max_out[v]) max_out[v] = val;
        }
    }
}

// Copia al arreglo destino los registros cuyo año se
// encuentre dentro del rango especificado.
// Devuelve la cantidad de registros copiados.
int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
                              RegistroClimatico* destino, int max_destino) {

    int copiados = 0;
    for (int i = 0; i < g_total && copiados < max_destino; i++) {
        if (g_dataset[i].anio >= anio_inicio && g_dataset[i].anio <= anio_fin) {
            destino[copiados] = g_dataset[i];
            copiados++;
        }
    }
    return copiados;
}

// Permite actualizar el campo hubo_ciclon de un registro
// ya almacenado en el dataset.
// Devuelve 1 si la operación fue exitosa o 0 si el índice
// recibido no es válido.
int dataset_marcar_ciclon(int index, int valor) {
    if (!indice_valido(index)) {
    return 0;
}
    g_dataset[index].hubo_ciclon = valor;
    return 1;
}
