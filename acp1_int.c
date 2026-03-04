/*
 * acp1_int.c - Medición de latencia de memoria con array de tipo INT
 * Experimento adicional: comparar comportamiento vs. tipo Double
 *
 * Diferencia clave respecto a acp1.c:
 *   - El array A[] es de tipo int (4 bytes) en lugar de double (8 bytes)
 *   - Caben el DOBLE de elementos por línea de caché (64B/4B = 16 int vs 8 double)
 *   - Para el mismo L y D, R es el doble → más datos con el mismo número de líneas
 *   - El acumulador S[] también es int para consistencia de tipo
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
    const int REPS = 10;  /* Repeticiones de la reducción */

    /*
     * Cálculo de R con tipo int (4 bytes):
     * R = (L × 64 bytes) / (D × 4 bytes/int)
     * Para el mismo L y D, R es el DOBLE que en el caso double.
     */
    long long R = (long long)L * CLS / (D * sizeof(int));
    if (R <= 0) R = 1;

    /* Índice máximo accedido: (R-1)*D */
    long long N = (R - 1) * D + 1;

    /* Reserva alineada a 64 bytes (inicio de línea de caché) */
    int *A   = (int*)aligned_alloc(CLS, N * sizeof(int));
    int *ind = (int*)malloc(R * sizeof(int));
    /* S[] almacena el resultado de cada repetición (requerido por el enunciado) */
    int  S[REPS];

    if (!A || !ind) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        return 1;
    }

    /* Inicialización del vector de índices: ind[i] = i * D */
    for (long long i = 0; i < R; i++) ind[i] = (int)(i * D);

    /*
     * Inicialización de A[] con valores en [-32767, -1] ∪ [1, 32767].
     * El rango acotado evita desbordamiento en la suma int
     * (int: rango [-2^31, 2^31-1]; con R hasta ~6M y valores hasta 32767
     *  la suma máxima ~2×10^11 desborda — usamos valores pequeños ±1..100
     *  para estar seguros en todos los casos de L).
     */
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        int val = 1 + rand() % 100; /* valor en [1, 100] */
        if (rand() % 2) val = -val; /* signo aleatorio */
        A[ind[i]] = val;
    }

    /* ── SECCIÓN DE MEDICIÓN ── */
    start_counter();

    for (int k = 0; k < REPS; k++) {
        int suma = 0;
        /* Reducción de suma con acceso indirecto: A[ind[i]] */
        for (long long i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        S[k] = suma; /* guardar resultado de cada repetición */
    }

    double ciclos_totales = get_counter();
    /* ── FIN DE MEDICIÓN ── */

    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    /* Imprimir los 10 resultados de S[] (requerido por el enunciado) */
    printf("Resultados S[]:");
    for (int k = 0; k < REPS; k++) printf(" %d", S[k]);
    printf("\n");

    /* Línea de métricas para el script de análisis Python */
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    free(A);
    free(ind);
    return 0;
}