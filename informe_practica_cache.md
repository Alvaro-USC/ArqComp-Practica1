# Informe de Práctica: Jerarquía de Memoria y Comportamiento de Memoria Caché

**Asignatura**: Arquitectura de Computadores  
**Práctica**: Estudio del efecto de la localidad de los accesos a memoria  
**Plataforma**: FinisTerrae III (CESGA)  

---

## 1. Introducción

### 1.1 Objetivos

El objetivo principal de esta práctica es caracterizar experimentalmente el coste temporal de acceso a memoria en un microprocesador moderno, analizando cómo la jerarquía de memoria caché afecta al rendimiento de programas según el patrón de acceso a los datos.

Específicamente, se busca:

- Medir el coste en ciclos por acceso a memoria variando diferentes parámetros
- Observar el efecto de la localidad espacial y temporal
- Analizar el impacto del prefetching (precarga) del procesador
- Relacionar los resultados con la arquitectura de caché (L1, L2)

### 1.2 Conceptos Fundamentales

**Jerarquía de Memoria**: Los procesadores modernos utilizan una jerarquía de memorias con diferentes velocidades y capacidades:
- Registros (más rápidos, menor capacidad)
- Caché L1 (decenas de KB, ~4 ciclos)
- Caché L2 (cientos de KB, ~12 ciclos)
- Caché L3 (varios MB, ~40 ciclos)
- Memoria principal RAM (GB, ~200 ciclos)

**Localidad Temporal**: Tendencia a acceder repetidamente a los mismos datos en un corto periodo de tiempo.

**Localidad Espacial**: Tendencia a acceder a datos cercanos en memoria tras acceder a una posición específica.

**Prefetching**: Mecanismo del procesador que anticipa accesos futuros y carga datos en caché antes de que sean solicitados.

---

## 2. Metodología

### 2.1 Descripción del Experimento

El experimento consiste en realizar una operación de reducción (suma) sobre elementos de un vector de tipo `double`, con un patrón de acceso específico:

```
Suma = A[0] + A[D] + A[2D] + A[3D] + ... + A[(R-1)*D]
```

Donde:
- **R**: Número de elementos a sumar
- **D**: Distancia (stride) entre elementos consecutivos
- **A[]**: Vector de datos tipo `double`

**Características clave**:
- El acceso a los elementos se hace de forma **indirecta** mediante un vector de índices `ind[]`
- Se repite la reducción 10 veces para evitar optimizaciones del compilador
- Se mide el tiempo total en ciclos de CPU

### 2.2 Parámetros del Experimento

#### Parámetro L (Líneas Caché)

El número de líneas caché diferentes que se acceden determina si los datos caben en L1, L2 o requieren acceso a memoria principal. Los valores de L son:

- `0.5 × S1`: La mitad de la caché L1
- `1.5 × S1`: Excede L1, usa L2
- `0.5 × S2`: La mitad de la caché L2
- `0.75 × S2`: Tres cuartos de L2
- `2 × S2`: El doble de L2 (no cabe)
- `4 × S2`: Cuatro veces L2
- `8 × S2`: Ocho veces L2

Donde:
- **S1**: Número de líneas caché en L1 datos
- **S2**: Número de líneas caché en L2

#### Parámetro D (Stride)

Se utilizan 5 valores potencia de 2 para D, seleccionados para estudiar diferentes escenarios de localidad:

- **D = 1**: Accesos consecutivos (localidad espacial máxima)
- **D = 8**: Un `double` por línea caché (64 bytes / 8 bytes = 8)
- **D = 16**: Salta una línea caché completa
- **D = 64**: Localidad espacial muy reducida
- **D = 128**: Localidad espacial mínima

### 2.3 Cálculo de Parámetros

#### Determinación de S1 y S2

Para el FinisTerrae III con procesadores **Intel Xeon Platinum 8352Y**:

**Arquitectura del procesador:**
- 2x Intel Xeon Platinum 8352Y (2 sockets)
- 32 cores por socket = 64 cores totales
- **Esta práctica se ejecuta en un solo core**
- Tamaño de línea caché (CLS): **64 bytes**

**Jerarquía de caché por core (privada):**
- **L1i (instrucciones)**: 32 KB por core (8-way set associative)
- **L1d (datos)**: 48 KB por core (12-way set associative)
- **L2 (unificada)**: 1280 KB = 1.25 MB por core (20-way set associative)
- **L3 (compartida)**: 48 MB entre todos los cores (12-way set associative)

