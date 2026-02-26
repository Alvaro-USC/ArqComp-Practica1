import os
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def media_geometrica(lista_valores):
    """Calcula la media geométrica de una lista de valores."""
    a = np.array(lista_valores)
    return a.prod()**(1.0/len(a))

def procesar_directorio(directorio):
    datos = []
    
    # Expresión regular para extraer ciclos: "Ciclos:14.5000"
    patron = re.compile(r"Ciclos:([\d\.]+)")
    
    # Recorrer archivos
    for filename in os.listdir(directorio):
        if not filename.endswith(".txt"): continue
        
        # Extraer D y L del nombre del archivo (D8_L384.txt)
        partes = filename.replace(".txt", "").split("_")
        try:
            D = int(partes[0].replace("D", ""))
            L = int(partes[1].replace("L", ""))
        except:
            continue

        ciclos = []
        with open(os.path.join(directorio, filename), 'r') as f:
            for linea in f:
                match = patron.search(linea)
                if match:
                    ciclos.append(float(match.group(1)))
        
        if not ciclos: continue

        # --- TRATAMIENTO ESTADÍSTICO ---
        # 1. Ordenar de menor a mayor (los menores son "mejores", menos ruido OS)
        ciclos.sort()
        # 2. Tomar los 3 mejores (Top 3 Best Latency)
        top_3 = ciclos[:3]
        # 3. Media geométrica
        media_geo = media_geometrica(top_3)
        
        datos.append({'D': D, 'L': L, 'Ciclos': media_geo})

    return pd.DataFrame(datos)

# --- MAIN ---
try:
    df = procesar_directorio("./resultados")
except:
    df = procesar_directorio("./resultados_local")
if df.empty:
    print("No se encontraron datos en ./resultados")
    exit()

# Ordenar para visualización
df = df.sort_values(by=['D', 'L'])

print("=== TABLA DE RESULTADOS PROCESADOS (Top 3 Geom Mean) ===")
print(df.pivot(index='L', columns='D', values='Ciclos'))

# --- GRAFICAR ---
plt.figure(figsize=(12, 7))

# Valores teóricos FinisTerrae III (para dibujar líneas verticales)
S1 = 768       # líneas L1d (48 KB / 64 B)
S2 = 20480     # líneas L2  (1.25 MB / 64 B)
S3 = 786432    # líneas L3  (48 MB / 64 B)  <-- NUEVO L3

for d_val in sorted(df['D'].unique()):
    subset = df[df['D'] == d_val]
    plt.plot(subset['L'], subset['Ciclos'], marker='o', label=f'Stride D={d_val}')

# Decoración profesional
plt.xscale('log') # Eje X logarítmico es OBLIGATORIO para jerarquía de memoria
plt.xlabel('Número de Líneas (L) - Escala Log')
plt.ylabel('Ciclos de CPU por Acceso')
plt.title('Jerarquía de Memoria: Intel Ice Lake (FinisTerrae III)')
plt.grid(True, which="both", ls="-", alpha=0.3)

# Marcar zonas de caché
plt.axvline(x=S1, color='r', linestyle='--', alpha=0.5, label='Límite L1')
plt.axvline(x=S2, color='g', linestyle='--', alpha=0.5, label='Límite L2')
plt.axvline(x=S3, color='orange', linestyle='--', alpha=0.5, label='Límite L3') 

plt.text(S1, plt.ylim()[1]*0.9, ' L1', color='r')
plt.text(S2, plt.ylim()[1]*0.9, ' L2', color='g')
plt.text(S3, plt.ylim()[1]*0.9, ' L3', color='orange') # <-- NUEVO L3

# La leyenda se llama AL FINAL para que recoja las líneas L1, L2 y L3
plt.legend()

plt.savefig('grafica_memoria.png')
print("\nGráfica guardada como 'grafica_memoria.png'")
plt.show()