# Diagrama de Conexiones (Pinout) - Line Follower

Este documento detalla las conexiones físicas entre el **Arduino Uno R4 WiFi**, los sensores, actuadores y alimentación.

## 1. Alimentación (Power)
*   **Batería**: 3x Baterías Li-ion 3.7V en Serie → **11.1V Nominal** (12.6V Max).
*   **Conexión Arduino**:
    *   **Positivo (+)** → Pin `VIN` del Arduino.
    *   **Negativo (-)** → Pin `GND` del Arduino.
    *   **GND Común**: Es CRÍTICO que el GND de la batería, el del Arduino y el del driver L298N estén unidos.
*   **Driver L298N**:
    *   `12V` Input → Batería Positivo (+).
    *   `GND` → Batería Negativo (-).
    *   `5V` Output → No conectado (El Arduino tiene su propio regulador, o puede usarse para alimentar sensores si necesario).


> **⚠️ IMPORTANTE - Jumper de 5V**:
> *   **Jumper Puesto (ON)**: El módulo usa la batería (12V) para generar sus propios 5V. La bornera de 5V es una **SALIDA**.
>     *   *Configuración recomendada para tu batería de 11.1V.*
> *   **Jumper Quitado (OFF)**: El regulador se apaga. Debes meter 5V externos por la bornera de 5V. La bornera de 5V es una **ENTRADA**.
>     *   *Solo haz esto si tu batería fuera mayor a 12V (para no quemar el regulador).*

> **⚠️ Precaución de Voltaje**: Con 11.1V, los motores de 6V se quemarían si se usan al 100%. El firmware limita la potencia automáticamente (`MAX_PWM_LIMIT`) para simular 6V. **No borres esa limitación en el código.**

## 2. Motores (Driver L298N)
| Motor | Driver Pin | Arduino Pin | Función |
| :--- | :--- | :--- | :--- |
| **Izquierdo** | IN1 | **D2** | Dirección A |
| | IN2 | **D4** | Dirección B |
| | ENA | **D3** (PWM) | Velocidad (Potencia) |
| **Derecho** | IN3 | **D12** | Dirección A |
| | IN4 | **D13** | Dirección B |
| | ENB | **D11** (PWM) | Velocidad (Potencia) |

## 3. Sensores de Línea (QTR-8A o Similar)
Se utilizan 6 sensores analógicos para detectar la línea y nodos.

| Sensor ID | Posición Física | Arduino Pin |
| :--- | :--- | :--- |
| 0 | Extremo Derecho | **A0** |
| 1 | Derecha Centro | **A1** |
| 2 | Centro Derecha | **A2** |
| 3 | Centro Izquierda | **A3** |
| 4 | Izquierda Centro | **A4** |
| 5 | Extremo Izquierdo | **A5** |

*   **VCC** → 5V del Arduino.
*   **GND** → GND del Arduino.
*   **Emitter (Opcional)** → **D6** (Para apagar/prender los IR y ahorrar energía/calibrar luz ambiente).

## 4. Otros Sensores
| Componente | Pin Trigger | Pin Echo | Notas |
| :--- | :--- | :--- | :--- |
| **Sonar (HC-SR04)** | **D12** | **D11** | *Conflicto actual con Motor Derecho*. Revisar `Config.h` antes de conectar. Desactivado en código main. |

## Resumen de Pines Utilizados
*   **Digitales**: 2, 3, 4, 6, 11, 12, 13 + (Pines usados por WiFi ESP32-S3 internos).
*   **Analógicos**: A0, A1, A2, A3, A4, A5.
