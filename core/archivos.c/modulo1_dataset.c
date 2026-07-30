#include <string.h>
#include <stdio.h>
#include "../archivos.h/modulo1_dataset.h"

// Dataset almacenado completamente en memoria.
// Se utiliza un arreglo estático y un contador para evitar
// asignación dinámica de memoria.

static RegistroClimatico g_dataset[MAX_REGISTROS];
static int g_total = 0;


/**
 * @brief Rutina auxiliar de despacho estático para abstraer el acceso a variables.
 * 
 * Implementa una función hash trivial vía sentencias 'switch' para mapear el
 * discriminador numérico de la característica a la proyección de la estructura.
 * 
 * @param r Puntero de lectura a la observación.
 * @param indice_variable Identificador escalar en el rango de [0, NUM_VARIABLES-1].
 * @return double Magnitud escalar de la medición respectiva.
 */
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

/**
 * @brief Valida el invariante de índice frente a la dimensionalidad real del dataset.
 * 
 * @param index Índice candidato a inspeccionar.
 * @return int Resultado de la validación: verdadero (!0) si es legal, falso (0) si no.
 */
static int indice_valido(int index) {
    return index >= 0 && index < g_total;
}

/**
 * @brief Rutina de arranque (bootstrap) o reinicio del contenedor global.
 * 
 * Se optimiza omitiendo cerado del búfer de memoria (memzero), ya que
 * el acceso queda encapsulado y regido por el contador lógico 'g_total'.
 */
void dataset_inicializar(void) {
    // Reset del apuntador de inserción lógica
    g_total = 0;
}

/**
 * @brief Interfaz pública para forzar un reset lógico asíncrono.
 */
void dataset_limpiar(void) {
    // Trunca el conjunto haciendo inaccesible la data heredada (borrado lógico rápido O(1))
    g_total = 0;
}

/**
 * @brief Mecanismo de persistencia secuencial (Append) en memoria de trabajo.
 * 
 * Realiza un control de fronteras (bounds checking) previo para impedir
 * una violación de memoria por saturación del array pre-asentado.
 */
int dataset_insertar(RegistroClimatico r) {
    // Retorno anticipado de código de fallo ante un intento de escritura sobre límite
    if (g_total >= MAX_REGISTROS) {
        return -1;
    }
    
    // Transferencia por copia de valor bloque-a-bloque de la estructura hacia el buffer
    g_dataset[g_total] = r;
    g_total++;
    
    // Retorna el ID ordinal base cero de la última inserción
    return g_total - 1;
}



/**
 * @brief Función extractora (getter) con enmascaramiento ante errores de puntero nulo.
 */
RegistroClimatico dataset_get(int index) {
    // Salvaguarda: Retorna una estructura predeterminada y purgada para fallos controlados
    if (!indice_valido(index)) {
        RegistroClimatico vacio;
        memset(&vacio, 0, sizeof(vacio)); // Cero semántico sobre todos los bits
        return vacio;
    }
    // Extracción atómica en memoria plana
    return g_dataset[index];
}

/**
 * @brief Expone el tamaño subyacente consumido en el heap estático.
 */
int dataset_total(void) {
    // Lectura pasiva del estado mutado
    return g_total;
}

/**
 * @brief Implementa el estimador insesgado de la media muestral poblacional.
 */
double dataset_promedio_variable(int indice_variable) {
    // Casuística frontera: Evitar una división entre cero matemática
    if (g_total == 0) {
        return 0.0;
    }
    
    double suma = 0.0;
    // Iteración O(N) acumulando sumas parciales flotantes
    for (int i = 0; i < g_total; i++) {
        // Aprovecha la heurística hash de 'variable_de' para aislar la componente
        suma += variable_de(&g_dataset[i], indice_variable);
    }
    // Promedio matemático: normalización sobre el cardinal
    return suma / g_total;
}

/**
 * @brief Cálculo de cotas extremales (rango [min, max]) mediante escaneo heurístico local.
 */
void dataset_min_max(double min_out[NUM_VARIABLES], double max_out[NUM_VARIABLES]) {
    // Si la matriz de datos adolece de registros, se reportan ceros (condición subnormal)
    if (g_total == 0) {
        for (int v = 0; v < NUM_VARIABLES; v++) {
            min_out[v] = 0.0;
            max_out[v] = 0.0;
        }
        return;
    }

    // Fase de arranque: siembra las cotas asumiendo el primer vector como candidato óptimo
    for (int v = 0; v < NUM_VARIABLES; v++) {
        double val0 = variable_de(&g_dataset[0], v);
        min_out[v] = val0;
        max_out[v] = val0;
    }

    // Fase de escrutinio exhaustivo sobre el dominio restante
    // Complejidad algorítmica general: O(N * D) donde D = NUM_VARIABLES
    for (int i = 1; i < g_total; i++) {
        // Bucle anidado ortogonal iterando la dimensionalidad del vector
        for (int v = 0; v < NUM_VARIABLES; v++) {
            double val = variable_de(&g_dataset[i], v);
            
            // Relajación cruzada del óptimo local por cada eje
            if (val < min_out[v]) min_out[v] = val;
            if (val > max_out[v]) max_out[v] = val;
        }
    }
}

/**
 * @brief Implementa una operación de query y volcado (dump) usando un predicado cronológico.
 */
int dataset_filtrar_por_anio(int anio_inicio, int anio_fin,
                              RegistroClimatico* destino, int max_destino) {
    int copiados = 0;
    
    // Ciclo de evaluación simultánea con condición de guarda (short-circuit limit match)
    for (int i = 0; i < g_total && copiados < max_destino; i++) {
        // Predicado de filtrado inclusivo sobre el atributo escalar 'anio'
        if (g_dataset[i].anio >= anio_inicio && g_dataset[i].anio <= anio_fin) {
            // Migración asíncrona de datos desde el heap maestro a la subrutina cliente
            destino[copiados] = g_dataset[i];
            copiados++;
        }
    }
    // Conteo efectivo de transacciones exitosas al cliente
    return copiados;
}

/**
 * @brief Setter mutador enfocado a la sobrescritura del Ground Truth.
 */
int dataset_marcar_ciclon(int index, int valor) {
    // Mecanismo de fallo rápido si el usuario inyecta punteros a regiones espurias
    if (!indice_valido(index)) {
        return 0; // Código subyacente falseable
    }
    
    // Modificación de la variable de clase categórica (target/label)
    g_dataset[index].hubo_ciclon = valor;
    
    // Confirmación binaria positiva
    return 1;
}
