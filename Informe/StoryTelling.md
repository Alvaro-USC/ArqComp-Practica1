
### 1. La Pista y las Estaciones de Agua (Líneas de Caché y L)

* 
**La Línea de Caché (64 bytes):** Imagina que la pista de atletismo está dividida en baldosas gigantes, y cada baldosa mide exactamente 64 metros (bytes). Cada vez que el corredor pisa una baldosa nueva, necesita agarrar una bandeja entera de agua (una línea de caché).


* 
**El parámetro L (Líneas):** Es la **longitud total de la carrera**. Si $L$ es pequeño (ej. 384 líneas), es una carrera cortita de barrio. Si $L$ es enorme (ej. 3.145.728 líneas), es una ultramaratón que cruza todo el país. El parámetro $L$ determina en qué nivel de la jerarquía (qué tan lejos) van a estar los datos.



### 2. El Equipo de Avituallamiento (Tipos de Caché: L1, L2, L3 y RAM)

El atleta es rapidísimo, pero no lleva el agua encima. Depende de su equipo, que está a diferentes distancias :

* 
**L1 (La mochila de hidratación):** Está pegada al corredor. Le caben pocos datos (48 KB), pero tarda poquísimo en beber, apenas unos **5 ciclos** (segundos en nuestro cronómetro).


* **L2 (El entrenador en bicicleta):** Va justo al lado de la pista. Tiene más agua (1.25 MB), pero el corredor tarda unos **14 ciclos** en agarrar la botella.


* **L3 (La furgoneta de apoyo):** Está aparcada fuera de la pista. Tiene mucha agua (48 MB), pero ir hasta ella frena al corredor unos **40-50 ciclos**.


* 
**RAM (El almacén de la fábrica):** Es gigantesco (256 GB), pero está en otra ciudad. Si el corredor tiene que esperar a que traigan agua de la RAM, se congela durante **150-200 ciclos**.



### 3. La Zancada y los Pasos (Stride D y R)

* 
**El Stride (D):** Es la **longitud de la zancada** del atleta.


* Si $D=1$, el corredor da pasitos muy cortos y pisa todos y cada uno de los metros de la baldosa.


* Si $D=16$ o $D=64$, el corredor va dando saltos gigantes.




* **La cantidad de elementos (R):** Es el **número total de pisadas** (accesos) que el corredor da en toda la carrera para recorrer las $L$ baldosas. Se calcula dividiendo la distancia total entre el tamaño del salto: $R = \frac{L \times \text{CLS}}{D \times \text{sizeof(tipo)}}$.


* **El Salto de Caché:** Ocurre cuando la zancada del corredor le hace cruzar la frontera de los 64 metros (bytes) y pisa una baldosa nueva. En ese milisegundo, la pista antigua desaparece y el equipo tiene que traerle una bandeja de agua nueva (nueva línea de caché).



### 4. Las Zapatillas del Corredor (Double vs Int)

* 
**Double (8 bytes):** Son unas botas pesadas. Con botas, en cada baldosa de 64 metros solo caben **8 pisadas** antes de saltar a la siguiente baldosa.


* 
**Int (4 bytes):** Son unas zapatillas de clavos ultraligeras. Ocupan la mitad de espacio, así que en la misma baldosa de 64 metros, el atleta puede dar **16 pisadas**. Como da más pasos aprovechando la misma bandeja de agua, el rendimiento mejora drásticamente (hasta un 43% más rápido en zonas lejanas).



### 5. El Cronómetro (Ciclos)

Los **ciclos** son simplemente los segundos que marca el cronómetro del entrenador. Miden el tiempo total que tardó el corredor, dividido entre el número de pasos ($R$). Si el valor es de 7 ciclos (color verde), el corredor voló. Si el valor es de 18 ciclos (color rojo), el corredor se quedó parado esperando el agua.

### 6. El Entrenador Telépata (El Prefetcher Hardware)

Aquí está el gran secreto de la carrera. El procesador tiene "prefetchers", que son como entrenadores telépatas que intentan adivinar dónde va a pisar el corredor para ponerle el agua en la mano *antes* de que la pida:

* **Cuando el corredor es predecible ($D=1$):** El entrenador telépata lee sus movimientos perfectamente. Incluso si la carrera es una ultramaratón (RAM profunda de 192 MB), el entrenador trae el agua desde el almacén con antelación y se la da en la mano. El corredor marca un tiempo de **7.2 ciclos**, como si nunca hubiera salido de la mochila L1. ¡El hardware hace trampa y oculta las distancias!


* **Cuando el corredor se vuelve errático ($D=16$):** El atleta da saltos raros. El "L2 Spatial Prefetcher" se confunde y sufre de *prefetch pollution*. En lugar de traer la botella de agua correcta, trae dos bandejas, una de ellas inútil, saturando la pista de basura. El corredor se tropieza con la basura y el tiempo se hunde hasta los **17.13 ciclos**.


**"Si tuviera que resumir la gran conclusión de esta práctica en un minuto, diría lo siguiente:**

El rendimiento de la memoria no es un límite físico fijo, sino el resultado de cómo nuestro programa dialoga con las predicciones del procesador. En nuestro experimento, la gran revelación fue descubrir que **el stride $D=1$ es completamente inmune a la jerarquía de memoria**.

Imaginemos a nuestra CPU como un corredor de élite. Cuando el atleta corre con pasos cortos y 100% predecibles ($D=1$), su 'entrenador telépata' —los cuatro prefetchers hardware trabajando en equipo— es tan brillante que oculta todas las latencias. No importa si la carrera es una ultramaratón que llega hasta los 192 MB de la RAM profunda; el tiempo se congela en unos increíbles $\sim$7.2 ciclos, idéntico a si los datos estuvieran pegados en la caché L1.

