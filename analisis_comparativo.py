"""
analisis_comparativo.py
Genera gráficas comparativas entre los tres experimentos:
  1. Double + acceso indirecto  (resultados/)
  2. Int    + acceso indirecto  (resultados_int/)
  3. Double + acceso directo    (resultados_directo/)
"""

import os
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec


# ── Helpers ──────────────────────────────────────────────────────────────────

def media_geometrica(valores):
    a = np.array(valores)
    return a.prod() ** (1.0 / len(a))


def procesar_directorio(directorio):
    """Lee todos los .txt de un directorio y devuelve un DataFrame con
    columnas D, L, Ciclos (media geométrica de los 3 mejores valores)."""
    datos = []
    patron = re.compile(r"Ciclos:([\d\.]+)")

    for filename in os.listdir(directorio):
        if not filename.endswith(".txt"):
            continue
        partes = filename.replace(".txt", "").split("_")
        try:
            D = int(partes[0].replace("D", ""))
            L = int(partes[1].replace("L", ""))
        except (IndexError, ValueError):
            continue

        ciclos = []
        with open(os.path.join(directorio, filename), "r") as f:
            for linea in f:
                m = patron.search(linea)
                if m:
                    ciclos.append(float(m.group(1)))

        if not ciclos:
            continue

        ciclos.sort()
        top3 = ciclos[:3]
        datos.append({"D": D, "L": L, "Ciclos": media_geometrica(top3)})

    df = pd.DataFrame(datos).sort_values(["D", "L"])
    return df


# ── Carga de datos ────────────────────────────────────────────────────────────

dirs = {
    "Double indirecto": "./resultados",
    "Int indirecto":    "./resultados_int",
    "Double directo":   "./resultados_directo",
}

dfs = {}
for nombre, path in dirs.items():
    if os.path.isdir(path):
        dfs[nombre] = procesar_directorio(path)
        print(f"[OK] {nombre}: {len(dfs[nombre])} filas cargadas desde '{path}'")
    else:
        print(f"[--] {nombre}: directorio '{path}' no encontrado, se omite")

if not dfs:
    print("No se encontró ningún directorio de resultados.")
    exit(1)


# ── Parámetros de arquitectura ────────────────────────────────────────────────
S1 = 768    # líneas L1d (48 KB / 64 B)
S2 = 20480  # líneas L2  (1.25 MB / 64 B)

COLORES = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd"]
MARKERS = ["o", "s", "^", "D", "v"]


# ── Figura 1: comparación global (un panel por experimento) ──────────────────

experimentos = list(dfs.keys())
fig, axes = plt.subplots(1, len(experimentos),
                         figsize=(6 * len(experimentos), 6),
                         sharey=True)

if len(experimentos) == 1:
    axes = [axes]

for ax, nombre in zip(axes, experimentos):
    df = dfs[nombre]
    for idx, d_val in enumerate(sorted(df["D"].unique())):
        sub = df[df["D"] == d_val].sort_values("L")
        ax.plot(sub["L"], sub["Ciclos"],
                marker=MARKERS[idx % len(MARKERS)],
                color=COLORES[idx % len(COLORES)],
                label=f"D={d_val}")

    ax.axvline(S1, color="red",   linestyle="--", alpha=0.5)
    ax.axvline(S2, color="green", linestyle="--", alpha=0.5)
    ax.text(S1 * 1.05, ax.get_ylim()[1] * 0.95, "L1", color="red",   fontsize=9)
    ax.text(S2 * 1.05, ax.get_ylim()[1] * 0.95, "L2", color="green", fontsize=9)

    ax.set_xscale("log")
    ax.set_title(nombre, fontsize=11)
    ax.set_xlabel("Líneas L (escala log)")
    ax.grid(True, which="both", linestyle="-", alpha=0.3)
    ax.legend(fontsize=8)

axes[0].set_ylabel("Ciclos de CPU por acceso")
fig.suptitle("Comparativa de experimentos — FinisTerrae III (Ice Lake)",
             fontsize=13, y=1.01)
plt.tight_layout()
plt.savefig("comparativa_experimentos.png", dpi=150, bbox_inches="tight")
print("Guardada: comparativa_experimentos.png")


# ── Figura 2: comparación directa por stride (doble vs int, directo vs indirecto) ──

# Seleccionamos strides representativos: D=1 (mejor caso) y D=16 (peor caso)
for d_val in [1, 16]:
    fig2, ax2 = plt.subplots(figsize=(9, 5))

    estilos = {
        "Double indirecto": ("solid",  "o"),
        "Int indirecto":    ("dashed", "s"),
        "Double directo":   ("dotted", "^"),
    }

    for idx, (nombre, (ls, mk)) in enumerate(estilos.items()):
        if nombre not in dfs:
            continue
        sub = dfs[nombre][dfs[nombre]["D"] == d_val].sort_values("L")
        if sub.empty:
            continue
        ax2.plot(sub["L"], sub["Ciclos"],
                 linestyle=ls, marker=mk,
                 color=COLORES[idx % len(COLORES)],
                 label=nombre, linewidth=1.8)

    ax2.axvline(S1, color="red",   linestyle="--", alpha=0.4)
    ax2.axvline(S2, color="green", linestyle="--", alpha=0.4)
    ymax = ax2.get_ylim()[1]
    ax2.text(S1 * 1.05, ymax * 0.95, "L1", color="red",   fontsize=9)
    ax2.text(S2 * 1.05, ymax * 0.95, "L2", color="green", fontsize=9)

    ax2.set_xscale("log")
    ax2.set_xlabel("Líneas L (escala log)")
    ax2.set_ylabel("Ciclos de CPU por acceso")
    ax2.set_title(f"Comparativa de variantes — Stride D={d_val}", fontsize=12)
    ax2.grid(True, which="both", linestyle="-", alpha=0.3)
    ax2.legend()
    plt.tight_layout()
    nombre_fig = f"comparativa_D{d_val}.png"
    plt.savefig(nombre_fig, dpi=150, bbox_inches="tight")
    print(f"Guardada: {nombre_fig}")


# ── Tabla resumen ─────────────────────────────────────────────────────────────

print("\n=== TABLAS PIVOTADAS POR EXPERIMENTO ===\n")
for nombre, df in dfs.items():
    print(f"--- {nombre} ---")
    pivot = df.pivot(index="L", columns="D", values="Ciclos")
    print(pivot.to_string(float_format="{:.3f}".format))
    print()