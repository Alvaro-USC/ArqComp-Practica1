/*
 * acp1.c - Medición de latencia de memoria
 * Compilar: gcc acp1.c -o acp1 -O0
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "counter.h" // Tu librería de contador

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <D> <L>\n", argv[0]);
        return 1;
    }

    int D = atoi(argv[1]);
    int L = atoi(argv[2]);

    const int CLS = 64;          // Cache Line Size
    const int REPS = 10;         // Repeticiones internas para estabilizar medición

    // Cálculo preciso de R (elementos)
    // R = (Líneas * 64 bytes) / (Stride * 8 bytes/double)
    long long R = (long long)L * CLS / (D * sizeof(double));
    if (R <= 0) R = 1;

    // Cálculo de N (Tamaño del vector)
    long long N = (R - 1) * D + 1;

    // Alineación a 64 bytes (CRÍTICO para precisión en L1)
    double *A = (double*)aligned_alloc(CLS, N * sizeof(double));
    int *ind = (int*)malloc(R * sizeof(int));
    double acum[REPS]; 

    if (!A || !ind) return 1;

    // Inicialización (Solo lo necesario para ahorrar tiempo)
    for (long long i = 0; i < R; i++) ind[i] = i * D;
    
    srand(time(NULL));
    for (long long i = 0; i < R; i++) A[ind[i]] = (double)rand() / RAND_MAX;

    // --- SECCIÓN CRÍTICA DE MEDICIÓN ---
    start_counter();
    
    for (int k = 0; k < REPS; k++) {
        double suma = 0.0;
        // Bucle desenrollado implícitamente por lógica simple
        for (long long i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        acum[k] = suma;
    }

    double ciclos_totales = get_counter();
    // -----------------------------------

    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);

    // Salida en formato clave=valor para fácil parsing en Python
    // Imprimimos todo para tener trazabilidad
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.6f\n", D, L, R, ciclos_por_acceso);

    // Evitar optimización del compilador usando el resultado
    if (acum[0] == -1.0) printf("Check: %f", acum[0]);

    free(A);
    free(ind);
    return 0;
}