**Latencias de acceso:**
- L1: ~5 ciclos
- L2: ~14 ciclos

**Cálculo de S1 y S2:**

Dado que trabajamos con **un solo core**, utilizamos sus cachés privadas:

- **S1** (líneas en L1 datos):
  ```
  S1 = 48 KB / 64 bytes = 49152 / 64 = 768 líneas
  ```

- **S2** (líneas en L2):
  ```
  S2 = 1280 KB / 64 bytes = 1310720 / 64 = 20480 líneas
  ```

**Nota importante**: 
- La L1 y L2 son **privadas** de cada core, no compartidas
- **L1d es de 48 KB** (no 32 KB), lo cual es inusual y generoso
- La L2 en este procesador es muy grande (1.25 MB), mucho mayor que en procesadores típicos (256-512 KB)
- La L3 es compartida pero no es el foco de esta práctica
- Al ejecutar en un solo core, evitamos efectos de contención de caché compartida

#### Relación entre R, D y L

Dado que cada elemento `double` ocupa 8 bytes y queremos acceder a L líneas caché diferentes:

```
L = (R × D × 8) / 64
```

Despejando R:

```
R = (L × 64) / (D × 8) = (L × 8) / D
```

#### Tabla de Valores de R

Para **S1 = 768** y **S2 = 20480**:

| L         | Valor L | D=1    | D=8   | D=16  | D=64  | D=128 |
|-----------|---------|--------|-------|-------|-------|-------|
| 0.5×S1    | 384     | 3072   | 384   | 192   | 48    | 24    |
| 1.5×S1    | 1152    | 9216   | 1152  | 576   | 144   | 72    |
| 0.5×S2    | 10240   | 81920  | 10240 | 5120  | 1280  | 640   |
| 0.75×S2   | 15360   | 122880 | 15360 | 7680  | 1920  | 960   |
| 2×S2      | 40960   | 327680 | 40960 | 20480 | 5120  | 2560  |
| 4×S2      | 81920   | 655360 | 81920 | 40960 | 10240 | 5120  |
| 8×S2      | 163840  | 1310720| 163840| 81920 | 20480 | 10240 |

**Cálculo**: R = (L × 8) / D

**Nota**: Los valores de L han cambiado porque ahora S1 = 768 líneas (L1d de 48 KB)

---

## 3. Implementación

### 3.1 Estructura del Programa

El programa se implementa en C estándar (C17) con las siguientes características:

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>
#include "counter.h"

