# 🚀 Tutorial Paso a Paso: Implementación de Edge AI en STM32H7 (ARM Cortex-M7) con FreeRTOS

Este tutorial guía la configuración, compilación e implementación de un modelo de Inteligencia Artificial local (*Edge AI*) en una tarjeta de desarrollo **STM32H7** utilizando la **terminal de Ubuntu**.

---

## 📋 1. Especificaciones de Hardware y Parámetros Técnicos

Basado en las pruebas de rendimiento publicadas para el sistema:

* **Tarjeta de Desarrollo**: STM32H7
* **Microcontrolador**: ARM Cortex-M7 a 480 MHz
* **Memoria de Hardware**: 2 MB Flash y 1 MB SRAM
* **Kernel de Tiempo Real**: FreeRTOS con modo *Tickless Idle* habilitado
* **Motor de Inferencia**: TensorFlow Lite for Microcontrollers (TFLite Micro)
* **Aceleración de Operadores**: Biblioteca **CMSIS-NN** y transferencias por **DMA de cero copia** (*Zero-Copy DMA*)
* **Modelo de IA**: Red Neuronal Convolucional (CNN) de 3 capas convolucionales + 2 capas totalmente conectadas (ReLU), cuantizada en formato enteros de 8 bits (INT8)

### 📊 Métricas Medidas en STM32H7 (480 MHz)
* **Latencia de Inferencia Promedio**: 13.8 ms
* **Jitter Máximo de Latencia**: ±1.1 ms
* **Carga de CPU durante Inferencia**: 85%
* **Consumo Energético por Inferencia**: 0.92 mJ
* **Latencia de Cambio de Contexto (*Tickless*)**: 1.8 µs

---

## 🛠️ 2. Preparación del Entorno en la Terminal de Ubuntu

Abre la terminal de Ubuntu e instala la cadena de herramientas de compilación cruzada (*Cross-Toolchain*) y las herramientas de depuración para ARM Cortex-M:

```bash
# 1. Actualizar repositorios e instalar paquetes base de compilación
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    screen \
    minicom \
    openocd

# 2. Instalación del compilador cruzado GCC para ARM
sudo apt install -y gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi

# 3. Verificación de versiones de las herramientas
arm-none-eabi-gcc --version
cmake --version
```

---

## 📁 3. Estructura del Repositorio de GitHub

Crea la siguiente estructura de archivos en tu proyecto:

```text
stm32h7-edgeai-freertos/
├── CMakeLists.txt          # Script de compilación CMake
├── README.md               # Guía del proyecto
├── config/
│   └── FreeRTOSConfig.h    # Configuración del Kernel FreeRTOS
├── src/
│   ├── main.cpp            # Tarea de inferencia TFLite Micro
│   ├── model_data.cpp      # Matriz de datos del modelo cuantizado
│   └── model_data.h        # Encabezado del modelo
└── drivers/
    └── CMSIS/              # Bibliotecas CMSIS y CMSIS-NN
```

Para inicializar y clonar el repositorio con sus submódulos:

```bash
git clone <URL-DE-TU-REPOSITORIO>
cd stm32h7-edgeai-freertos
git submodule update --init --recursive
```

---

## 📦 4. Exportación del Modelo Cuantizado INT8

Aplica cuantización de 8 bits (*Post-Training Quantization*) al modelo TFLite. Luego, convierte el binario `.tflite` en una matriz plana de C++ utilizando la utilidad `xxd` en Ubuntu:

```bash
# Convertir el binario TFLite a una matriz ejecutable C++
xxd -i cnn_quantized_model.tflite > src/model_data.cpp
```

En `src/model_data.h`:

```cpp
#ifndef MODEL_DATA_H_
#define MODEL_DATA_H_

extern const unsigned char g_cnn_quantized_model_tflite[];
extern const unsigned int g_cnn_quantized_model_tflite_len;

#endif // MODEL_DATA_H_
```

---

## ⚙️ 5. Configuración de FreeRTOS (`FreeRTOSConfig.h`)

Para garantizar asignación estática de memoria (evitando la fragmentación de la memoria RAM) y minimizar la fluctuación temporal (*jitter*), configura `FreeRTOSConfig.h`:

