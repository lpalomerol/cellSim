# Resumen introductorio sobre los estados celulares de Cell-Sim

Este documento describe brevemente los diferentes estados celulares de Cell-Sim, sus características y cómo se relacionan con la dinámica de transición entre estados celulares. Los estados celulares son representaciones abstractas de las condiciones fisiológicas y genéticas de una célula en un momento dado, y son fundamentales para entender la respuesta celular a diferentes estímulos y perturbaciones.


Lista de estados hasta el momento:
  - Baseline (BRCA1+/- / TP53+/+): Estado celular inicial con estabilidad genómica y funcionalidad normal.
  - T1 (BRCA1+/- / TP53+/-): Estado celular con inestabilidad genómica moderada, caracterizado por la pérdida de un alelo de TP53.
  - T2 (BRCA1+/- / TP53-/-): Estado celular con inestabilidad genómica más pronunciada, caracterizado por la pérdida de ambos alelos de TP53, mientras que BRCA1 permanece heterocigótico.
  - T3 (BRCA1-/- / TP53-/-): Estado celular con inestabilidad genómica severa, caracterizado por la pérdida de ambos alelos de BRCA1 y TP53, lo que conduce a una alta susceptibilidad a mutaciones y alteraciones cromosómicas.
  - Estado "Primer": Este estado representa la condición inicial que debe cumplir toda célula antes de convertirse en tumoral. Es un estado transitorio donde la célula, que aún mantiene cierta funcionalidad, está predispuesta a la transformación tumoral siempre que se cumplan ciertas condiciones tanto intrínsecas de la célula como del microambiente tumoral. Este proceso es crítico. 
  - Estado Tumoral: Estado neoplásico activo con resistencia a la apoptosis, aceleración mitótica y potencial diversificación clonal interna.

# Genes relevantes

El modelo considera dos genes relevantes: BRCA1 y TP53. La presencia o ausencia de alelos funcionales de estos genes determina la estabilidad genómica y la capacidad de la célula para reparar el ADN y regular el ciclo celular. La pérdida de función de estos genes está asociada con un aumento en la inestabilidad genómica y una mayor susceptibilidad a la transformación tumoral.

El rol de BRCA1 es crucial en la reparación del ADN y la prevención de mutaciones, mientras que TP53 actúa como un regulador clave del ciclo celular y un supresor de tumores. La interacción entre estos genes y su estado funcional determina el comportamiento celular y la respuesta a daños genéticos. El rol de ambos genes es múltiple y complementario, por lo que hay que definirlo de forma conjunta para poder entender la dinámica de transición entre estados celulares y la progresión hacia la tumorigénesis.

Cuando TP53 está presente y es funcional, la pérdida del alelo de BRCA1 implica que la célula entra en estado de apoptosis. En cambio, cuando TP53 está ausente o no funcional, la pérdida del alelo de BRCA1 implica que la célula pierde su capacidad de reparación del ADN y se vuelve más propensa a la acumulación de mutaciones, lo que aumenta el riesgo de transformación tumoral. Por lo tanto, la interacción entre BRCA1 y TP53 es fundamental para determinar el destino celular y la susceptibilidad a la tumorigénesis. Por su parte, la pérdida de los alelos de TP53 conduce a una mayor inestabilidad genómica junto con la pérdida de la capacidad de regulación por apoptosis. 

# Tasa de mutación dinámica (Parámetros μ y β)

En Cell-Sim, la probabilidad de que un gen sufra una mutación en cada ciclo celular no es una constante estática, sino una tasa dinámica modulada por la integridad de TP53 y el nivel de daño intrínseco acumulado. Siguiendo la especificación formal del modelo, cada gen conductor dispone de una tasa basal de mutación somática calibrada:

- **μ_TP53 (o μ = 0.0030 año⁻¹):** Tasa basal de mutación somática de TP53 (`tp53_rate`). Actúa biológicamente como un **interruptor fisiológico On/Off**: su inactivación bialélica apaga los checkpoints celulares de apoptosis.
- **β_BRCA1 (o β = 0.0459 año⁻¹):** Tasa basal de mutación somática del alelo restante de BRCA1 (`brca1_rate`). Actúa biológicamente como un **dial de latencia clínica**: modula la velocidad con la que el tejido colapsa su vía de recombinación homóloga.