int main(int argc, char *argv[]) {
    // 1. Lectura de parámetros D y L
    // 2. Cálculo de R y tamaño del vector
    // 3. Reserva de memoria alineada
    // 4. Inicialización de índices y datos
    // 5. Calentamiento de caché
    // 6. Medición de ciclos
    // 7. Cálculo de resultados
    // 8. Liberación de memoria
}
```

### 3.2 Código Completo

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>
#include "counter.h"

int main(int argc, char *argv[]) {
    // Verificar argumentos de entrada
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <D> <L>\n", argv[0]);
        fprintf(stderr, "  D: stride (distancia entre elementos)\n");
        fprintf(stderr, "  L: número de líneas caché a acceder\n");
        return 1;
    }
    
    // Leer parámetros de entrada
    int D = atoi(argv[1]);  // Stride
    int L = atoi(argv[2]);  // Líneas caché
    
    // Constantes de la arquitectura
    const int CLS = 64;  // Tamaño de línea caché en bytes
    
    // Calcular número de elementos a sumar
    int R = (L * CLS) / (D * sizeof(double));
    
    // Calcular tamaño total del vector
    // Necesitamos hasta el índice (R-1)*D
    int N = (R - 1) * D + 1;
    
    // Verificar que R es razonable
    if (R <= 0) {
        fprintf(stderr, "Error: R=%d no válido (D=%d, L=%d)\n", R, D, L);
        return 1;
    }
    
    // Reservar memoria alineada para el vector A
    double *A = aligned_alloc(CLS, N * sizeof(double));
    
    // Reservar memoria para el vector de índices
    int *ind = malloc(R * sizeof(int));
    
    // Vector para almacenar resultados de las 10 repeticiones
    double S[10];
    
    // Verificar que la reserva fue exitosa
    if (!A || !ind) {
        fprintf(stderr, "Error: No se pudo reservar memoria\n");
        if (A) free(A);
        if (ind) free(ind);
        return 1;
    }
    
    // Inicializar vector de índices
    // ind[i] = i * D para i = 0, 1, ..., R-1
    for (int i = 0; i < R; i++) {
        ind[i] = i * D;
    }
    
    // Fase de calentamiento: inicializar datos con valores aleatorios
    // Valores en el intervalo [1,2) con signo aleatorio
    srand(time(NULL));
    for (int i = 0; i < R; i++) {
        // Generar valor en [1, 2)
        double val = 1.0 + ((double)rand() / RAND_MAX);
        // Signo aleatorio
        if (rand() % 2) {
            val = -val;
        }
        A[ind[i]] = val;
    }
    
    // Imprimir frecuencia del procesador
    double freq = mhz(1, 1);
    
    // Iniciar medición de ciclos
    start_counter();
    
    // Realizar 10 repeticiones de la reducción
    for (int rep = 0; rep < 10; rep++) {
        double suma = 0.0;
        
        // Suma de R elementos con acceso indirecto
        for (int i = 0; i < R; i++) {
            suma += A[ind[i]];
        }
        
        // Almacenar resultado para evitar optimizaciones
        S[rep] = suma;
    }
    
    // Obtener ciclos totales
    double ciclos_totales = get_counter();
    
    // Imprimir resultados de las sumas (fuera de la medición)
    printf("\n=== Resultados de las sumas ===\n");
    for (int i = 0; i < 10; i++) {
        printf("S[%d] = %.10f\n", i, S[i]);
    }
    
    // Calcular ciclos por acceso
    // Total de accesos = 10 repeticiones × R elementos
    double ciclos_por_acceso = ciclos_totales / (10.0 * R);
    
    // Imprimir resultados del experimento
    printf("\n=== Resultados del Experimento ===\n");
    printf("Parámetros:\n");
    printf("  D (stride)            = %d\n", D);
    printf("  L (líneas caché)      = %d\n", L);
    printf("  R (elementos sumados) = %d\n", R);
    printf("  N (tamaño vector)     = %d\n", N);
    printf("  Frecuencia CPU        = %.1f MHz\n", freq);
    printf("\nResultados:\n");
    printf("  Ciclos totales        = %.0f\n", ciclos_totales);
    printf("  Ciclos por acceso     = %.4f\n", ciclos_por_acceso);
    printf("  Tiempo por acceso     = %.4f ns\n", 
           (ciclos_por_acceso / freq) * 1000);
    
    // Liberar memoria
    free(A);
    free(ind);
    
    return 0;
}
```

### 3.3 Script de Ejecución para FinisTerrae III

```bash
#!/bin/bash
# Solicitar un nodo con 1 core durante 2 horas
#SBATCH -n 1 -c 1 -t 02:00:00 --mem=4G
#SBATCH --job-name p1acg##  # Sustituir ## por número de equipo

# Compilar con gcc sin optimizaciones
gcc acp1.c -o acp1 -O0

# Crear directorio para resultados
mkdir -p resultados

# Valores de S1 y S2 para Intel Xeon Platinum 8352Y (un solo core)
# L1d: 48 KB → S1 = 768 líneas (privada del core)
# L2: 1280 KB → S2 = 20480 líneas (privada del core)
S1=768
S2=20480

# Calcular valores de L
L1=$((S1 / 2))      # 0.5 × S1 = 384
L2=$((S1 * 3 / 2))  # 1.5 × S1 = 1152
L3=$((S2 / 2))      # 0.5 × S2 = 10240
L4=$((S2 * 3 / 4))  # 0.75 × S2 = 15360
L5=$((S2 * 2))      # 2 × S2 = 40960
L6=$((S2 * 4))      # 4 × S2 = 81920
L7=$((S2 * 8))      # 8 × S2 = 163840

# Valores de D (stride)
STRIDES="1 8 16 64 128"
LINEAS="$L1 $L2 $L3 $L4 $L5 $L6 $L7"

echo "Iniciando experimentos..."
echo "Procesador: Intel Xeon Platinum 8352Y (1 core)"
echo "L1d: 48 KB → S1 = $S1 líneas (privada, 12-way)"
echo "L2: 1280 KB → S2 = $S2 líneas (privada, 20-way)"
echo "Valores de L: $LINEAS"
echo "Valores de D: $STRIDES"

# Ejecutar experimentos
for D in $STRIDES
do
    echo ""
    echo "=== Ejecutando experimentos para D=$D ==="
    
    for L in $LINEAS
    do
        echo "  Experimento D=$D, L=$L"
        
        # Realizar 10 mediciones independientes
        for rep in {1..10}
        do
            ./acp1 $D $L >> resultados/D${D}_L${L}.txt
        done
    done
done

echo ""
echo "Experimentos completados. Resultados en directorio 'resultados/'"
```

