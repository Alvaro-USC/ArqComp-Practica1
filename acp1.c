#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "counter.h" // Asegúrate de que counter.h esté en la misma carpeta

int main(int argc, char *argv[]) {
    // 1. Verificación de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <D> <L>\n", argv[0]);
        return 1;
    }

    int D = atoi(argv[1]); // Stride
    int L = atoi(argv[2]); // Número de líneas de caché a tocar

    // 2. Constantes y configuración
    const int CLS = 64;          // Tamaño de línea de caché (bytes)
    const int REPS = 10;         // Repeticiones internas para estabilidad
    
    // Cálculo de R: Número de elementos a sumar
    // L líneas * 64 bytes/línea = Bytes totales requeridos
    // Bytes totales / (D saltos * 8 bytes/double) = Elementos necesarios
    // Simplificado: R = (L * 8) / D
    long long R = (long long)L * CLS / (D * sizeof(double));

    // Cálculo de N: Tamaño total del vector para soportar el salto D
    long long N = (R - 1) * D + 1;

    if (R <= 0) {
        fprintf(stderr, "Error: Parámetros inválidos (R=%lld)\n", R);
        return 1;
    }

    // 3. Reserva de memoria
    // Usamos aligned_alloc para que el vector empiece al inicio de una línea caché
    double *A = (double*)aligned_alloc(CLS, N * sizeof(double));
    int *ind = (int*)malloc(R * sizeof(int));
    double acum[REPS]; // Para guardar resultados y evitar optimización

    if (!A || !ind) {
        fprintf(stderr, "Error de memoria (N=%lld)\n", N);
        return 1;
    }

    // 4. Inicialización
    // Inicializar vector de índices para acceso indirecto
    for (long long i = 0; i < R; i++) {
        ind[i] = i * D;
    }

    // Inicializar datos con valores aleatorios
    srand(time(NULL));
    for (long long i = 0; i < R; i++) {
        A[ind[i]] = (double)rand() / RAND_MAX;
    }

    // 5. Medición
    // Calentamiento y estimación de frecuencia (opcional, pero útil para logs)
    // mhz(0, 1); 
    
    start_counter();
    
    // Bucle de repeticiones (medimos todo el bloque)
    for (int k = 0; k < REPS; k++) {
        double suma = 0.0;
        // Bucle crítico de suma con acceso indirecto
        for (long long i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        acum[k] = suma; // Evitar que el compilador elimine el bucle
    }

    double ciclos_totales = get_counter();

    // 6. Resultados
    double ciclos_por_acceso = ciclos_totales / ((double)R * REPS);
    
    // Salida formateada para fácil parseo (grep "Ciclos:")
    printf("D=%d\tL=%d\tR=%lld\tCiclos:%.4f\n", D, L, R, ciclos_por_acceso);

    // Imprimir un resultado aleatorio para asegurar uso de datos
    // (Esto es un truco extra anti-optimización)
    if (acum[0] == -1.0) printf("Ignore this: %f", acum[0]);

    free(A);
    free(ind);
    return 0;
}