En cada ciclo de división, el modelo evalúa si un gen muta comparando una variable aleatoria uniforme con el umbral efectivo de mutación del gen correspondiente (Umbral_mut):

```text
Umbral_mut(TP53)  = (μ_TP53  + k_inestabilidad) * D_intr
Umbral_mut(BRCA1) = (β_BRCA1 + k_inestabilidad) * D_intr
```
*(donde k_inestabilidad = 0 si TP53 es +/+, y toma una penalización positiva fija en cuanto se activa isUnstable = true por la pérdida de al menos un alelo de TP53)*.

Este umbral se compone de dos mecanismos dinámicos:

1. **Penalización aditiva por inestabilidad genómica (isUnstable):**
   - En estado Baseline (TP53+/+), el genoma se considera estable y la probabilidad de mutación parte exclusivamente de las tasas basales μ_TP53 y β_BRCA1.
   - En cuanto la célula pierde un alelo de TP53 (a partir de T1), se activa la inestabilidad genómica (isUnstable = true). En ese instante se suma la penalización k_inestabilidad a ambos genes, elevando bruscamente su susceptibilidad mutacional.

2. **Amplificación multiplicativa por daño intrínseco (D_intr):**
   - El término anterior se multiplica por el contador de daño intrínseco acumulado (D_intr). Cuanto más estrés genotóxico acumula la célula, mayor es el factor de amplificación, acelerando de forma no lineal el riesgo de mutagénesis.

### Efecto específico sobre las tasas basales de BRCA1 y TP53

Esta dinámica condiciona de manera crítica el destino y la velocidad de progresión de la célula según su estado:

* **En Baseline (BRCA1+/- y TP53+/+):**
  - Tanto TP53 como el alelo funcional restante de BRCA1 mutan únicamente a sus tasas basales mínimas (μ_TP53 y β_BRCA1).
  - Dado que TP53 está intacto (k_inestabilidad = 0) y el daño D_intr crece de forma casi imperceptible (sólo por fricción de edad), la probabilidad de sufrir mutaciones es extraordinariamente baja. El tejido puede permanecer sano y estable durante décadas.

* **En T1 (BRCA1+/- y TP53+/-) — La ruptura de la fidelidad:**
  - Al perderse el primer alelo de TP53, la penalización k_inestabilidad se suma tanto a μ_TP53 como a β_BRCA1.
  - **Sobre el alelo restante de TP53:** La tasa efectiva (μ_TP53 + k_inestabilidad) * D_intr se multiplica, haciendo muy probable el "segundo golpe" somático que conduce a la inactivación bialélica (TP53-/-).
  - **Sobre el alelo restante de BRCA1:** Su probabilidad efectiva (β_BRCA1 + k_inestabilidad) * D_intr también se dispara; si BRCA1 muta en este estado, la actividad residual de TP53 detecta el colapso de la recombinación homóloga y desencadena la muerte por apoptosis intrínseca (Ω_int).
  - **Efecto de retroalimentación:** Como en T1, T2 y T3 el daño D_intr se acumula mucho más rápido que en Baseline, el factor multiplicativo D_intr crece ciclo a ciclo, generando una aceleración progresiva de la mutagénesis.

# Acumulación de daño celular

El modelo considera **dos** indicadores principales: el daño intrínseco o macromolecular (D_intr, identificado en código como D1) y el daño inmunológico o inmunosupresión (D_inmune, identificado en código como D2). Ambos reflejan la degradación acumulativa de la célula a lo largo del tiempo y son fundamentales para gobernar las transiciones entre estados y la eventual transformación tumoral. Trabajamos con dos parámetros base de incremento de daño: 

 - Δ_low: Refleja la acumulación de daño en células con al menos un alelo funcional de BRCA1 (degradación celular basal/controlada).  
 - Δ_high: Refleja la acumulación de daño tras la pérdida completa de alelos funcionales (degradación celular acelerada).