---

## 4. Experimentos Adicionales

### 4.1 Comparación con Tipo `int`

Para obtener la máxima calificación, se debe comparar el comportamiento usando datos de tipo `int` en lugar de `double`.

**Cambios en el código**:
```c
// Cambiar declaración del vector A
int *A = aligned_alloc(CLS, N * sizeof(int));

// Cambiar vector de resultados
int S[10];

// Ajustar cálculo de R
int R = (L * CLS) / (D * sizeof(int));
```

**Análisis esperado**:
- `sizeof(int) = 4 bytes` vs `sizeof(double) = 8 bytes`
- Caben el doble de enteros por línea caché
- Mejor utilización de cada línea caché cargada
- Posible reducción en ciclos/acceso para D pequeños

### 4.2 Acceso Directo vs Indirecto

Comparar el uso de `A[ind[i]]` (indirecto) vs `A[i*D]` (directo).

**Cambios en el código**:
```c
// Versión con acceso directo (sin vector ind[])
for (int i = 0; i < R; i++) {
    suma += A[i * D];
}
```

**Análisis esperado**:
- El acceso indirecto añade overhead de cargar `ind[i]`
- Posible penalización en latencia
- El prefetcher puede comportarse diferente

---

## 5. Análisis de Resultados

### 5.1 Gráficas Esperadas

Se debe generar una gráfica con:

- **Eje X**: Número de líneas caché (L)
- **Eje Y**: Ciclos por acceso
- **Series**: Una curva para cada valor de D

**Estructura esperada de la gráfica**:

```
Ciclos/acceso
     ^
     |
 200 |                                    D=128 ──────
     |                               D=64 ─────
 150 |                          
     |                     D=16 ────
 100 |               
     |          D=8  ───
  50 |     
     |  D=1 ───────────
   0 |_________________________________________________> L (líneas)
     0   384 1152   10K   15K        40K    80K   160K
        (L1)  (↓)   (L2)  (↓)        (RAM)
```

**Puntos de transición esperados:**
- **L < 768** (< S1): Datos en L1, ~5 ciclos/acceso
- **L ≈ 768-1152**: Transición L1→L2
- **768 < L < 20480**: Datos en L2, ~14 ciclos/acceso
- **L > 20480** (> S2): Acceso a RAM, >100 ciclos/acceso

**Nota**: Con S1 = 768 líneas (48 KB L1d) y S2 = 20480 líneas (1.25 MB L2), la transición a memoria principal ocurrirá mucho más tarde que en procesadores con L2 más pequeña.

### 5.2 Interpretación de Comportamientos

#### Región 1: L < S1 (datos en L1)

- **Ciclos/acceso esperados: ~5 ciclos** (latencia documentada de L1)
- Todos los datos caben en caché L1 (48 KB)
- **Efecto de D**:
  - D=1: Máximo aprovechamiento de prefetching y localidad espacial
  - D=8: Un `double` por línea (64/8=8), aún buen aprovechamiento
  - D grande: Desperdicio de líneas caché cargadas

#### Región 2: S1 < L < S2 (datos en L2)

- **Ciclos/acceso esperados: ~14 ciclos** (latencia documentada de L2)
- Los datos ya no caben en L1 (768 líneas), se usa L2
- La L2 es muy grande (1.25 MB = 20480 líneas)
- Se observa un incremento claro desde ~5 a ~14 ciclos

#### Región 3: L > S2 (datos en RAM)

- **Ciclos elevados: >100-200 ciclos**
- Los datos requieren acceso a memoria principal
- **Mayor impacto de D**:
  - D pequeño: El prefetcher puede anticipar accesos (~100-150 ciclos)
  - D grande: Cada acceso es un miss completo (~200+ ciclos)
  - Sin prefetching efectivo, cada acceso va a RAM

### 5.3 Efecto del Prefetching

Los procesadores modernos incluyen hardware prefetcher que detecta patrones de acceso:

**Stride Prefetcher**:
- Detecta accesos con stride constante
- Funciona bien para D pequeños y regulares
- Se degrada con D muy grandes o irregulares

**Comportamiento esperado**:
- **D=1**: Prefetching óptimo (acceso secuencial)
- **D=8-16**: Prefetching aún efectivo
- **D=64-128**: Prefetching degradado o inefectivo

