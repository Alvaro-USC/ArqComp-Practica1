/*
 * acp1_directo.c - Medición de latencia de memoria con acceso DIRECTO
 * Experimento adicional: comparar acceso directo A[i*D] vs. indirecto A[ind[i]]
 *
 * Diferencia clave respecto a acp1.c:
 *   - Se elimina el vector de índices ind[]
 *   - Los elementos se acceden directamente como A[i * D]
 *   - Efecto esperado: menor presión sobre caché (ind[] ya no ocupa líneas de caché)
 *     y patrón de acceso más predecible → el prefetcher puede actuar mejor
 *   - En la zona L1 la diferencia será mínima; en RAM puede ser notable
 *
 * Compilar: gcc acp1_directo.c -o acp1_directo -O0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "counter.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <D> <L>\n", argv[0]);
        return 1;
    }

    int D = atoi(argv[1]);
    int L = atoi(argv[2]);

    const int CLS = 64;   /* Tamaño de línea de caché en bytes */
    const int REPS = 10;  /* Repeticiones internas para estabilizar medición */

    /*
     * Cálculo de R: igual que en acp1.c
     * R = (L * 64 bytes) / (D * 8 bytes/double)
     */
    long long R = (long long)L * CLS / (D * sizeof(double));
    if (R <= 0) R = 1;

    /* Tamaño total del vector: el último índice accedido es (R-1)*D */
    long long N = (R - 1) * D + 1;

    /*
     * Reserva de memoria alineada a 64 bytes.
     * NOTA: no reservamos ind[] porque el acceso es DIRECTO (A[i*D])
     * Esto libera capacidad de caché que en acp1.c era consumida por ind[]
     */
    double *A = (double*)aligned_alloc(CLS, N * sizeof(double));
    double acum[REPS];

    if (!A) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        return 1;
    }

    /*
     * Inicialización de datos:
     * Usamos acceso directo A[i*D] también en la inicialización para
     * garantizar que las mismas posiciones que se leerán están inicializadas.
     * Valores aleatorios en [1,2) con signo aleatorio para evitar overflow.
     */
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        double val = 1.0 + (double)rand() / RAND_MAX;  /* Valor en [1, 2) */
        if (rand() % 2) val = -val;                     /* Signo aleatorio */
        A[i * D] = val;
    }

    /* --- SECCIÓN CRÍTICA DE MEDICIÓN --- */
    start_counter();

    for (int k = 0; k < REPS; k++) {
        double suma = 0.0;
        /*
         * Acceso DIRECTO: A[i * D]
         * Sin vector de índices intermedio.
         * El compilador (con -O0) calculará i*D en cada iteración sin optimizar.
         */
        for (long long i = 0; i < R; i++) {
            suma += A[i * D];
        }
        acum[k] = suma;
    }

    double ciclos_totales = get_counter();
    /* ------------------------------------ */

    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    /* Mismo formato de salida que acp1.c para análisis conjunto */
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    /* Uso del resultado para evitar que el compilador elimine el bucle */
    if (acum[0] == -1.0) printf("Check: %f", acum[0]);

    free(A);
    return 0;
}