En código, las ecuaciones de incremento por ciclo son las siguientes:

```
        double delta_tp53 = 0.0;
        if (tp53 == "+/-") {
            delta_tp53 = low_delta_;
        } else if (tp53 == "-/-") {
            delta_tp53 = high_delta_;
        }
        double delta_brca1 = 0.0;
        if (brca1 == "+/-") {
            delta_brca1 = low_delta_;
        } else if (brca1 == "-/-") {
            delta_brca1 = 2 * high_delta_; // 2 veces Δ_high por el efecto pronunciado de BRCA1-/- en evasión inmune
        }

        // D1 (Daño ADN intrínseco) = Δ(TP53) + factor de edad
        // D2 (Inmunosupresión)     = Δ(TP53) + Δ(BRCA1) + factor de edad
        double age_factor = age * 0.00001;
        double delta_d1 = delta_tp53  + age_factor;
        double delta_d2 = delta_tp53 + delta_brca1 + age_factor;
```

Ambos parámetros se definen dinámicamente en la calibración. Para este análisis asumiremos que Δ_low es 0.107 y que Δ_high es el doble, 0.214.

### Comportamiento de D_intr (D1)
El daño intrínseco D_intr es un contador acumulativo de estrés macromolecular y genotóxico. En el modelo, su incremento por ciclo depende exclusivamente de la edad y del estado de TP53 (ya que BRCA1 no contribuye directamente a D_intr, al provocar la apoptosis si muta en presencia de TP53 funcional):

  - BRCA1+/- y TP53+/+: La acumulación es mínima y controlada, reflejando reparación eficiente: Δ_D_intr = edad * 10^-5.
  - BRCA1+/- y TP53+/-: Acumulación moderada por pérdida de un alelo de TP53: Δ_D_intr = 0.107 + edad * 10^-5.
  - BRCA1+/- y TP53-/-: Acumulación acelerada por pérdida total de TP53: Δ_D_intr = 0.214 + edad * 10^-5.
  - BRCA1-/- y TP53-/-: La tasa de daño intrínseco se mantiene en Δ_D_intr = 0.214 + edad * 10^-5 (el impacto adicional de la pérdida de BRCA1 se proyecta sobre D_inmune).

### Comportamiento de D_inmune (D2)
El indicador D_inmune cuantifica la pérdida de control por parte del microambiente y la capacidad de la célula para evadir la vigilancia inmunológica tisular. A diferencia de D_intr, en D_inmune participan activamente tanto TP53 como BRCA1:

  - BRCA1+/- y TP53+/+: Acumulación basal ligada a la heterocigosidad germinal de BRCA1: Δ_D_inmune = 0.107 + edad * 10^-5.
  - BRCA1+/- y TP53+/-: Acumulación intermedia por la combinación de TP53+/- y BRCA1+/-: Δ_D_inmune = 0.107 + 0.107 + edad * 10^-5 = 0.214 + edad * 10^-5.
  - BRCA1+/- y TP53-/-: Acumulación severa por inactivación bialélica de TP53: Δ_D_inmune = 0.214 + 0.107 + edad * 10^-5 = 0.321 + edad * 10^-5.
  - BRCA1-/- y TP53-/-: Acumulación máxima del modelo debido a la doble deleción de BRCA1 (2 * Δ_high) sumada a TP53-/- (Δ_high): Δ_D_inmune = 2 * 0.214 + 0.214 + edad * 10^-5 = 0.642 + edad * 10^-5.

### Umbrales críticos del sistema (umbrales de decisión θ_intr y θ_inmune)

Para gobernar el destino biológico de las células pre-malignas, el modelo define dos constantes umbral calibradas:

