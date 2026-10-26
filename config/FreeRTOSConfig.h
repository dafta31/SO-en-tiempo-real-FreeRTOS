#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 1   /* Reduce el jitter y el consumo energético */
#define configSUPPORT_STATIC_ALLOCATION         1   /* Asignación estática determinista de memoria */
#define configSUPPORT_DYNAMIC_ALLOCATION        0   /* Desactiva el heap dinámico */
#define configCPU_CLOCK_HZ                      ( 480000000UL ) /* 480 MHz en STM32H7 */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )

#endif /* FREERTOS_CONFIG_H */
