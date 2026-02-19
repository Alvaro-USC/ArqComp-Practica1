#!/bin/bash
#SBATCH -n 1
#SBATCH -c 1
#SBATCH --mem=16G
#SBATCH -t 02:00:00
#SBATCH --job-name p1acg_int

# ============================================================
# CAMBIOS RESPECTO A LA VERSIÓN ANTERIOR:
#
# 1. -t aumentado de 01:00:00 a 02:00:00.
#
# 2. Añadidos los mismos 4 valores nuevos de L que en
#    script_experimentos.sh para mantener comparabilidad:
#    Val_L8  = 524288  → 32 MB  → L3
#    Val_L9  = 786432  → 48 MB  → límite L3
#    Val_L10 = 1572864 → 96 MB  → RAM real
#    Val_L11 = 3145728 → 192 MB → RAM profunda
#
# NOTA sobre tipo int: con int (4 bytes) R = el DOBLE que con
# double para el mismo L. Para L=3145728 y D=1:
#   R = 3145728 × 64 / (1 × 4) = 50.331.648 elementos
#   A[] ocupa ~192 MB de footprint en caché (mismo que double)
# ============================================================

echo "Compilando acp1_int.c..."
gcc acp1_int.c -o acp1_int -O0

mkdir -p resultados_int

# --- Parámetros de arquitectura ---
# S1 (L1d) = 768 líneas   → 48 KB
# S2 (L2)  = 20480 líneas → 1.25 MB
# S3 (L3)  = 786432 líneas → 48 MB   ← NUEVO

# --- Valores originales ---
Val_L1=384
Val_L2=1152
Val_L3=10240
Val_L4=15360
Val_L5=40960
Val_L6=81920
Val_L7=163840

# --- Valores nuevos (L3 completa y RAM) ---               ← NUEVO
Val_L8=524288   # 0.67× S3 → 32 MB  → L3
Val_L9=786432   # 1.0 × S3 → 48 MB  → límite L3
Val_L10=1572864 # 2.0 × S3 → 96 MB  → RAM real
Val_L11=3145728 # 4.0 × S3 → 192 MB → RAM profunda

# --- Strides (sin cambios) ---
STRIDES="1 8 16 64 128"

# --- Lista completa de L ---                              ← CAMBIADO
LINES="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7 $Val_L8 $Val_L9 $Val_L10 $Val_L11"

echo "Iniciando experimentos con INT..."
echo "S1=768, S2=20480, S3=786432"
echo "Nuevos valores alcanzan RAM (96 MB y 192 MB)"

for i in {1..10}
do
    echo "Repetición global $i de 10..."
    for D in $STRIDES
    do
        for L in $LINES
        do
            ./acp1_int $D $L >> resultados_int/D${D}_L${L}.txt
        done
    done
done

echo "Finalizado. Datos en carpeta 'resultados_int/'"