1. **umbral_primer (θ_intr = 2.0):** Barrera pre-neoplásica de daño intrínseco (`d1_primer_threshold_` en código). Cuando una célula desprotegida (TP53-/-) acumula suficiente estrés genotóxico para superar este valor (D_intr > umbral_primer), abandona su estado previo e ingresa forzosamente en el estado transitorio "Primer".
2. **umbral_inmune (θ_inmune = 5.0):** Barrera de escape frente a la vigilancia inmune tisular (`d2_apoptosis_threshold_` en código). Determina la resolución binaria de la célula en Primer: si D_inmune > umbral_inmune, la célula evade la apoptosis y se inmortaliza; si D_inmune <= umbral_inmune, la vigilancia tisular la destruye de inmediato mediante apoptosis extrínseca (Ω_ext).

*Ratio de barrera (θ_inmune / θ_intr = 5.0 / 2.0 = 2.5):* Esta relación formaliza que franquear la respuesta inmune del tejido requiere 2.5 veces más acumulación de degradación biológica que alcanzar la inestabilidad celular interna necesaria para ser pre-tumoral.


# Descripción de los estados celulares

# Baseline (BRCA1+/- / TP53+/+)

Este es el estado celular inicial, caracterizado por la estabilidad genómica y la funcionalidad normal. En este estado, la célula mantiene un equilibrio homeostático y es capaz de responder adecuadamente a señales externas e internas. La presencia de un alelo funcional de BRCA1 y TP53 permite la reparación eficiente del ADN y la regulación del ciclo celular. No obstante, estas células acumulan un daño genético reducido pero constante a lo largo del tiempo.  

En cuanto a la dinámica mutacional, dado que ambos alelos de TP53 están intactos, el genoma permanece estable (isUnstable = false y k_inestabilidad = 0). En consecuencia, tanto el alelo funcional restante de BRCA1 como los dos alelos de TP53 están sujetos únicamente a sus tasas basales espontáneas mínimas de mutación. La probabilidad de adquirir una nueva mutación en este estado es muy baja y sólo aumenta de forma casi imperceptible con la edad mediante el incremento lento de D_intr.

Este estado celular evoluciona según los siguientes criterios:
- Si la célula mantiene ambos genes funcionales de BRCA1 y TP53, el daño acumulado es mínimo y la célula permanece en el estado Baseline.
- Si la célula pierde un alelo de TP53, transiciona al estado T1, reflejando una inestabilidad genómica moderada.
- Si la célula pierde el alelo de BRCA1 mientras TP53 sigue funcional, la célula entra en apoptosis intrínseca (Ω_int) y no progresa a estados más inestables. En otras palabras, muere. 

Esta célula también se puede dividir, y lo hace a una tasa fisiológica estándar (d_basal = 0.0010).

# T1 (BRCA1+/- / TP53+/-)

Este estado representa el primer paso hacia la desestabilización genómica de la célula. Se produce cuando una célula en estado Baseline sufre la pérdida de un alelo de TP53 (haploinsuficiencia funcional). Aunque la célula sigue siendo viable y retiene un alelo funcional de cada gen que le permite mantener el control básico del ciclo celular, los mecanismos de detección y reparación de daños comienzan a deteriorarse. En consecuencia, el ritmo de acumulación de daño tanto intrínseco (D_intr) como inmunológico (D_inmune) se acelera respecto al estado inicial.

En el plano mutacional, la pérdida del primer alelo de TP53 activa la condición de inestabilidad genómica (isUnstable = true). Esto provoca que se sume de inmediato la penalización k_inestabilidad a las probabilidades de mutación de todos los genes:
- Para el alelo restante de TP53, su tasa efectiva de mutación aumenta significativamente, haciendo mucho más probable el "segundo golpe" que conducirá a la pérdida total de función (TP53-/-).
- Para el alelo restante de BRCA1, la probabilidad de perderse también se eleva; si esto llega a suceder en este estado, la célula activará la apoptosis intrínseca (Ω_int) debido a la actividad residual de TP53.
Además, dado que D_intr se acumula a un ritmo superior en T1, el factor multiplicador del umbral mutacional crece más rápido ciclo a ciclo, generando una aceleración progresiva en la probabilidad de sufrir mutaciones.

