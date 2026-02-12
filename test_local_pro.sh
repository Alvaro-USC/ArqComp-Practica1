#!/bin/bash

# ==========================================
# TEST LOCAL EXHAUSTIVO (Simulación PRO)
# ==========================================

# 1. DETECCIÓN DE HARDWARE LOCAL
echo "=== 1. DETECTANDO HARDWARE ==="

# Intentar obtener tamaños en Bytes (fallback a valores estándar si falla)
L1_BYTES=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null || echo 32768)   # 32KB default
L2_BYTES=$(getconf LEVEL2_CACHE_SIZE 2>/dev/null || echo 262144)   # 256KB default

# Calcular S1 y S2 (Líneas de 64 bytes)
S1=$((L1_BYTES / 64))
S2=$((L2_BYTES / 64))

echo "   > L1 Detectada: $(($L1_BYTES/1024)) KB -> S1 = $S1 líneas"
echo "   > L2 Detectada: $(($L2_BYTES/1024)) KB -> S2 = $S2 líneas"

# 2. DEFINICIÓN DE LA MATRIZ DE PRUEBAS
# Calculamos los mismos puntos de corte que en el guion, pero adaptados a TU PC
Val_L1=$((S1 / 2))         # 0.5 * S1
Val_L2=$((S1 * 3 / 2))     # 1.5 * S1
Val_L3=$((S2 / 2))         # 0.5 * S2
Val_L4=$((S2 * 3 / 4))     # 0.75 * S2
Val_L5=$((S2 * 2))         # 2.0 * S2
Val_L6=$((S2 * 4))         # 4.0 * S2
Val_L7=$((S2 * 8))         # 8.0 * S2 (~8 MB en tu Ryzen, sigue siendo L3)

# --- NUEVOS TAMAÑOS PARA RYZEN 9 ---
# Tu L3 es gigante (64MB+). Necesitamos ir mucho más lejos.
Val_L8=$((S2 * 64))        # ~64 MB (Frontera de la L3)
Val_L9=$((S2 * 256))       # ~256 MB (RAM Profunda - Aquí verás el salto >100 ciclos)

# Listas de iteración (AÑADE LAS NUEVAS VARIABLES AQUÍ AL FINAL)
L_VALS="$Val_L1 $Val_L2 $Val_L3 $Val_L4 $Val_L5 $Val_L6 $Val_L7 $Val_L8 $Val_L9"
D_VALS="1 8 16 64 128"

echo "   > Valores de L a probar: $L_VALS"
echo "   > Valores de D a probar: $D_VALS"
echo "-----------------------------------"

# 3. COMPILACIÓN
echo "=== 2. COMPILANDO ==="
# Aseguramos -O0 para evitar optimizaciones que rompan el test
if gcc acp1.c -o acp1_local -O0; then
    echo "   ✅ Compilación exitosa (acp1_local)."
else
    echo "   ❌ Error: No se pudo compilar. Revisa 'counter.h' y 'acp1.c'."
    exit 1
fi

# 4. PREPARACIÓN DE ENTORNO
OUT_DIR="resultados_local"
if [ -d "$OUT_DIR" ]; then
    echo "   🧹 Limpiando carpeta $OUT_DIR antigua..."
    rm -f $OUT_DIR/*.txt
else
    mkdir -p $OUT_DIR
fi

# 5. EJECUCIÓN DEL BUCLE
echo "=== 3. EJECUTANDO EXPERIMENTOS ==="
echo "   Nota: Esto tomará más tiempo que el test rápido."
echo "   Se realizarán 5 repeticiones por configuración (en HPC haremos 10)."

# Usamos 5 repeticiones para no estar esperando mucho en local, 
# pero suficientes para que el script de Python funcione bien.
TOTAL_REPS=5

for rep in $(seq 1 $TOTAL_REPS); do
    echo -ne "   🔄 Progreso: Repetición $rep de $TOTAL_REPS...\r"
    
    for D in $D_VALS; do
        for L in $L_VALS; do
            # Ejecutamos el binario local y guardamos en la carpeta local
            # El formato de nombre de archivo es IDÉNTICO al del cluster
            ./acp1_local $D $L >> $OUT_DIR/D${D}_L${L}.txt
        done
    done
done

echo -e "\n=== ✅ FINALIZADO ==="
echo "   Los datos están en la carpeta: '$OUT_DIR/'"
echo "   Ahora puedes probar tu script de Python así:"
echo "   python3 analisis.py" 
echo "   (Asegúrate de cambiar en el python: procesar_directorio('./$OUT_DIR'))"

# Limpieza del binario temporal
rm acp1_local