### 5.4 Localidad Espacial vs Temporal

**Localidad Espacial**:
- Determinada por el valor de D
- D pequeño → alta localidad espacial
- D grande → baja localidad espacial

**Localidad Temporal**:
- Las 10 repeticiones acceden a los mismos datos
- Para L pequeño (< S1), alta reutilización en caché
- Para L grande (> S2), los datos se desalojan entre repeticiones

---

## 6. Tratamiento Estadístico

### 6.1 Variabilidad de Mediciones

Debido a efectos del sistema operativo, otras tareas, etc., es importante:

1. **Realizar múltiples mediciones** (10 repeticiones sugeridas)
2. **Seleccionar las 3 mejores** (menor número de ciclos)
3. **Calcular media geométrica**:

```
Media_geométrica = (x₁ × x₂ × x₃)^(1/3)
```

### 6.2 Script de Procesamiento

```python
import numpy as np

def procesar_mediciones(archivo):
    # Leer todas las mediciones
    ciclos = []
    with open(archivo) as f:
        for linea in f:
            if "Ciclos por acceso" in linea:
                valor = float(linea.split('=')[1].strip())
                ciclos.append(valor)
    
    # Ordenar y tomar las 3 mejores
    ciclos_ordenados = sorted(ciclos)
    top_3 = ciclos_ordenados[:3]
    
    # Media geométrica
    media_geom = np.prod(top_3) ** (1/3)
    
    return media_geom
```

---

## 7. Plataforma de Ejecución

### 7.1 FinisTerrae III

**Características relevantes**:
- Supercomputador del CESGA (Centro de Supercomputación de Galicia)
- Sistema operativo: Linux
- Compilador: GCC
- Acceso mediante sistema de colas SLURM

### 7.2 Comandos Útiles

```bash
# Consultar arquitectura del procesador
lscpu

# Ver información de caché
lscpu | grep -i cache
cat /proc/cpuinfo

# Enviar trabajo a cola
sbatch script_experimentos.sh

# Consultar estado del trabajo
squeue -u nombre_usuario

# Ver salida del trabajo
cat slurm-JOBID.out
```

### 7.3 Compilación

**Opciones de compilación**:
```bash
gcc acp1.c -o acp1 -O0
```

- `-O0`: **Sin optimizaciones** (requerido por la práctica)
- Permite observar el comportamiento real de los accesos a memoria
- El compilador no reordena ni elimina accesos

---

## 8. Entrega y Evaluación

### 8.1 Formato de Entrega

**Contenido del informe**:
1. Breve descripción de la metodología
2. Gráficas con ejes etiquetados y títulos
3. Interpretación de resultados
4. Código fuente comentado

**Formato**: PDF

**Forma**: Presencial, explicación al profesor

### 8.2 Criterios de Evaluación

- ✓ Cumplimiento de especificaciones (40% de nota de prácticas)
- ✓ Calidad de resultados y gráficas
- ✓ Calidad de las explicaciones
- ✓ Autonomía en el desarrollo
- ✓ Código comentado y funcional
- ✓ Realización de experimentos adicionales

### 8.3 Requisitos Obligatorios

1. **El código debe compilar y ejecutarse correctamente**
2. Usar C estándar (C17)
3. Un único fichero fuente
4. Incluir `#include "counter.h"`
5. Código comentado pero sin exceso
6. No usar directivas específicas de GCC

---

## 9. Conclusiones

### 9.1 Aprendizajes Esperados

Esta práctica permite comprender experimentalmente:

1. **Impacto de la jerarquía de caché** en el rendimiento
2. **Importancia de la localidad** en el acceso a datos
3. **Funcionamiento del prefetching** en procesadores modernos
4. **Técnicas de medición** de rendimiento a bajo nivel
5. **Diseño de experimentos** sistemáticos

### 9.2 Aplicaciones Prácticas

Los conocimientos adquiridos son fundamentales para:

- Optimización de código de alto rendimiento
- Diseño de algoritmos cache-friendly
- Comprensión de bottlenecks de rendimiento
- Programación de sistemas HPC (High Performance Computing)

### 9.3 Recomendaciones

Para obtener buenos resultados:

1. **Verificar cuidadosamente** S1 y S2 del procesador
2. **Ejecutar experimentos individuales** (uno por ejecución)
3. **Realizar fase de calentamiento** adecuada
4. **Tomar múltiples mediciones** para robustez estadística
5. **Documentar** todas las decisiones tomadas