Este estado celular evoluciona según los siguientes criterios:
- Si la célula no sufre nuevas mutaciones, continúa acumulando daño celular a un ritmo moderado y permanece en el estado T1.
- Si la célula pierde el segundo alelo de TP53, transiciona al estado T2 (BRCA1+/- / TP53-/-), perdiendo por completo la protección del guardián del genoma y entrando en una fase de inestabilidad crítica.
- Si la célula pierde el alelo restante de BRCA1 mientras mantiene un alelo funcional de TP53, dicho alelo aún es capaz de detectar el daño severo en el ADN y conduce a la célula a la apoptosis intrínseca (Ω_int), provocando su muerte y evitando la progresión tumoral.

Al igual que en Baseline, las células en estado T1 conservan su capacidad de dividirse a una tasa fisiológica estándar (d_basal = 0.0010), transfiriendo su genotipo y los daños acumulados a la descendencia.

# T2 (BRCA1+/- / TP53-/-)

Este estado representa una fase de desprotección genómica crítica. Se alcanza cuando una célula en T1 sufre la pérdida del segundo alelo de TP53 (segundo golpe o pérdida de heterocigosidad). Al quedar completamente inactivado el "guardián del genoma", la célula pierde tanto los puntos de control del ciclo celular dependientes de TP53 como la capacidad de inducir apoptosis ante aberraciones genéticas severas. Mientras tanto, BRCA1 permanece en heterocigosis (+/-), proporcionando todavía una capacidad residual de reparación por recombinación homóloga.

En el plano mutacional y cinético:
- La inestabilidad genómica es plena (isUnstable = true, con k_inestabilidad activo). 
- El daño intrínseco (D_intr) se incrementa de forma acelerada debido a la ausencia total de TP53 (sumando Δ_high = 0.214 en cada ciclo), lo que dispara el factor multiplicativo del umbral de mutación.
- El daño inmunológico (D_inmune) acumula simultáneamente el impacto de la pérdida de ambos alelos de TP53 y el alelo funcional ausente de BRCA1 (0.214 + 0.107 = 0.321 por ciclo más la fricción de edad), acercando a la célula al umbral de evasión inmune.
- Crucialmente, la pérdida del alelo restante de BRCA1 deja de ser letal: la célula ahora es genómicamente permisiva ante la deficiencia homóloga de BRCA1.

Este estado celular evoluciona según los siguientes criterios:
- Si la célula continúa acumulando daño intrínseco hasta que D_intr supera el umbral crítico (D_intr > umbral_primer) antes de alterar BRCA1, transiciona directamente al estado "Primer" como pre-tumoral (manteniendo BRCA1+/-).
- Si la célula pierde el segundo alelo de BRCA1, transiciona al estado T3 (BRCA1-/- / TP53-/-). A diferencia de lo que ocurría en Baseline o T1, la célula no entra en apoptosis porque carece de TP53 funcional para ejecutarla, sobreviviendo con inestabilidad genómica severa.
- Si no sufre nuevas mutaciones y no ha superado el umbral de Primer, la célula permanece en T2 acumulando daño a un ritmo rápido.

Al igual que en los estados previos, la célula en T2 sigue dividiéndose a la tasa fisiológica estándar (d_basal = 0.0010) y transmitiendo su genotipo y los niveles acumulados de daño a sus células hijas.

# T3 (BRCA1-/- / TP53-/-)

Este estado representa la máxima inestabilidad genómica y degradación celular del modelo pre-neoplásico. Se alcanza exclusivamente desde T2, cuando una célula que ya ha perdido ambos alelos de TP53 sufre la pérdida del alelo restante de BRCA1. Al carecer totalmente de BRCA1, la célula pierde la capacidad de reparar roturas de doble cadena de ADN por recombinación homóloga; no obstante, al no contar con TP53 funcional para censurar este daño catastrófico, la célula sobrevive y continúa proliferando, acumulando aberraciones cromosómicas masivas.

