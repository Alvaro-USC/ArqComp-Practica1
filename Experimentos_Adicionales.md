# Experimentos Adicionales
## Anexo al Informe de Práctica 1 — Arquitectura de Computadores

---

## Experimento Adicional 1: Array de tipo `int` vs. `double`

### Motivación

El tipo `double` ocupa 8 bytes, por lo que en una línea de caché de 64 bytes caben **8 elementos**. El tipo `int` ocupa 4 bytes, por lo que caben **16 elementos** por línea. Usando los mismos valores de L (número de líneas referenciadas), el programa con `int` accede al doble de elementos (R es el doble), pero toca exactamente el mismo número de líneas de caché. La comparación permite aislar el efecto del tamaño del elemento sobre la latencia medida.

### Diferencias en el código

| Aspecto | `acp1.c` (double) | `acp1_int.c` (int) |
|:-------:|:-----------------:|:------------------:|
| Tipo de `A[]` | `double` (8 B) | `int` (4 B) |
| Elementos por línea de caché | 8 | 16 |
| R para mismo L y D | R = L×64 / (D×8) | R = L×64 / (D×4) = **2×R_double** |
| Acumulador `suma` | `double` | `int` |
| Riesgo de overflow | Ninguno | Controlado: valores en [-32767, 32767] |

### Resultado esperado

Dado que L (número de líneas) es el mismo en ambos experimentos, la presión sobre la jerarquía de caché es idéntica: se referencian exactamente las mismas líneas. Por tanto, la **latencia en ciclos/acceso debería ser similar** entre ambas versiones para los mismos valores de D y L.

Las diferencias, si las hay, provendrán de:

- **Operaciones de suma**: la suma de enteros (`ADD`) es ligeramente más barata en la ALU que la de punto flotante (`FADD`), lo que podría reducir marginalmente los ciclos/acceso en la versión `int`, especialmente en la zona L1 donde el cuello de botella es la computación y no la memoria.
- **Ancho de banda efectivo**: con `int`, el mismo número de bytes transportados contiene el doble de elementos útiles. Si el sistema está limitado por ancho de banda de memoria (zona RAM, strides grandes), la versión `int` podría mostrar una ligera ventaja.
- **Prefetcher**: el patrón de acceso a nivel de líneas es idéntico, por lo que el comportamiento del prefetcher no debería cambiar.

---

## Experimento Adicional 2: Acceso directo `A[i*D]` vs. indirecto `A[ind[i]]`

### Motivación

En el experimento base, los elementos de `A[]` se acceden de forma indirecta a través de un vector de enteros `ind[]`: `A[ind[i]]`. Esto introduce una **dependencia de datos en cadena**: para calcular la dirección de `A[]` hay que leer primero `ind[i]`, lo que genera dos accesos a memoria por iteración en lugar de uno. El experimento directo elimina esta dependencia.

### Diferencias en el código

| Aspecto | `acp1.c` (indirecto) | `acp1_directo.c` (directo) |
|:-------:|:--------------------:|:--------------------------:|
| Acceso al array | `A[ind[i]]` | `A[i * D]` |
| Vector `ind[]` | Sí, ocupa memoria y caché | No existe |
| Accesos por iteración | 2 (leer `ind[i]` + leer `A[]`) | 1 (leer `A[]` directamente) |
| Dependencia de datos | Encadenada (gather) | Simple (stride fijo) |
| Patrón para el prefetcher | Irregular aparente | Stride perfectamente regular |

### Impacto en el footprint de caché

El vector `ind[]` tiene R elementos de tipo `int` (4 bytes). Para L=163840 y D=1, R = 163840 × 8 = 1.310.720 elementos → **~5 MB solo para `ind[]`**. Este vector consume líneas de caché que compiten con `A[]`, reduciendo la efectividad de la caché para los datos relevantes.

En la versión directa, todo el espacio de caché disponible se dedica exclusivamente a `A[]`.

### Resultado esperado

**Zona L1 y L2 (L pequeño):** La diferencia debería ser mínima. Con pocos datos, tanto `A[]` como `ind[]` caben en caché y la penalización del acceso doble es despreciable.

**Zona RAM (L grande, especialmente D=1):** Aquí se esperan las diferencias más notables:

- Con acceso **directo**, el patrón `A[0], A[D], A[2D]...` es un stride perfectamente uniforme. El prefetcher lo detecta y actúa con máxima eficiencia.
- Con acceso **indirecto**, aunque `ind[i] = i*D` también es regular, el procesador tiene que leer primero `ind[i]` antes de poder calcular la dirección de `A[]`. Esta dependencia serializa los accesos y puede reducir la profundidad efectiva del prefetch.
- Adicionalmente, `ind[]` compite con `A[]` por el espacio en caché. Con L=163840 y D=1, `ind[]` ocupa ~5 MB, lo que desborda completamente L2 (1.25 MB) y fuerza accesos a L3 o RAM para leer los propios índices.

Se espera que **el acceso directo sea igual o más rápido que el indirecto**, con la mayor diferencia en strides pequeños (D=1, D=8) y conjuntos de datos grandes (zona RAM).

**Para strides grandes (D=64, D=128):** La diferencia será menor porque con pocos elementos (R pequeño), `ind[]` es pequeño y cabe en caché. El cuello de botella es la latencia de `A[]`, no la de los índices.

---

## Guía de ejecución en FinisTerrae III

```bash
# Subir los nuevos ficheros
scp acp1_int.c acp1_directo.c script_int.sh script_directo.sh \
    curso1523@ft3.cesga.es:~/ArqComp-Practica1/

# Conectarse
ssh curso1523@ft3.cesga.es
cd ~/ArqComp-Practica1/

# Dar permisos de ejecución
chmod +x script_int.sh script_directo.sh

# Lanzar los dos trabajos (pueden ejecutarse en paralelo)
sbatch script_int.sh
sbatch script_directo.sh

# Monitorizar
squeue -u curso1523

# Una vez terminados, descargar resultados
scp -r curso1523@ft3.cesga.es:~/ArqComp-Practica1/resultados_int/ ./
scp -r curso1523@ft3.cesga.es:~/ArqComp-Practica1/resultados_directo/ ./

# Generar gráficas comparativas (requiere resultados/, resultados_int/, resultados_directo/)
python3 analisis_comparativo.py
```

El script `analisis_comparativo.py` genera tres figuras:

- `comparativa_experimentos.png` — tres paneles en paralelo, uno por experimento
- `comparativa_D1.png` — comparación de las tres variantes para D=1 (mejor caso prefetcher)
- `comparativa_D16.png` — comparación de las tres variantes para D=16 (peor caso)

---

*FinisTerrae III (CESGA) — Intel Xeon Platinum 8352Y (Ice Lake) — gcc -O0*