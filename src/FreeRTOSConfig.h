/*
 * FreeRTOS Kernel Configuration
 *
 * Este archivo configura FreeRTOS para STM32F401.
 * Documentación: https://www.freertos.org/a00110.html
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Configuración básica del sistema
 *----------------------------------------------------------*/

/* Frecuencia del CPU en Hz.
 * El STM32F401 puede correr hasta 84MHz.
 * Por defecto arranca con HSI a 16MHz. */
#define configCPU_CLOCK_HZ              (16000000UL)

/* Frecuencia del tick del RTOS (interrupciones por segundo).
 * 1000 = tick cada 1ms. Valores típicos: 100-1000 Hz */
#define configTICK_RATE_HZ              ((TickType_t)1000)

/* Niveles de prioridad de tareas (0 = más baja) */
#define configMAX_PRIORITIES            (5)

/* Tamaño mínimo del stack por tarea (en words, no bytes).
 * 128 words = 512 bytes. Para Cortex-M4 un word = 4 bytes */
#define configMINIMAL_STACK_SIZE        ((uint16_t)128)

/* Tamaño del heap de FreeRTOS en bytes.
 * De aquí se asigna memoria para tareas, queues, semáforos, etc. */
#define configTOTAL_HEAP_SIZE           ((size_t)(32 * 1024))

/* Longitud máxima del nombre de una tarea */
#define configMAX_TASK_NAME_LEN         (16)

/* Usar contador de 32 bits para el tick (vs 16 bits) */
#define configUSE_16_BIT_TICKS          0

/* Permitir que tareas de igual prioridad compartan tiempo de CPU */
#define configUSE_PREEMPTION            1
#define configUSE_TIME_SLICING          1

/* Hook functions (callbacks opcionales) */
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configUSE_MALLOC_FAILED_HOOK    0
#define configCHECK_FOR_STACK_OVERFLOW  0

/*-----------------------------------------------------------
 * Features habilitadas
 *----------------------------------------------------------*/

/* Mutexes y semáforos */
#define configUSE_MUTEXES               1
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_COUNTING_SEMAPHORES   1

/* Software timers */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    (configMINIMAL_STACK_SIZE * 2)

/* Queue sets y task notifications */
#define configUSE_QUEUE_SETS            1
#define configUSE_TASK_NOTIFICATIONS    1

/*-----------------------------------------------------------
 * Configuración de memoria
 *----------------------------------------------------------*/

/* Usar asignación de memoria dinámica (pvPortMalloc/vPortFree) */
#define configSUPPORT_DYNAMIC_ALLOCATION    1

/* No usar asignación estática por ahora */
#define configSUPPORT_STATIC_ALLOCATION     0

/*-----------------------------------------------------------
 * Funciones de API opcionales
 * 1 = incluir, 0 = excluir (ahorra Flash)
 *----------------------------------------------------------*/

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle      1

/*-----------------------------------------------------------
 * Configuración específica de Cortex-M
 *----------------------------------------------------------*/

/* Cortex-M4 tiene 4 bits de prioridad (16 niveles).
 * ST usa los 4 bits más significativos. */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

/* Prioridad más baja de interrupción */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15

/* Prioridad máxima desde la que se pueden llamar funciones
 * de FreeRTOS que terminan en "FromISR".
 * Interrupciones con prioridad 0-4 NO pueden llamar a FreeRTOS.
 * Interrupciones con prioridad 5-15 SÍ pueden. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Conversión a formato del registro NVIC (shifted) */
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/*-----------------------------------------------------------
 * Handlers del kernel
 *
 * FreeRTOS necesita manejar estas excepciones del Cortex-M.
 * Las renombramos para que coincidan con los nombres en el
 * vector table del startup code de ST.
 *----------------------------------------------------------*/

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
