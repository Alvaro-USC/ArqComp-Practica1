#!/bin/bash

# --- CONFIGURACIÓN AUTOMÁTICA ---
echo "=== DETECTANDO HARDWARE LOCAL ==="

# Intentamos obtener el tamaño de caché L1 de Datos y L2 en Bytes
# Si falla getconf, usamos valores por defecto seguros (32KB y 256KB)
L1_SIZE=$(getconf LEVEL1_DCACHE_SIZE 2>/dev/null || echo 32768)
L2_SIZE=$(getconf LEVEL2_CACHE_SIZE 2>/dev/null || echo 262144)

# Convertimos a Líneas (dividiendo por 64 bytes)
S1=$((L1_SIZE / 64))
S2=$((L2_SIZE / 64))

echo "Detectado L1d: $(($L1_SIZE/1024)) KB -> S1 = $S1 líneas"
echo "Detectado L2:  $(($L2_SIZE/1024)) KB -> S2 = $S2 líneas"
echo "-----------------------------------"

# --- COMPILACIÓN ---
echo "Compilando acp1.c..."
if gcc acp1.c -o acp1_test -O0; then
    echo "✅ Compilación exitosa."
else
    echo "❌ Error al compilar. Verifica que 'counter.h' está en la carpeta."
    exit 1
fi

# --- DEFINICIÓN DE PRUEBA RÁPIDA ---
# Elegimos 3 tamaños estratégicos para TU máquina:
# 1. Dentro de L1 (Pequeño)
TEST_L1=$((S1 / 2))
# 2. Dentro de L2 (Mediano)
TEST_L2=$((S2 * 3 / 4))
# 3. En RAM (Grande - forzamos que sea el triple de L2)
TEST_RAM=$((S2 * 3))

# Probamos solo dos Strides extremos para ver la diferencia rápido
STRIDES="1 128"
LINES="$TEST_L1 $TEST_L2 $TEST_RAM"

echo ""
echo "=== INICIANDO MINI-EXPERIMENTO LOCAL ==="
echo "Esto tardará unos segundos..."
echo "Format: D (Salto) | L (Líneas) | Zona Esperada | Ciclos/Acceso"
echo "-------------------------------------------------------------"

for L in $LINES; do
    # Determinar nombre de la zona para imprimir
    if [ "$L" -eq "$TEST_L1" ]; then ZONA="L1 (Rápido)"; fi
    if [ "$L" -eq "$TEST_L2" ]; then ZONA="L2 (Medio) "; fi
    if [ "$L" -eq "$TEST_RAM" ]; then ZONA="RAM (Lento) "; fi

    for D in $STRIDES; do
        # Ejecutamos el programa compilado
        # Filtramos la salida para ver solo la línea de datos
        OUTPUT=$(./acp1_test $D $L | grep "Ciclos:")
        
        # Extraemos solo el número de ciclos (un poco de magia bash)
        CYCLES=$(echo $OUTPUT | awk -F'Ciclos:' '{print $2}')
        
        printf "D=%-3d | L=%-6d | %s | %s\n" $D $L "$ZONA" "$CYCLES"
    done
    echo "-------------------------------------------------------------"
done

echo ""
echo "✅ Si ves valores lógicos (L1 < L2 < RAM), tu código funciona."
echo "   Nota: En tu máquina local puede haber 'ruido' (otras apps abiertas)."
echo "   Ahora puedes enviar el script definitivo al FinisTerrae."

# Limpieza
rm acp1_test
