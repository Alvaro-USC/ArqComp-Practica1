# Valores Experimentales para FinisTerrae III
# Intel Xeon Platinum 8352Y - Ejecución en 1 CORE

## Características del Procesador

- **Sistema**: FinisTerrae III (CESGA)
- **Procesador**: 2x Intel Xeon Platinum 8352Y
- **Configuración de la práctica**: **1 solo core**
- **Tamaño de línea caché (CLS)**: 64 bytes

## Cachés Privadas del Core (las que usamos)

- **L1i (instrucciones)**: 32 KB (8-way set associative)
- **L1d (datos)**: 48 KB (12-way set associative) ← **La que usamos para S1**
- **L2 (unificada)**: 1280 KB = 1.25 MB (20-way set associative)
- **L3 (compartida)**: 48 MB (12-way set associative, NO la usamos)

## Latencias de Caché

- **L1**: 5 ciclos
- **L2**: 14 ciclos  
- **L3**: ~40-50 ciclos (estimado)
- **RAM**: ~200+ ciclos

## Valores Calculados

- **S1** (líneas en L1d): 48 KB / 64 B = **768 líneas**
- **S2** (líneas en L2): 1280 KB / 64 B = **20480 líneas**

**Importante**: 
- Solo consideramos L1d y L2 porque son privadas del core
- Al usar 1 core, evitamos interferencias y resultados más limpios
- La L3 es compartida y no es el foco de este estudio
- **L1d es de 48 KB**, no los típicos 32 KB

## Valores de L (líneas caché a acceder)

| Descripción | Fórmula      | Valor L |
|-------------|--------------|---------|
| 0.5 × S1    | 768 / 2      | 384     |
| 1.5 × S1    | 768 × 1.5    | 1152    |
| 0.5 × S2    | 20480 / 2    | 10240   |
| 0.75 × S2   | 20480 × 0.75 | 15360   |
| 2 × S2      | 20480 × 2    | 40960   |
| 4 × S2      | 20480 × 4    | 81920   |
| 8 × S2      | 20480 × 8    | 163840  |

## Valores de D (stride)

D = {1, 8, 16, 64, 128}

## Tabla Completa: R (elementos a sumar) para cada combinación D×L

Fórmula: R = (L × 8) / D

### D = 1 (acceso consecutivo)

| L       | R       | Tamaño A[] | Comentario                    |
|---------|---------|------------|-------------------------------|
| 384     | 3072    | 3072       | 0.5×L1 - La mitad de L1      |
| 1152    | 9216    | 9216       | 1.5×L1 - Excede L1, usa L2   |
| 10240   | 81920   | 81920      | 0.5×L2 - La mitad de L2      |
| 15360   | 122880  | 122880     | 0.75×L2 - Tres cuartos de L2 |
| 40960   | 327680  | 327680     | 2×L2 - No cabe en L2         |
| 81920   | 655360  | 655360     | 4×L2 - Mucho mayor que L2    |
| 163840  | 1310720 | 1310720    | 8×L2 - Acceso a RAM          |

### D = 8 (un double por línea caché)

| L       | R      | Tamaño A[] | Comentario                    |
|---------|--------|------------|-------------------------------|
| 384     | 384    | 2689       | 0.5×L1                       |
| 1152    | 1152   | 8065       | 1.5×L1                       |
| 10240   | 10240  | 71681      | 0.5×L2                       |
| 15360   | 15360  | 107521     | 0.75×L2                      |
| 40960   | 40960  | 286721     | 2×L2                         |
| 81920   | 81920  | 573441     | 4×L2                         |
| 163840  | 163840 | 1146881    | 8×L2                         |

### D = 16

| L       | R      | Tamaño A[] | Comentario                    |
|---------|--------|------------|-------------------------------|
| 384     | 192    | 3057       | 0.5×L1                       |
| 1152    | 576    | 9201       | 1.5×L1                       |
| 10240   | 5120   | 81921      | 0.5×L2                       |
| 15360   | 7680   | 122881     | 0.75×L2                      |
| 40960   | 20480  | 327681     | 2×L2                         |
| 81920   | 40960  | 655361     | 4×L2                         |
| 163840  | 81920  | 1310721    | 8×L2                         |

### D = 64

| L       | R     | Tamaño A[]  | Comentario                    |
|---------|-------|-------------|-------------------------------|
| 384     | 48    | 3009        | 0.5×L1                       |
| 1152    | 144   | 9153        | 1.5×L1                       |
| 10240   | 1280  | 81921       | 0.5×L2                       |
| 15360   | 1920  | 122881      | 0.75×L2                      |
| 40960   | 5120  | 327681      | 2×L2                         |
| 81920   | 10240 | 655361      | 4×L2                         |
| 163840  | 20480 | 1310721     | 8×L2                         |

### D = 128