---

## Anexos

### Anexo A: Tabla de Resultados (Plantilla)

| D   | L     | R     | Ciclos Totales | Ciclos/Acceso | Región Caché |
|-----|-------|-------|----------------|---------------|--------------|
| 1   | 256   | 2048  | ...            | ...           | L1           |
| 1   | 768   | 6144  | ...            | ...           | L1→L2        |
| 1   | 2048  | 16384 | ...            | ...           | L2           |
| ... | ...   | ...   | ...            | ...           | ...          |

### Anexo B: Información del Procesador FinisTerrae III

```
Sistema: FinisTerrae III (CESGA)
Procesador: 2x Intel Xeon Platinum 8352Y
Arquitectura: x86_64
Cores totales: 64 (32 por socket)
Threads por core: 2
Frecuencia base: 2.2 GHz
Frecuencia turbo: 3.4 GHz

Jerarquía de Caché Total:
┌─────────────────────────────────────────────────────┐
│ L1 Instruction Cache: 32 x 32 KB  = 1 MB            │
│                       (8-way set associative)        │
│ L1 Data Cache:        32 x 48 KB  = 1.5 MB          │
│                       (12-way set associative)       │
│ L2 Cache:             32 x 1280 KB = 40 MB          │
│                       (20-way set associative)       │
│ L3 Cache:             48 MB (compartida)             │
│                       (12-way set associative)       │
└─────────────────────────────────────────────────────┘

Latencias de Caché:
- L1: 5 ciclos
- L2: 14 ciclos
- L3: ~40-50 ciclos (estimado)
- RAM: ~200+ ciclos

CONFIGURACIÓN PARA LA PRÁCTICA (1 CORE):
┌─────────────────────────────────────────────┐
│ Caché Privada del Core:                     │
│ - L1i: 32 KB → 512 líneas de 64 bytes      │
│        8-way set associative                │
│ - L1d: 48 KB → 768 líneas de 64 bytes      │
│        12-way set associative               │
│ - L2: 1280 KB → 20480 líneas de 64 bytes   │
│        20-way set associative               │
│                                             │
│ Caché L3 (compartida, no relevante):        │
│ - Total: 48 MB                              │
│        12-way set associative               │
└─────────────────────────────────────────────┘

Tamaño de línea caché: 64 bytes
```

**Valores calculados para la práctica (1 core):**
- **CLS** (Cache Line Size) = 64 bytes
- **S1** = 768 líneas (L1 datos de 48 KB - privada del core)
- **S2** = 20480 líneas (L2 de 1280 KB - privada del core)

**Características destacables:**
- **L1d de 48 KB** es inusualmente grande (típicamente 32 KB)
- **L2 de 1.25 MB** por core es muy generosa (típicamente 256-512 KB)
- Alta asociatividad: 12-way (L1d), 20-way (L2)
- Esto permite menos conflictos en caché y mejor rendimiento

**Nota importante sobre L3:**
- La L3 de 48 MB es compartida entre todos los cores del socket
- En esta práctica NO la consideramos porque:
  1. Trabajamos con un solo core
  2. L1 y L2 son privadas y suficientes para estudiar la jerarquía
  3. El enunciado especifica solo L1 y L2

**Ventajas de usar un solo core:**
- No hay contención por caché compartida (L3)
- Resultados más consistentes y reproducibles
- Menor interferencia del sistema operativo
- Cachés L1 y L2 dedicadas exclusivamente a nuestro experimento

**Comandos para verificar:**
```bash
# Ver información de caché
lscpu | grep -i cache

# Detalles por nivel (para el core 0)
cat /sys/devices/system/cpu/cpu0/cache/index0/size  # L1d: 48K
cat /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity  # 12
cat /sys/devices/system/cpu/cpu0/cache/index2/size  # L2: 1280K
cat /sys/devices/system/cpu/cpu0/cache/index2/ways_of_associativity  # 20
cat /sys/devices/system/cpu/cpu0/cache/index3/size  # L3: 49152K

# Tamaño de línea caché
cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size  # 64

# Latencias (si está disponible perf)
perf stat -e cache-references,cache-misses ./programa
```

### Anexo C: Referencias

- Intel 64 and IA-32 Architectures Optimization Reference Manual
- "What Every Programmer Should Know About Memory" - Ulrich Drepper
- Documentación de GCC: https://gcc.gnu.org/
- CESGA: https://www.cesga.es/

---

**Fin del Informe**
