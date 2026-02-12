#!/bin/bash
#SBATCH -n 1
#SBATCH -c 1           # CRÍTICO: 1 core para evitar migración y cache thrashing
#SBATCH -t 00:40:00
#SBATCH --mem=16G      # 16GB es suficiente y entra rápido en cola
#SBATCH --job-name p1_final

# 1. Compilación
gcc acp1.c -o acp1 -O0

# 2. Preparar salida
mkdir -p resultados
rm -f resultados/*.txt # Limpiar ejecuciones previas

# 3. Parámetros FinisTerrae III (Ice Lake)
# S1=768 líneas, S2=20480 líneas
L_VALS="384 1152 10240 15360 40960 81920 163840"
D_VALS="1 8 16 64 128"

echo "Iniciando barrido de memoria..."

# 4. Ejecución
# Hacemos el bucle de repeticiones EXTERNO al archivo.
# Esto genera 10 líneas de datos por cada archivo .txt
for D in $D_VALS; do
    for L in $L_VALS; do
        echo "Procesando D=$D, L=$L ..."
        for rep in {1..10}; do
            ./acp1 $D $L >> resultados/D${D}_L${L}.txt
        done
    done
done

echo "Fin. Datos listos en carpeta 'resultados/'"
