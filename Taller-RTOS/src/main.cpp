#include "FreeRTOS.h"
#include "task.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h"

// Reserva estática del Tensor Arena en SRAM (128 KB)
constexpr int kTensorArenaSize = 128 * 1024;
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];

StaticTask_t xTaskBuffer;
StackType_t xStack[2048];

void vEdgeAITask(void *pvParameters) {
    // 1. Cargar el modelo cuantizado desde la memoria Flash
    const tflite::Model* model = tflite::GetModel(cnn_quantized_model_tflite);

    // 2. Registrar operadores optimizados acelerados por CMSIS-NN
    static tflite::MicroMutableOpResolver<5> micro_resolver;
    micro_resolver.AddConv2D();
    micro_resolver.AddAveragePool2D();
    micro_resolver.AddFullyConnected();
    micro_resolver.AddRelu();
    micro_resolver.AddReshape();

    // 3. Inicializar el intérprete de TFLite Micro
    tflite::ErrorReporter* error_reporter = tflite::GetDefaultErrorReporter();
    static tflite::MicroInterpreter interpreter(
        model, micro_resolver, tensor_arena, kTensorArenaSize, error_reporter);

    // Asignación estática de tensores
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        while (1); // Bloqueo por fallo de asignación
    }

    TfLiteTensor* output = interpreter.output(0);

    for (;;) {
        // Ejecución de la inferencia en tiempo real
        if (interpreter.Invoke() == kTfLiteOk) {
            uint8_t top_class = output->data.uint8[0];
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main(void) {
    xTaskCreateStatic(vEdgeAITask, "EdgeAI_Task", 2048, NULL, tskIDLE_PRIORITY + 2, xStack, &xTaskBuffer);
    vTaskStartScheduler();
    while (1);
}
