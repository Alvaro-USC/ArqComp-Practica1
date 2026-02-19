#!/bin/bash
#SBATCH -n 1
#SBATCH -c 1
#SBATCH --mem=16G
#SBATCH -t 01:00:00
#SBATCH --job-name p1acg_dir

# Experimento adicional 2: acceso DIRECTO A[i*D] en lugar de indirecto A[ind[i]]
# Comparar con los resultados de script_experimentos.sh (acceso indirecto)

echo "Compilando acp1_directo.c..."
gcc acp1_directo.c -o acp1_directo -O0

mkdir -p resultados_directo

# Mismos parámetros que el experimento base para comparación directa
Val_L1=384
Val_L2=1152
Val_L3=10240
Val_L4=15360
Val_L5=40960
Val_L6=81920
Val_L7=163840

STRIDES="1 8 16 64 128"
LINES="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7"

echo "Iniciando experimentos con acceso DIRECTO..."
echo "S1=768, S2=20480 (mismos que experimento base)"

for i in {1..10}
do
    echo "Repetición global $i de 10..."
    for D in $STRIDES
    do
        for L in $LINES
        do
            ./acp1_directo $D $L >> resultados_directo/D${D}_L${L}.txt
        done
    done
done

echo "Finalizado. Datos en carpeta 'resultados_directo/'"