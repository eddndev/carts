# Guía de Solución de Problemas (Troubleshooting)

## Síntoma: "Leds Congelados" o Falla de Conexión en Sala
Si el robot funciona en el cuarto pero falla en la sala (imagen de "Radar" congelada o estática), existen 3 causas probables:

### 1. Interferencia RF (Canales WiFi)
*   **Problema**: El **Arduino Uno R4 WiFi** utiliza la banda de **2.4GHz**. Si tu Router de casa y tu Hotspot móvil están en el mismo canal (ej. Canal 6), el Arduino se "ensordece" por la potencia del Router cercano y no escucha al teléfono.
*   **Diagnóstico**: La llamada `WiFi.begin()` se bloquea intentando negociar con el ruido.
*   **Solución**:
    *   Aleja el "punto de inicio" del robot al menos 3 metros del Router.
    *   Si tu Android/iPhone lo permite, cambia la banda del Hotspot a "Solo 2.4GHz" o activa "Maximizar Compatibilidad".

### Caso de Estudio Real (El "Efecto Mickey")
En tus pruebas vimos esto:
1.  **Escenario Ideal**: `CartControl` tiene señal alta (-34 dBm) y el router de casa `MickeyRtr` está lejos (-70 dBm). Todo funciona.
2.  **El Problema**: Al moverte a la sala, `MickeyRtr` subió a **-38 dBm** (señal fortísima, probablemente estás a 1 metro del router).
3.  **Resultado**: Inmediatamente `CartControl` desapareció de la lista (0 dispositivos).
4.  **Causa (Near-Far Problem)**: El transmisor de `MickeyRtr` es tan fuerte que satura la radio del Arduino (como si alguien te gritara al oído), impidiéndole escuchar al Hotspot aunque este esté cerca.
5.  **Solución**: Aléjate al menos 3-4 metros de `MickeyRtr` o apágalo temporalmente para las pruebas.

### 2. Brownout por Batería (Voltaje en Frío)
*   **Problema**: Las baterías Li-ion de 3.7V pierden capacidad de entregar corriente en frío.
*   **Mecánica**: El chip WiFi (ESP32-S3) consume picos altos de corriente al intentar conectar. Si el voltaje total cae momentáneamente por debajo de ~5V en la entrada del regulador, el chip se reinicia o se cuelga.
*   **Diagnóstico**: El robot intenta conectar, parece congelarse, y quizás parpadea o se reinicia.
*   **Solución**:
    *   Carga las baterías al 100% (12.6V total).
    *   Revisa si el cableado de alimentación es muy largo o delgado (caida de tensión).

### 3. Bloqueo de Hardware (Firmware)
*   **Explicación**: Aunque el código está diseñado para ser "No Bloqueante", la librería interna del Arduino Uno R4 (`WiFiS3`) tiene bugs conocidos donde `WiFi.begin()` puede congelar el procesador principal si el entorno de radio es muy hostil.
*   **Mitigación (Aplicada)**: Hemos aumentado el tiempo entre intentos a **5 segundos**.
*   **Comportamiento Esperado**: Ahora deberías ver la animación de Radar moviéndose fluidamente por 5 segundos, congelarse brevemente (1-2s) mientras intenta conectar, y volver a moverse. Si se congela *para siempre*, es un fallo eléctrico (Punto 2).

## Lista de Verificación
1.  [ ] Baterías cargadas a >11.5V.
2.  [ ] Hotspot en modo "Maximizar Compatibilidad" (2.4GHz).
3.  [ ] Alejado 3m de otros Routers.
4.  [ ] Nombre de red (SSID) y Pass en `Config.h` son idénticos al Hotspot.
