# 🛠️ Taller Práctico: Sistemas Operativos de Tiempo Real (RTOS) con FreeRTOS

¡Bienvenido al **Taller Práctico de Sistemas Operativos de Tiempo Real (RTOS) con enfoque en FreeRTOS y ARM Cortex-M**! Este repositorio contiene la estructura completa del código fuente, configuraciones del Kernel, tutoriales paso a paso y demostraciones prácticas listas para ser ejecutadas en un entorno virtualizado sin necesidad de hardware físico [5, 6].

---

## 📌 1. Introducción al Tema

En el desarrollo de sistemas embebidos modernos —desde dispositivos médicos y controles automotrices hasta nodos de **Edge AI / TinyML**— los microcontroladores deben responder a eventos del mundo real dentro de límites de tiempo estrictos y predecibles (determinismo temporal) [2, 5, 13]. 

A diferencia de los Sistemas Operativos de Propósito General (GPOS) como Linux o Windows, un **Sistema Operativo de Tiempo Real (RTOS)** como **FreeRTOS** [5, 6, 15]:
* **Garantiza la máxima latencia de respuesta**: Utiliza un planificador preventivo por prioridades (*Fixed-Priority Preemptive Scheduler*) [5, 10].
* **Minimiza la huella de memoria**: Requiere solo de 1 KB a 10 KB de Flash/ROM y unos pocos Kilobytes de SRAM [5, 10, 165].
* **Ofrece primitivas de sincronización deterministas**: Colas FIFO, semáforos binarios/contadores, temporizadores de software y Mutexes con protocolo de *Herencia de Prioridades* (*Priority Inheritance*) para prevenir bloqueos de sistema por inversión de prioridad [5, 17, 18].

En este taller se abordan desde las bases teóricas del cambio de contexto (*Context Switching*) en arquitecturas **ARM Cortex-M** hasta la implementación avanzada de tareas con asignación estática de memoria (`xTaskCreateStatic`) e inferencia local de modelos de Inteligencia Artificial [2, 5, 8, 19].

---

## 📁 2. Estructura y Distribución de Directorios

El proyecto está organizado en una arquitectura modular limpia para facilitar la compilación cruzada con **CMake** y **Ninja** [5, 7]:

```text
Taller-RTOS/
├── README.md                     # Guía principal del taller y repositorio
├── CMakeLists.txt                # Script de compilación principal para ARM GCC
├── config/
│   └── FreeRTOSConfig.h          # Parámetros y directivas de configuración del Kernel
├── src/
│   ├── main.cpp                  # Aplicación principal, tareas e inferencia Edge AI / Demostración
│   ├── model_data.cpp            # Matriz de datos C++ del modelo cuantizado INT8
│   └── model_data.h              # Encabezado con declaración del modelo TFLite
├── drivers/
│   ├── FreeRTOS/                 # Fuentes oficiales del Kernel FreeRTOS (C y Ensamblador)
│   │   ├── include/              # Cabeceras públicas (FreeRTOS.h, task.h, semphr.h, etc.)
│   │   ├── portable/             # Capa de portabilidad específica para ARM Cortex-M
│   │   │   └── GCC/ARM_CM3/      # Archivos del port GCC (port.c, portmacro.h)
│   │   ├── tasks.c               # Motor central del planificador
│   │   ├── list.c                # Listas dobles enlazadas para colas de listos/bloqueados
│   │   ├── queue.c               # Implementación de colas, semáforos y mutexes
│   │   └── heap_4.c              # Esquema de gestión dinámica de memoria RAM
│   └── CMSIS/                    # Bibliotecas de interfaz de microcontroladores ARM
│       ├── Include/              # Cabeceras de núcleo Cortex-M
│       └── NN/Include/           # Extensión CMSIS-NN para aceleración de redes neuronales
└── build/                        # Directorio generado para artefactos compilados (.elf, .bin, .hex)
```

---

## 🧰 3. Requisitos Previos

### 🧠 **Conocimientos Previos Recomendados**
1. **Lenguaje C / C++ Embebido**: Manejo de punteros, estructuras, almacenamiento de memoria en pila (*Stack*) y punteros a funciones [5, 13].
2. **Conceptos de Arquitectura de Computadores**: Registros de la CPU, interrupciones hardware (ISRs), mapas de memoria Flash/RAM y excepciones ARM Cortex-M (SysTick, SVC, PendSV) [5, 8].
3. **Fundamentos de Sistemas Operativos**: Conceptos básicos de procesos/hilos, estados de tarea (*Ready, Running, Blocked*), concurrencia y secciones críticas [5, 15].

### 💻 **Requisitos de Software (Entorno Ubuntu / WSL2)**
El taller está diseñado para ejecutarse completamente en la terminal de **Linux Ubuntu** (o en **WSL2** sobre Windows) sin requerir placas físicas de desarrollo.

Instale las herramientas necesarias ejecutando en su terminal:
```bash
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    qemu-system-arm \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    gdb-multiarch
```

---

## 🎓 4. Metodología e Impartición del Taller

El taller se imparte bajo una **modalidad teórico-práctica e interactiva** organizada en cuatro bloques secuenciales:

```text
[ Bloque 1: Fundamentos Teóricos ] ──► [ Bloque 2: Configuración e Inserción del Kernel ]
                                                                   │
[ Bloque 4: Reto Autónomo en Casa ] ◄── [ Bloque 3: Laboratorio Práctico en QEMU ]
```

1. **Bloque 1: Fundamentos Teóricos y Arquitectura (30 min)**
   * Exposición interactiva sobre la diferencia entre GPOS y RTOS [5, 15].
   * Análisis del cambio de contexto en ARM Cortex-M: apilamiento por hardware ($R0-R3, R12, LR, PC, xPSR$) y desapilamiento en la excepción `PendSV_Handler` [5, 8].

2. **Bloque 2: Configuración del Kernel y Toolchain (30 min)**
   * Descarga y estructuración del Kernel en `drivers/FreeRTOS/` [5].
   * Ajuste fino del archivo `FreeRTOSConfig.h` (`configUSE_PREEMPTION`, `configUSE_MUTEXES`, asignación estática vs dinámica) [5, 10].

3. **Bloque 3: Demostración Práctica Guiada en Vivo (15 - 30 min)**
   * Recreación del célebre fallo de la misión **Mars Pathfinder**: observación visual de la **Inversión de Prioridades** usando un semáforo binario [5, 17].
   * Corrección en tiempo real aplicando un **Mutex** y observando la activación de la **Herencia de Prioridades** [5, 18].
   * Ejecución y monitoreo en el emulador **QEMU**:
     ```bash
     cd build && cmake -G Ninja .. && ninja
     qemu-system-arm -M mps2-an385 -cpu cortex-m3 -nographic -kernel freertos_qemu_demo.elf -semihosting
     ```

4. **Bloque 4: Laboratorio Autónomo y Reto Final (Trabajo Personal)**
   * Medición de latencia de inferencia en **Edge AI** simulada utilizando *TensorFlow Lite for Microcontrollers* y asignación estática del `tensor_arena` de 128 KB en SRAM [2, 10].

---

## 📚 5. Referencias y Material Complementario

* **FreeRTOS Reference Manual**: Documentación oficial de API y macros del Kernel [10].
* **Mastering the FreeRTOS™ Real Time Kernel**: Guía práctica por Richard Barry [5].
* **ARM Cortex-M RTOS Context Switching (Memfault Interrupt)**: Análisis en ensamblador de la conmutación de contexto [8].
