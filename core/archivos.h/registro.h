#ifndef REGISTRO_H
#define REGISTRO_H

// ==========================================================
// ENTIDAD CENTRAL DEL PROYECTO
// Equivalente a commit.h en ProyectoBisect.
// TODOS los modulos dependen de esta estructura, no se toca
// sin avisar al resto del equipo.
// ==========================================================

#define MAX_REGISTROS 5000

// Cantidad de variables climaticas usadas como vector de entrada
// al algoritmo Fuzzy C-Means: sst, presion, humedad, viento, cizalladura
#define NUM_VARIABLES 5

// Indices de cada variable dentro de un vector double[NUM_VARIABLES]
#define VAR_SST         0
#define VAR_PRESION     1
#define VAR_HUMEDAD     2
#define VAR_VIENTO      3
#define VAR_CIZALLADURA 4

// Representa una observacion climatica puntual:
// un dia, en una zona del Caribe.
typedef struct {
    int    anio;
    int    mes;
    int    dia;
    double latitud;
    double longitud;

    double sst;          // Temperatura superficial del mar (C)
    double presion;       // Presion atmosferica a nivel del mar (hPa)
    double humedad;         // Humedad relativa nivel medio (%)
    double viento;           // Viento sostenido (km/h)
    double cizalladura;       // Wind shear (m/s)

    // Etiqueta real: 1 si en esa fecha/zona hubo ciclon segun HURDAT2,
    // 0 si no. SOLO se usa para validar en el Modulo 5 (evaluador).
    // NUNCA se le pasa al algoritmo FCM durante el entrenamiento/prediccion,
    // porque el objetivo es que el sistema lo "adivine" solo.
    int    hubo_ciclon;
} RegistroClimatico;

/**
 * @brief Convierte un RegistroClimatico en un vector de características continuo.
 *
 * Esta función toma una estructura de observación climática (RegistroClimatico)
 * y extrae las variables predictoras clave, depositándolas secuencialmente en un 
 * arreglo. Este arreglo (vector_out) representa el formato de entrada numérico 
 * estándar requerido por el núcleo del algoritmo Fuzzy C-Means (Módulo 3).
 *
 * @param r Puntero constante a la estructura de registro climático que contiene la observación.
 * @param vector_out Arreglo de salida (double) pre-asignado con tamaño de al menos NUM_VARIABLES.
 */
void registro_a_vector(const RegistroClimatico* r, double vector_out[NUM_VARIABLES]);

#endif // REGISTRO_H