En el plano mutacional y cinético:
- La inestabilidad genómica es total (isUnstable = true), manteniendo la penalización k_inestabilidad en todos los genes.
- El daño intrínseco (D_intr) continúa acumulándose a la tasa acelerada de TP53-/- (0.214 por ciclo más la fricción de edad), ya que el efecto directo de la inactivación de BRCA1 se canaliza hacia el escape inmune.
- El daño inmunológico (D_inmune) alcanza su **tasa máxima de acumulación** en el modelo (sumando 2 * Δ_high de BRCA1 + Δ_high de TP53 = 0.428 + 0.214 = 0.642 por ciclo). Esta aceleración drástica simula la profunda desregulación antigénica y la rápida adquisición de mecanismos de evasión frente al microambiente inmunitario.

Este estado celular evoluciona según los siguientes criterios:
- Con ambos genes conductores ya completamente inactivados (doble nulo), el destino principal de la célula es el avance inexorable hacia el estado "Primer": en cuanto D_intr supera el umbral crítico (D_intr > umbral_primer), la célula transiciona inmediatamente a Primer.
- Debido a la rápida acumulación de D_inmune en T3, la célula suele alcanzar el umbral de escape inmune mucho antes de llegar a Primer o poco después, lo que incrementa drásticamente sus probabilidades de eludir con éxito la vigilancia inmune tisular y consumar la transformación tumoral.
- Si aún no ha cruzado el umbral D_intr > umbral_primer, la célula permanece en T3 acumulando daño a velocidad máxima.

La célula en estado T3 continúa dividiéndose a la tasa fisiológica estándar (d_basal = 0.0010), heredando la progenie clonal el genotipo doble deficiente y su elevada carga de daño D_intr y D_inmune.

# Estado Primer (Pre-tumoral transitorio)

El estado "Primer" representa el cuello de botella evolutivo más crítico del modelo: es la condición transitoria obligatoria que debe alcanzar toda célula antes de convertirse en tumoral. No se alcanza mediante una mutación puntual directa, sino por la combinación de una base genética desprotegida (TP53-/-) y la superación de un umbral cuantitativo de daño genotóxico: el daño intrínseco acumulado debe superar el umbral crítico (D_intr > umbral_primer). A este estado pueden llegar tanto células procedentes de T2 (BRCA1+/-) como de T3 (BRCA1-/-).

A diferencia de los estados anteriores, Primer es un estado de **un solo ciclo**: la célula no permanece en él dividiéndose ni acumulando daño a lo largo del tiempo. En el mismo ciclo en que D_intr supera umbral_primer, la célula se vuelve plenamente visible para la vigilancia del tejido y se enfrenta a una resolución binaria inmediata gobernada por su nivel de daño inmunológico (D_inmune):

1. **Escape inmune y transformación tumoral (D_inmune > umbral_inmune):**
   - Si la célula ha acumulado suficiente daño inmunológico para eludir la respuesta del tejido, bloquea la acción inmunitaria.
   - En ese mismo ciclo se activa el programa tumoral (develop_neoplasm), transicionando definitivamente al estado **Tumoral** y adquiriendo resistencia a la apoptosis.

2. **Eliminación por inmunoedición tisular (D_inmune <= umbral_inmune):**
   - Si el daño genético D_intr cruzó el umbral crítico antes de que la célula desarrollara suficiente capacidad de evasión (D_inmune insuficiente), el sistema inmunitario del tejido detecta a la célula pre-maligna y la elimina en ese mismo ciclo mediante apoptosis extrínseca (Ω_ext, extrinsic_apoptosis_immune_surveillance).

### Comportamiento según el origen (T2 vs T3)
- **Células procedentes de T3:** Como acumulan D_inmune a la velocidad máxima (0.642 por ciclo), es sumamente probable que crucen el umbral de escape inmune antes de llegar a Primer. Por ello, la gran mayoría consuma con éxito la transformación tumoral.
- **Células procedentes de T2:** Al acumular D_inmune de forma más lenta (0.321 por ciclo), existe una ventana crítica donde pueden superar D_intr > umbral_primer sin haber alcanzado suficiente D_inmune, siendo destruidas eficazmente por la vigilancia tisular.

