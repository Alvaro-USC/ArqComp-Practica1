#!/bin/bash
#SBATCH -n 1 
#SBATCH -c 1 
#SBATCH --mem=16G 
#SBATCH -t 01:00:00
#SBATCH --job-name p1acg_eqNN 

# NOTA: He cambiado -c 64 a -c 1 y bajado la memoria.
# Para medir latencia de caché L1/L2 es CRÍTICO usar un solo core 
# para evitar migraciones y ruido. 16G son suficientes.

# 1. Compilación
# -O0 es vital para que no se elimine el bucle de acceso a memoria
echo "Compilando..."
gcc acp1.c -o acp1 -O0

# Crear carpeta de resultados si no existe
mkdir -p resultados

# 2. Definición de Parámetros para Intel Xeon Platinum 8352Y
# S1 (L1d) = 48KB / 64B = 768 líneas
# S2 (L2)  = 1.25MB / 64B = 20480 líneas

# Definimos los valores de L (Líneas a acceder)
# L1 = 0.5 * S1 (Cae en L1)
Val_L1=384
# L2 = 1.5 * S1 (Desborda L1, cae en L2)
Val_L2=1152
# L3 = 0.5 * S2 (Cae en L2 holgadamente)
Val_L3=10240
# L4 = 0.75 * S2 (Cae en L2)
Val_L4=15360
# L5 = 2.0 * S2 (Desborda L2, cae en RAM)
Val_L5=40960
# L6 = 4.0 * S2 (RAM profunda)
Val_L6=81920
# L7 = 8.0 * S2 (RAM muy profunda)
Val_L7=163840

# Lista de valores de D (Stride/Salto)
STRIDES="1 8 16 64 128"

# Lista de valores de L
LINES="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7"

echo "Iniciando experimentos..."
echo "S1 (L1)=768, S2 (L2)=20480"

# 3. Bucle de Ejecución
# Repetimos 10 veces el experimento completo (estadística externa)
for i in {1..10}
do
    echo "Repetición global $i de 10..."
    
    for D in $STRIDES
    do
        for L in $LINES
        do
            # Ejecutamos y guardamos en archivo específico
            # >> concatena al final del archivo para tener las 10 mediciones juntas
            ./acp1 $D $L >> resultados/D${D}_L${L}.txt
        done
    done
done

echo "Finalizado. Datos en carpeta 'resultados/'"