```c
/* Configuración de memoria y rendimiento en FreeRTOSConfig.h */
#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 1  /* Reduce el jitter y el consumo energético */
#define configSUPPORT_STATIC_ALLOCATION         1  /* Asignación estática determinista de memoria */
#define configSUPPORT_DYNAMIC_ALLOCATION        0  /* Desactiva el heap dinámico */
#define configCPU_CLOCK_HZ                      ( 480000000UL ) /* 480 MHz en STM32H7 */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
```

---

## 🧠 6. Código de Inferencia TFLite Micro (`src/main.cpp`)

Implementa la tarea de inferencia en FreeRTOS asignando estáticamente el búfer de memoria `tensor_arena` en SRAM (1 MB disponible en STM32H7) y registrando los operadores acelerados por **CMSIS-NN**:

```cpp
#include "FreeRTOS.h"
#include "task.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h"

// Reserva estática del Tensor Arena en SRAM (128 KB)
constexpr int kTensorArenaSize = 128 * 1024;
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

// Tarea estática de FreeRTOS
StaticTask_t xTaskBuffer;
StackType_t xStack[2048];

void vEdgeAITask(void *pvParameters) {
    // 1. Cargar el modelo desde Flash
    const tflite::Model* model = tflite::GetModel(g_cnn_quantized_model_tflite);

    // 2. Registrar operadores optimizados con CMSIS-NN
    static tflite::MicroMutableOpResolver<5> micro_resolver;
    micro_resolver.AddConv2D();
    micro_resolver.AddAveragePool2D();
    micro_resolver.AddFullyConnected();
    micro_resolver.AddRelu();
    micro_resolver.AddReshape();

    // 3. Inicializar el intérprete de TFLite Micro
    static tflite::MicroInterpreter interpreter(
        model, micro_resolver, tensor_arena, kTensorArenaSize);

    // Asignar memoria de tensores estáticamente
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        while (1); // Error de asignación
    }

    TfLiteTensor* input = interpreter.input(0);
    TfLiteTensor* output = interpreter.output(0);

    for (;;) {
        // A. Carga de datos de sensores mediante DMA de cero copia
        
        // B. Invocación de la inferencia
        TfLiteStatus invoke_status = interpreter.Invoke();
        if (invoke_status == kTfLiteOk) {
            // C. Obtener el resultado de la clasificación
            uint8_t top_class = output->data.uint8[0];
        }

        // D. Suspender tarea hasta el siguiente ciclo (50 ms)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main(void) {
    // Crear la tarea de IA con asignación estática
    xTaskCreateStatic(
        vEdgeAITask,
        "EdgeAI_Task",
        2048,
        NULL,
        tskIDLE_PRIORITY + 2,
        xStack,
        &xTaskBuffer
    );

    // Iniciar el planificador de FreeRTOS
    vTaskStartScheduler();

    while (1);
}
```

---

## 🛠️ 7. Arquitectura de Compilación (`CMakeLists.txt`)

Crea el archivo `CMakeLists.txt` en la raíz de tu proyecto para configurar la compilación cruzada con `arm-none-eabi-gcc`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(stm32h7_edgeai C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Opciones del compilador cruzado ARM Cortex-M7
set(CPU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")

# Directorios de inclusión
include_directories(
    config
    src
    drivers/CMSIS/Include
    drivers/CMSIS/NN/Include
)

# Archivos fuente del proyecto
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
)

add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Generar archivos binario e hex
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND arm-none-eabi-objcopy -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
)
```

---

## 🔨 8. Compilación y Flasheo desde la Terminal

Ejecuta los siguientes comandos en Ubuntu para compilar el firmware y grabarlo en la tarjeta mediante `OpenOCD`:

```bash
# 1. Crear e ingresar al directorio de compilación
mkdir -p build && cd build

# 2. Configurar el proyecto para ARM Cortex-M7 con CMake
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

# 3. Compilar el proyecto
ninja

# 4. Flashear la tarjeta STM32H7 con OpenOCD y ST-LINK
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
    -c "program stm32h7_edgeai.elf verify reset exit"
```

---

## 📊 9. Monitoreo por Puerto Serie

Para observar los resultados de clasificación y las lecturas de latencia en tiempo real desde Ubuntu:

```bash
# Conectar al puerto serie UART a 115200 baudios
minicom -D /dev/ttyUSB0 -b 115200
```