> **Decisión de diseño y justificación metodológica (Resolución en un solo ciclo):**  
> En la fisiopatología real del tejido, una lesión pre-maligna (como atipias epiteliales, carcinomas ductales in situ o microfocos displásicos) puede perdurar durante un tiempo indeterminado en una ventana de latencia antes de su resolución definitiva (escape neoplásico o eliminación inmunitaria).  
> Sin embargo, permitir que la célula residiera de forma prolongada en el estado Primer habría exigido introducir parámetros adicionales de tiempo de estancia, tasas de transición secundaria y reglas de acumulación intermedias que habrían sobreparametrizado innecesariamente la simulación.  
> Conforme al principio de parsimonia (Navaja de Ockham) y con el objetivo deliberado de no complicar innecesariamente el modelo con grados de libertad difíciles de contrastar clínicamente, se adoptó la decisión de formalizar **Primer como un estado consecutivo e instantáneo**: una compuerta estocástica binaria de un solo ciclo que evalúa inmediatamente el balance entre agresión celular y vigilancia tisular.

# Estado Tumoral (Neoplásico)

La transformación neoplásica en Cell-Sim no constituye un estado final estático o monolítico, sino un ecosistema biológico dinámico formado por un conjunto de subestados con su propia evolución tumoral interna. Aunque a nivel computacional la condición neoplásica se formaliza mediante una reprogramación fenotípica unificada, la evolución dentro del tumor emerge de manera orgánica a partir de dos motores concurrentes:

1. **La degradación continua y acelerada:** Los contadores de daño celular intrínseco (D_intr) e inmunológico (D_inmune) continúan incrementándose tras la transformación, profundizando el estrés genotóxico de la célula.
2. **La mutabilidad somática activa:** La dinámica nuclear permanece activa en cada división celular. En aquellos tumores originados sin pérdida previa del segundo alelo de BRCA1 (BRCA1+/-), existe una probabilidad constante y muy elevada de que este gen sufra una mutación somática post-transformación.

Gracias a esta interacción, el modelo es capaz de simular **heterogeneidad tumoral interna**, permitiendo la coexistencia y competencia de distintos subclones con perfiles genotípicos y capacidades de adaptación divergentes dentro de una misma masa neoplásica.

> **Nota sobre la acumulación de inestabilidad y saturación de umbrales:**  
> Tras la transformación tumoral, la tasa de incremento de daño sigue las mismas reglas matemáticas que en los estadios previos: en cada ciclo de división se suman los incrementos fijos correspondientes a su genotipo (Δ_tp53 y Δ_brca1) junto a la fricción biológica del envejecimiento (edad * 10^-5).  
> No obstante, en este estadio tumoral, el impacto funcional de estos incrementos adicionales es **prácticamente indiferente**:
> - La célula ya superó irreversiblemente el umbral pre-neoplásico (D_intr > umbral_primer), habiendo consumado la transformación.
> - La célula ya superó el umbral de escape inmune (D_inmune > umbral_inmune) y adquirió la inmortalidad celular permanente (has_evaded_apoptosis_ = true), por lo que acumular mayor D_inmune no altera su viabilidad.
> El único efecto biológico residual de que D_intr continúe creciendo en el tumor es que mantiene o amplifica ligeramente el umbral de mutación, garantizando que si el tumor retiene un alelo funcional de BRCA1, la presión mutagénica sobre dicho alelo permanezca en su cota más agresiva.

---

### Las Tres Fases Dinámicas del Desarrollo Tumoral

Para estructurar la trayectoria vital del clon neoplásico y su progenie, el desarrollo tumoral se articula en tres fases dinámicas. Conviene señalar que **la distinción entre la Fase I y la Fase II es una separación analítico-conceptual**: en la implementación formal del modelo `cellSim`, ambas ocurren de manera **simultánea e indisociable** en el mismo ciclo temporal en que se supera la compuerta de Primer (`develop_neoplasm()` blinda a la célula frente a la apoptosis al tiempo que desacopla inmediatamente la tasa mitótica a $d_{neo}$). La Fase III, en cambio, representa la evolución temporal posterior de la colonia proliferante.

