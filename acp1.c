/*
 * acp1.c - Medición de latencia de memoria
 * Compilar: gcc acp1.c -o acp1 -O0
 *
 * Parámetros:
 *   D: stride (distancia entre elementos accedidos, en número de doubles)
 *   L: número de líneas de caché distintas que se referencian
 *
 * El programa calcula R (elementos a sumar) a partir de L y D, reserva
 * memoria alineada a 64 bytes, realiza 10 repeticiones de una reducción
 * de suma con acceso indirecto A[ind[i]], y mide los ciclos totales.
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
    const int REPS = 10;  /* Repeticiones de la reducción (media y anti-optimización) */

    /*
     * Cálculo de R: número de elementos a sumar.
     * Cada acceso A[i*D] toca una línea de caché diferente cuando D >= CLS/sizeof(double).
     * R = (L líneas × 64 bytes/línea) / (D × 8 bytes/double)
     */
    long long R = (long long)L * CLS / (D * sizeof(double));
    if (R <= 0) R = 1;

    /* Índice máximo accedido: (R-1)*D; tamaño mínimo del vector */
    long long N = (R - 1) * D + 1;

    /* Reserva alineada a 64 bytes para que A[0] coincida con inicio de línea de caché */
    double *A   = (double*)aligned_alloc(CLS, N * sizeof(double));
    int    *ind = (int*)malloc(R * sizeof(int));
    /* S[] almacena el resultado de cada repetición (requerido por el enunciado) */
    double  S[REPS];

    if (!A || !ind) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        return 1;
    }

    /* Inicialización del vector de índices: ind[i] = i * D */
    for (long long i = 0; i < R; i++) ind[i] = (int)(i * D);

    /*
     * Inicialización de A[] con valores aleatorios en [-2, -1) ∪ [1, 2).
     * El rango [1,2) con signo aleatorio evita desbordamiento en la suma Double
     * y garantiza que los datos leídos sean distintos de cero (calentamiento real).
     */
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        double val = 1.0 + (double)rand() / RAND_MAX; /* valor en [1, 2) */
        if (rand() % 2) val = -val;                   /* signo aleatorio */
        A[ind[i]] = val;
    }

    /* ── SECCIÓN DE MEDICIÓN ── */
    start_counter();

    for (int k = 0; k < REPS; k++) {
        double suma = 0.0;
        /* Reducción de suma con acceso indirecto: A[ind[i]] */
        for (long long i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        S[k] = suma; /* guardar resultado de cada repetición */
    }

    double ciclos_totales = get_counter();
    /* ── FIN DE MEDICIÓN ── */

    /* Ciclos medios por acceso: total / (R accesos × REPS repeticiones) */
    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    /* Imprimir los 10 resultados de S[] (requerido por el enunciado) */
    printf("Resultados S[]:");
    for (int k = 0; k < REPS; k++) printf(" %.6f", S[k]);
    printf("\n");

    /* Línea de métricas para el script de análisis Python */
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    free(A);
    free(ind);
    return 0;
}