#!/bin/bash
#SBATCH -n 1
#SBATCH -c 1
#SBATCH --mem=16G
#SBATCH -t 02:00:00
#SBATCH --job-name p1acg_eqNN

# ============================================================
# CAMBIOS RESPECTO A LA VERSIÓN ANTERIOR:
#
# 1. -t aumentado de 01:00:00 a 02:00:00 porque los nuevos
#    valores de L son mucho más grandes y tardan más.
#
# 2. Añadidos 4 nuevos valores de L para desbordar L3 y
#    alcanzar RAM real (los 7 originales no se han tocado):
#
#    Val_L8  = 524288  → 32 MB  → dentro de L3 (punto de referencia)
#    Val_L9  = 786432  → 48 MB  → límite exacto de L3
#    Val_L10 = 1572864 → 96 MB  → RAM real (desborda L3)
#    Val_L11 = 3145728 → 192 MB → RAM profunda
#
# 3. S3 añadido como constante documentada:
#    S3 (L3) = 48 MB / 64 B = 786432 líneas
#
# Los resultados nuevos van a la misma carpeta 'resultados/'
# con el mismo formato de nombre D${D}_L${L}.txt, por lo que
# son compatibles con el analisis_comparativo.py existente.
# ============================================================

echo "Compilando..."
gcc acp1.c -o acp1 -O0

mkdir -p resultados

# --- Parámetros de arquitectura Intel Xeon Platinum 8352Y ---
# S1 (L1d) = 48 KB  / 64 B = 768 líneas
# S2 (L2)  = 1.25 MB / 64 B = 20480 líneas
# S3 (L3)  = 48 MB  / 64 B = 786432 líneas   ← NUEVO

# --- Valores originales (L1, L2 y L3 cache) ---
Val_L1=384      # 0.5 × S1 → 24 KB  → L1
Val_L2=1152     # 1.5 × S1 → 72 KB  → L2 (desborda L1)
Val_L3=10240    # 0.5 × S2 → 640 KB → L2 holgado
Val_L4=15360    # 0.75× S2 → 960 KB → L2 ajustado
Val_L5=40960    # 2.0 × S2 → 2.5 MB → L3 (desborda L2)
Val_L6=81920    # 4.0 × S2 → 5 MB   → L3
Val_L7=163840   # 8.0 × S2 → 10 MB  → L3

# --- Valores nuevos (para desbordar L3 y llegar a RAM) ---  ← NUEVO
Val_L8=524288   # 0.67× S3 → 32 MB  → L3 (punto de referencia pre-desborde)
Val_L9=786432   # 1.0 × S3 → 48 MB  → límite exacto de L3
Val_L10=1572864 # 2.0 × S3 → 96 MB  → RAM real
Val_L11=3145728 # 4.0 × S3 → 192 MB → RAM profunda

# --- Strides (sin cambios) ---
STRIDES="1 8 16 64 128"

# --- Lista completa de L (originales + nuevos) ---          ← CAMBIADO
LINES="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7 $Val_L8 $Val_L9 $Val_L10 $Val_L11"

echo "Iniciando experimentos..."
echo "S1 (L1)=768  → hasta 24 KB"
echo "S2 (L2)=20480 → hasta 1.25 MB"
echo "S3 (L3)=786432 → hasta 48 MB"
echo "Nuevos valores alcanzan RAM (96 MB y 192 MB)"

# 10 repeticiones externas para estadística
for i in {1..10}
do
    echo "Repetición global $i de 10..."
    for D in $STRIDES
    do
        for L in $LINES
        do
            ./acp1 $D $L >> resultados/D${D}_L${L}.txt
        done
    done
done

echo "Finalizado. Datos en carpeta 'resultados/'"