| L       | R     | Tamaño A[]  | Comentario                    |
|---------|-------|-------------|-------------------------------|
| 384     | 24    | 2945        | 0.5×L1                       |
| 1152    | 72    | 9089        | 1.5×L1                       |
| 10240   | 640   | 81921       | 0.5×L2                       |
| 15360   | 960   | 122881      | 0.75×L2                      |
| 40960   | 2560  | 327681      | 2×L2                         |
| 81920   | 5120  | 655361      | 4×L2                         |
| 163840  | 10240 | 1310721     | 8×L2                         |

## Resumen de Experimentos

**Total de experimentos**: 5 valores de D × 7 valores de L = **35 experimentos**

**Con 10 repeticiones por experimento**: 350 ejecuciones

## Tamaños de Memoria Requeridos

### Máximo por experimento:
- **D=1, L=163840**: R=1,310,720 elementos
- Tamaño: 1,310,720 × 8 bytes = **10.49 MB**

### Mínimo por experimento:
- **D=128, L=256**: R=16 elementos
- Tamaño: 1,921 × 8 bytes = **15 KB**

## Comandos para el Script

```bash
# IMPORTANTE: Solicitar solo 1 core
#SBATCH -n 1 -c 1 -t 02:00:00 --mem=4G

# Valores de L (actualizados con S1=768)
L_VALUES="384 1152 10240 15360 40960 81920 163840"

# Valores de D
D_VALUES="1 8 16 64 128"

# Loop de ejecución
for D in $D_VALUES; do
    for L in $L_VALUES; do
        for rep in {1..10}; do
            ./acp1 $D $L >> resultados/D${D}_L${L}.txt
        done
    done
done
```

## Ventajas de Usar 1 Solo Core

1. **Cachés dedicadas**: L1 y L2 completamente dedicadas a nuestro experimento
2. **Sin contención**: No hay otros procesos compitiendo por las cachés privadas
3. **Resultados reproducibles**: Menor variabilidad entre ejecuciones
4. **Interpretación clara**: Los efectos observados son puramente de la jerarquía L1→L2→RAM
5. **Evita L3**: La L3 compartida añadiría complejidad innecesaria al análisis

## Valores Esperados de Rendimiento

### Ciclos por acceso (aproximados)

| Región      | L          | Ciclos/acceso | Latencia   | Observación              |
|-------------|------------|---------------|------------|--------------------------|
| L1          | < 768      | ~5            | ~2-3 ns    | Latencia documentada: 5  |
| Transición  | 768-1500   | 5-14          | ~3-7 ns    | Mix L1/L2                |
| L2          | 1.5K-20K   | ~14           | ~6-8 ns    | Latencia documentada: 14 |
| Transición  | 20K-40K    | 14-100        | ~10-50 ns  | Mix L2/RAM               |
| RAM         | > 40K      | 100-200+      | ~50-100 ns | Acceso a memoria principal|

**Notas importantes:**
- Las latencias documentadas son: L1=5 ciclos, L2=14 ciclos
- Con frecuencia de ~2.2-3.4 GHz → 1 ciclo ≈ 0.3-0.45 ns
- La transición L1→L2 ocurre en L ≈ 768-1152 (S1 = 768)
- La transición L2→RAM ocurre en L ≈ 20480-40960 (S2 = 20480)
- La gran L2 (1.25 MB) hará que la transición a RAM sea muy notable

## Efecto del Stride D

### D=1 (secuencial)
- Máxima reutilización de líneas caché
- Prefetcher muy efectivo
- Mínimos ciclos/acceso

### D=8 (un double por línea)
- Aprovecha completamente cada línea caché cargada
- Prefetcher aún efectivo
- Incremento moderado de ciclos

### D=16-64 (saltos de líneas)
- Desperdicio de líneas caché
- Prefetcher menos efectivo
- Incremento notable de ciclos

### D=128 (muy disperso)
- Máximo desperdicio
- Prefetcher probablemente inefectivo
- Máximos ciclos/acceso

## Archivo de Resultados Esperado

Cada archivo `resultados/D{D}_L{L}.txt` contendrá 10 mediciones con formato:

```
=== Resultados de las sumas ===
S[0] = ...
S[1] = ...
...
S[9] = ...

=== Resultados del Experimento ===
Parámetros:
  D (stride)            = X
  L (líneas caché)      = Y
  R (elementos sumados) = Z
  ...
Resultados:
  Ciclos totales        = ...
  Ciclos por acceso     = ...
  Tiempo por acceso     = ... ns
```

## Post-procesamiento

Para obtener el mejor resultado de cada experimento:

1. Ejecutar 10 repeticiones
2. Ordenar por ciclos/acceso (menor a mayor)
3. Tomar las 3 mejores mediciones
4. Calcular media geométrica: (x₁ × x₂ × x₃)^(1/3)

```python
import numpy as np

def procesar_experimento(archivo):
    ciclos = []
    with open(archivo) as f:
        for linea in f:
            if "Ciclos por acceso" in linea:
                valor = float(linea.split('=')[1].strip())
                ciclos.append(valor)
    
    # Ordenar y tomar top 3
    top3 = sorted(ciclos)[:3]
    
    # Media geométrica
    return np.prod(top3) ** (1/3)
```
