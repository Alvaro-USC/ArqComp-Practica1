/*
 * acp1_directo.c - Medición de latencia de memoria con acceso DIRECTO
 * Experimento adicional: comparar acceso directo A[i*D] vs. indirecto A[ind[i]]
 *
 * Diferencia clave respecto a acp1.c:
 *   - Se elimina el vector de índices ind[]
 *   - Los elementos se acceden directamente como A[i * D]
 *   - Al no existir ind[], toda la caché se dedica exclusivamente a A[],
 *     y el patrón de acceso es más predecible → el prefetcher puede actuar mejor
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
    const int REPS = 10;  /* Repeticiones de la reducción */

    /*
     * Cálculo de R: igual que en acp1.c
     * R = (L × 64 bytes) / (D × 8 bytes/double)
     */
    long long R = (long long)L * CLS / (D * sizeof(double));
    if (R <= 0) R = 1;

    /* Índice máximo accedido: (R-1)*D */
    long long N = (R - 1) * D + 1;

    /*
     * Reserva alineada a 64 bytes.
     * No se reserva ind[] porque el acceso es DIRECTO (A[i*D]),
     * lo que libera capacidad de caché respecto a acp1.c.
     */
    double *A = (double*)aligned_alloc(CLS, N * sizeof(double));
    /* S[] almacena el resultado de cada repetición (requerido por el enunciado) */
    double  S[REPS];

    if (!A) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        return 1;
    }

    /*
     * Inicialización de A[] con valores en [-2,-1) ∪ [1,2) con signo aleatorio.
     * Evita desbordamiento en la suma Double y garantiza calentamiento real de caché.
     */
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        double val = 1.0 + (double)rand() / RAND_MAX; /* valor en [1, 2) */
        if (rand() % 2) val = -val;                    /* signo aleatorio */
        A[i * D] = val;
    }

    /* ── SECCIÓN DE MEDICIÓN ── */
    start_counter();

    for (int k = 0; k < REPS; k++) {
        double suma = 0.0;
        /* Acceso DIRECTO: A[i * D], sin vector de índices intermedio */
        for (long long i = 0; i < R; i++) {
            suma += A[i * D];
        }
        S[k] = suma; /* guardar resultado de cada repetición */
    }

    double ciclos_totales = get_counter();
    /* ── FIN DE MEDICIÓN ── */

    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    /* Imprimir los 10 resultados de S[] (requerido por el enunciado) */
    printf("Resultados S[]:");
    for (int k = 0; k < REPS; k++) printf(" %.6f", S[k]);
    printf("\n");

    /* Línea de métricas para el script de análisis Python */
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    free(A);
    return 0;
}