Sin embargo, si el atleta se vuelve errático y da saltos intermedios extraños, como en nuestro peor caso con $D=16$, el entrenador se confunde. El *L2 Spatial Prefetcher* entra en pánico y genera *prefetch pollution*, llenando la pista de 'bandejas de agua' inútiles que duplican el tráfico y hunden nuestro tiempo hasta los 17.13 ciclos en la RAM.

En definitiva, la jerarquía no es una escalera estricta. Gracias al hardware prefetcher, la penalización real desde la L1 hasta la RAM se atenúa en un factor de 14 veces, pero **solo** si programamos a favor de sus predicciones."

¡Perfecto! Vamos a blindar esa defensa. Primero, te explicaré el misterio técnico del salto 16 (que es una pregunta de sobresaliente), y luego pasaremos a las preguntas trampa.

### 1. El misterio de D=16: ¿Por qué el procesador se vuelve "tonto"?

Para entender por qué D=16 es el peor caso absoluto (17.13 ciclos en RAM ), hay que hacer un cálculo matemático muy sencillo y conocer una manía que tiene el procesador.

* **El tamaño de la zancada:** Usamos el tipo `double`, que ocupa 8 bytes. Si nuestro stride es D=16, la distancia física en memoria entre un dato y el siguiente es exactamente **128 bytes** (16 elementos $\times$ 8 bytes).
* **La manía del procesador:** El procesador tiene un mecanismo llamado *L2 Spatial Prefetcher* (o Adjacent Cache Line Prefetcher). Este mecanismo está diseñado con una regla estricta: siempre que pides una línea de caché de 64 bytes, él automáticamente trae la línea de 64 bytes contigua para formar un bloque de 128 bytes.



**¿Qué pasa entonces cuando corremos el programa?**
Tu programa lee el elemento en el byte 0 (Línea 1). El procesador trae la Línea 1, pero el *L2 Spatial Prefetcher* dice: "¡Te traigo también la Línea 2 por si acaso!".
Tu programa da su salto de 128 bytes y aterriza exactamente en el byte 128 (Línea 3). El procesador trae la Línea 3, y el prefetcher trae la Línea 4.

¿Ves el problema? **Tu programa salta exactamente por encima de la línea que el procesador está trayendo por iniciativa propia.** El hardware está trabajando el doble trayendo datos (Línea 2, Línea 4, Línea 6...) que tu código **jamás** va a utilizar. A esto se le llama **prefetch pollution**. No solo estás colapsando el "ancho de banda" (la carretera) de la memoria con basura, sino que esa basura está ocupando espacio en la caché L2 y L3, expulsando datos que sí podrían ser útiles. Por eso el rendimiento se desploma.

---

### 2. Preguntas Trampa (y cómo responderlas con seguridad)

Los profesores suelen hacer preguntas que parecen atacar un error de tu configuración para comprobar si copiaste el código o si realmente entiendes lo que le estás pidiendo a la máquina.

**Pregunta Trampa 1:** *"He visto en tu script de Slurm que has puesto `#SBATCH -c 64`. Estás reservando 64 cores del superordenador FinisTerrae III, pero el código C es secuencial, no tiene hilos ni MPI. ¿No es esto un desperdicio enorme de recursos para correr en un solo core?"*

* 
**Tu respuesta:** "Puede parecer un desperdicio, pero es absolutamente esencial para la fiabilidad del experimento. Al reservar el nodo completo con `-c 64`, impedimos que el sistema de colas (Slurm) meta programas de otros usuarios en el mismo nodo físico. Si otro usuario estuviera ejecutando algo, su proceso ensuciaría la memoria caché L3, que es compartida por todo el socket. Para medir latencias de memoria con precisión de ciclos, necesitamos que el nodo esté en un entorno estéril y completamente dedicado a nuestra medición."



**Pregunta Trampa 2:** *"A la hora de compilar, usaste `gcc -O0`. ¿Por qué no compilaste con `-O3` para que el programa fuera más rápido y eficiente?"*

* **Tu respuesta:** "Porque nuestro objetivo no es que el programa sea rápido, sino medir la latencia pura del hardware de memoria. Si compilamos con `-O3`, el compilador es tan inteligente que podría eliminar el bucle entero al darse cuenta de que no usamos la suma para nada vital, o podría usar vectorización avanzada (AVX) y desenrollado de bucles. Con `-O0` evitamos que el compilador enmascare o altere los accesos a memoria, garantizando que por cada iteración del bucle se hace la petición física a la memoria que queremos medir."



**Pregunta Trampa 3:** *"En las gráficas, veo que al final la latencia se aplana. ¿Cómo puedes estar seguro matemáticamente de que a partir de L=1572864 has llegado a la RAM pura y no estás simplemente en una zona muy lenta de la L3?"*

* **Tu respuesta:** "Lo sabemos por la estabilización total de los valores y el tamaño del footprint. La caché L3 del procesador Ice Lake-SP tiene exactamente 48 MB. En nuestro experimento, para L=1572864 líneas, el footprint es de 96 MB, y para L=3145728 es de 192 MB. Ambos valores desbordan masivamente la L3. Al observar los datos, vemos que entre los 96 MB y los 192 MB la diferencia de ciclos es inferior al 2%. Esta falta de degradación adicional confirma que hemos tocado el 'fondo' de la jerarquía: la memoria RAM pura, donde la latencia ya es constante independientemente del tamaño del vector."

