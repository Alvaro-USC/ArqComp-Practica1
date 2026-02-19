/*
 * acp1_int.c - Medición de latencia de memoria con array de tipo INT
 * Experimento adicional: comparar comportamiento vs. tipo Double
 *
 * Diferencia clave respecto a acp1.c:
 *   - El array A[] es de tipo int (4 bytes) en lugar de double (8 bytes)
 *   - Caben el DOBLE de elementos por línea de caché (64B / 4B = 16 ints vs 8 doubles)
 *   - Para el mismo L, R es el doble → se accede a más datos con el mismo número de líneas
 *   - El acumulador de suma también es int para consistencia
 *
 * Compilar: gcc acp1_int.c -o acp1_int -O0
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
     * Cálculo de R (número de elementos a sumar):
     * Ahora el elemento es int (4 bytes), no double (8 bytes)
     * R = (L * 64 bytes) / (D * 4 bytes/int)
     * Para el mismo L y D, R es el DOBLE que en el caso double
     */
    long long R = (long long)L * CLS / (D * sizeof(int));
    if (R <= 0) R = 1;

    /* Tamaño total del vector: el último índice accedido es (R-1)*D */
    long long N = (R - 1) * D + 1;

    /* Reserva de memoria alineada a 64 bytes (inicio de línea de caché) */
    int *A = (int*)aligned_alloc(CLS, N * sizeof(int));
    int *ind = (int*)malloc(R * sizeof(int));
    int acum[REPS];  /* Acumulador de resultados (tipo int para consistencia) */

    if (!A || !ind) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        return 1;
    }

    /* Inicialización del vector de índices: ind[i] = i * D */
    for (long long i = 0; i < R; i++) ind[i] = i * D;

    /*
     * Inicialización de datos con valores aleatorios acotados en [-2^15, 2^15)
     * para evitar desbordamiento en la suma (int: rango [-2^31, 2^31-1])
     */
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        int val = rand() % 32768;           /* Valor en [0, 32767] */
        if (rand() % 2) val = -val;         /* Signo aleatorio */
        A[ind[i]] = val;
    }

    /* --- SECCIÓN CRÍTICA DE MEDICIÓN --- */
    start_counter();

    for (int k = 0; k < REPS; k++) {
        int suma = 0;
        /* Reducción de suma con acceso indirecto: A[ind[i]] */
        for (long long i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        acum[k] = suma;
    }

    double ciclos_totales = get_counter();
    /* ------------------------------------ */

    /* Ciclos medios por acceso sobre las 10 repeticiones × R accesos */
    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    /* Salida en mismo formato que acp1.c para facilitar el análisis conjunto */
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    /* Uso del resultado para evitar que el compilador elimine el bucle */
    if (acum[0] == -999999) printf("Check: %d", acum[0]);

    free(A);
    free(ind);
    return 0;
}