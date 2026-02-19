#!/bin/bash
#SBATCH -n 1
#SBATCH -c 1
#SBATCH --mem=16G
#SBATCH -t 01:00:00
#SBATCH --job-name p1acg_int

# Experimento adicional 1: array de tipo INT en lugar de DOUBLE
# Comparar con los resultados de script_experimentos.sh (tipo double)

echo "Compilando acp1_int.c..."
gcc acp1_int.c -o acp1_int -O0

mkdir -p resultados_int

# Mismos parámetros de arquitectura que el experimento base
# S1 = 768 líneas (L1d = 48 KB), S2 = 20480 líneas (L2 = 1.25 MB)
# NOTA: para int (4 bytes), R = (L * 64) / (D * 4) = el DOBLE que para double
# Los valores de L son los mismos para que la comparación sea directa

Val_L1=384
Val_L2=1152
Val_L3=10240
Val_L4=15360
Val_L5=40960
Val_L6=81920
Val_L7=163840

STRIDES="1 8 16 64 128"
LINES="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7"

echo "Iniciando experimentos con INT..."
echo "S1=768, S2=20480 (mismos que experimento base)"

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