#### Fase I: Adquisición de la Identidad Neoplásica y Evasión Inmune Inicial (Conceptual)
Es el momento molecular fundacional de la neoplasia, resultante de la resolución exitosa del estado transitorio "Primer".
- **Consumación del escape:** La célula pre-tumoral supera el punto de control tisular gracias a un nivel de daño inmunológico superior al umbral de apoptosis ($D_{inmune} > \theta_{inmune} = 5.0$), neutralizando la respuesta citotóxica del microambiente tisular.
- **Reprogramación fenotípica:** En ese instante se ejecuta el programa de transformación neoplásica (`develop_neoplasm()`). La célula adquiere una resistencia irreversible a la muerte celular (`has_evaded_apoptosis_ = true`): los puntos de control del ciclo celular quedan definitivamente anulados y la célula se vuelve inmortal tanto frente a daños genotóxicos internos como frente a señales apoptóticas extrínsecas del tejido circundante.

#### Fase II: Desacoplamiento Proliferativo y Expansión Clonal ("Big Bang") (Cinética)
Coincidente en el tiempo con la consolidación de la inmortalidad celular, la célula tumoral altera radicalmente su cinética mitótica frente a las células tisulares circundantes.
- **Ruptura del control homeostático:** La célula se desacopla del ritmo mitótico basal del tejido sano, el cual está estrictamente regulado por la inhibición por contacto y las necesidades fisiológicas de recambio celular.
- **Mitosis clonal acelerada:** Se activa una tasa de división clonal específica para células neoplásicas ($d_{neo} = 0.1584$, correspondiente a `neoplastic_division_rate_`, frente al ritmo basal $d_{basal} = 0.0010$). Este crecimiento autónomo desencadena una rápida colonización del espacio tisular, dando origen a la masa tumoral primaria y asegurando que todas las células hijas hereden automáticamente el fenotipo neoplásico y la resistencia a la muerte.

> **Nota Metodológica de Simultaneidad:** En términos operacionales dentro de la simulación, Fase I y Fase II forman una única compuerta de escape tumoral unificada. No existe latencia ni estado biológico intermedio entre adquirir la identidad tumoral y asumir la cinética mitótica acelerada.

#### Fase III: Hipermutación, Ramificación Somática y Heterogeneidad Clonal Intra-tumoral
Durante la expansión de la colonia neoplásica, la ausencia total de TP53 y el elevado daño intrínseco (D_intr >> umbral_primer) generan un microambiente de hipermutabilidad permanente donde el umbral de mutación para todos los genes es extremadamente bajo.
- **Ramificación del linaje BRCA1:** 
  - Si el clon fundador era heterocigoto (BRCA1+/-), sus células hijas mantienen inicialmente una capacidad residual de reparación del ADN.
  - Con cada ciclo mitótico bajo hipermutabilidad, surge una probabilidad real de que células individuales sufran el "segundo golpe" somático en BRCA1, transicionando a BRCA1-/- sin ningún riesgo de apoptosis (pues carecen de TP53 y son refractarias a la muerte).
- **Emergencia de subclones coexistentes:** Dentro de la misma población neoplásica comienzan a coexistir dos subpoblaciones celulares con distinta biología:
  1. *Subclones portadores (BRCA1+/- / TP53-/-):* Con recombinación homóloga parcialmente competente y acumulación moderada de daño inmunológico.
  2. *Subclones con LOH (BRCA1-/- / TP53-/-):* Con colapso total de la recombinación homóloga (fenotipo "BRCAness"), inestabilidad cromosómica masiva y acumulación de daño inmunológico a velocidad máxima (0.642 por ciclo).
- **Trascendencia biológica:** Esta ramificación modela fielmente la evolución clonal intra-tumoral descrita en la oncología experimental, donde la aparición de subclones hipermutados confiere al tumor una mayor plasticidad biológica, acelerando el escape a tratamientos farmacológicos y la progresión hacia fenotipos de mayor agresividad clínica.

