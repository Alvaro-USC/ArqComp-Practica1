---

# Estudio de la Jerarquía de Memoria y Localidad (FinisTerrae III)

Este repositorio contiene el código fuente, scripts de automatización y documentación para la **Práctica 1 de Arquitectura de Computadores**. El objetivo es caracterizar experimentalmente el comportamiento de la jerarquía de memoria (Caché L1, L2 y RAM) en un entorno de computación de alto rendimiento (HPC) utilizando el supercomputador **FinisTerrae III** del CESGA.

---

## 📋 Descripción de la Práctica

El proyecto analiza cómo los patrones de acceso a memoria afectan el rendimiento de un programa. Se mide el **coste en ciclos de CPU** para acceder a elementos de un vector, variando dos parámetros clave:

1. **Tamaño del conjunto de datos ():** Determina si los datos caben en **L1**, **L2** o residen en **RAM**.
2. **Patrón de acceso ( o *Stride*):** Controla la **localidad espacial**.
* : Acceso secuencial (máxima localidad).
* : Acceso disperso (mínima localidad).



### Objetivos Técnicos

* Medir la latencia de acceso a memoria en ciclos de reloj.
* Identificar las "fronteras" físicas entre niveles de caché (L1 vs L2 vs RAM).
* Evaluar la eficacia del *Hardware Prefetcher* en procesadores modernos (Intel Ice Lake).
* Analizar el impacto de la localidad espacial y temporal.

---

## ⚙️ Arquitectura del Sistema (FinisTerrae III)

Los experimentos están calibrados específicamente para el nodo de computación basado en **Ice Lake** del CESGA:

| Componente | Especificación |
| --- | --- |
| **Procesador** | Intel Xeon Platinum 8352Y (Ice Lake) |
| **Frecuencia** | 2.20 GHz (Base) - 3.40 GHz (Turbo) |
| **Caché L1d** | **48 KB** por núcleo (12-way, Privada) |
| **Caché L2** | **1.25 MB** por núcleo (20-way, Privada) |
| **Caché L3** | 48 MB (Compartida) |
| **RAM** | 256 GB DDR4 |
| **Línea de Caché (CLS)** | 64 Bytes |

> **Nota:** Los scripts están configurados con `S1=768` líneas (L1) y `S2=20480` líneas (L2) para coincidir con esta arquitectura.

---

## 📂 Estructura del Repositorio

A continuación se detalla el propósito de cada archivo:

### 1. Código Fuente

* **`acp1.c`**: Programa principal en C.
* Implementa un bucle de reducción (`suma += A[ind[i]]`) con acceso indirecto para evitar optimizaciones del compilador.
* Usa `aligned_alloc` para alinear los datos a 64 bytes (inicio de línea de caché).
* Realiza 10 repeticiones internas para estabilizar la medición.


* **`counter.h`**: Librería de medición de bajo nivel.
* Utiliza la instrucción ensamblador `rdtsc` para leer el contador de ciclos de la CPU directamente.
* Proporciona las funciones `start_counter()` y `get_counter()` para obtener mediciones precisas sin el *overhead* del sistema operativo.



### 2. Scripts de Ejecución

* **`script_experimentos.sh`**: Script maestro para el planificador de trabajos **Slurm**.
* Solicita un nodo exclusivo con **1 solo core** (`-c 1`) para evitar migraciones de procesos y ruido en la caché.
* Compila el código con `-O0` (sin optimizaciones).
* Ejecuta la matriz completa de experimentos: 5 Strides ()  7 Tamaños ().
* Genera los archivos de salida en la carpeta `resultados/`.


* **`test_local.sh`**: Script de prueba para desarrollo local.
* Detecta automáticamente la caché L1 y L2 de tu PC personal.
* Ejecuta una versión reducida ("mini-experimento") para verificar que el código compila y no tiene errores de memoria antes de enviarlo al supercomputador.



### 3. Documentación y Datos

* **`valores_cache.txt`**: Salida cruda de `lscpu --caches` y `getconf -a` del nodo de cómputo, usada para calcular los parámetros  y .
* **`valores_experimentales.md`**: Documento técnico con las tablas calculadas de  (elementos a sumar) para cada combinación de  y , basado en la fórmula .
* **`informe_practica_cache.md`**: Borrador del informe final con la metodología, gráficas esperadas y análisis teórico.
* **`LICENSE`**: Licencia de uso del código (MIT/GPL).

---

## 🚀 Guía de Uso

### 1. Prueba Local (En tu PC)

Antes de gastar horas de cómputo en el cluster, verifica que todo funciona:

```bash
chmod +x test_local.sh
./test_local.sh

```

*Si ves una tabla con valores de ciclos, el código está listo.*

### 2. Ejecución en FinisTerrae III

1. Conéctate al cluster vía SSH.
2. Sube los archivos (`acp1.c`, `counter.h`, `script_experimentos.sh`).
3. Envía el trabajo a la cola:
```bash
sbatch script_experimentos.sh

```


4. Verifica el estado:
```bash
squeue -u tu_usuario

```



### 3. Procesamiento de Resultados

Una vez finalizado, tendrás una carpeta `resultados/` con archivos `.txt`. Para generar la gráfica:

1. Extrae los ciclos por acceso de cada archivo.
2. Calcula la media geométrica de las 3 mejores mediciones (menor tiempo) de las 10 repeticiones.
3. Grafica **Ciclos/Acceso (Eje Y)** vs **Líneas  (Eje X)**.

---

## 📊 Resultados Esperados

La gráfica resultante debería mostrar una forma de "escalera":

1. **Zona L1 ():** Rendimiento máximo, ~4-5 ciclos.
2. **Zona L2 ():** Rendimiento medio, ~14 ciclos.
3. **Zona RAM ():** Caída de rendimiento.
* Con  (secuencial): El *prefetcher* ocultará latencia (~20-50 ciclos).
* Con  (aleatorio): Latencia pura de RAM (>150 ciclos).



---

## 📝 Autoría y Referencias

**Asignatura:** Arquitectura de Computadores
**Plataforma:** CESGA (Centro de Supercomputación de Galicia)

*Este trabajo sigue la metodología para la caracterización de jerarquías de memoria descrita en "Computer Architecture: A Quantitative Approach" (Hennessy & Patterson).*
