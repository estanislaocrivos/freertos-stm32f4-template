/*
 * FreeRTOS Kernel Configuration
 *
 * This file configures FreeRTOS for STM32F401.
 * Documentation: https://www.freertos.org/a00110.html
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* CPU frequency in Hz.
 * STM32F401 can run up to 84MHz.
 * By default it boots with HSI at 16MHz. */
#define configCPU_CLOCK_HZ                  (16000000UL)

/* RTOS tick frequency (interrupts per second).
 * 1000 = tick every 1ms. Typical values: 100-1000 Hz */
#define configTICK_RATE_HZ                  ((TickType_t)1000)

/* Task priority levels (0 = lowest) */
#define configMAX_PRIORITIES                (5)

/* Minimum stack size per task (in words, not bytes).
 * 128 words = 512 bytes. For Cortex-M4 one word = 4 bytes */
#define configMINIMAL_STACK_SIZE            ((uint16_t)128)

/* FreeRTOS heap size in bytes.
 * Memory for tasks, queues, semaphores, etc. is allocated from here. */
#define configTOTAL_HEAP_SIZE               ((size_t)(32 * 1024))

/* Maximum task name length */
#define configMAX_TASK_NAME_LEN             (16)

/* Use 32-bit tick counter (vs 16-bit) */
#define configUSE_16_BIT_TICKS              0

/* Allow tasks of equal priority to share CPU time */
#define configUSE_PREEMPTION                1
#define configUSE_TIME_SLICING              1

/* Hook functions (optional callbacks) */
#define configUSE_IDLE_HOOK                 0
#define configUSE_TICK_HOOK                 0
#define configUSE_MALLOC_FAILED_HOOK        0
#define configCHECK_FOR_STACK_OVERFLOW      0

/* Mutexes and semaphores */
#define configUSE_MUTEXES                   1
#define configUSE_RECURSIVE_MUTEXES         1
#define configUSE_COUNTING_SEMAPHORES       1

/* Software timers */
#define configUSE_TIMERS                    1
#define configTIMER_TASK_PRIORITY           (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH            10
#define configTIMER_TASK_STACK_DEPTH        (configMINIMAL_STACK_SIZE * 2)

/* Queue sets and task notifications */
#define configUSE_QUEUE_SETS                1
#define configUSE_TASK_NOTIFICATIONS        1

/* Use dynamic memory allocation (pvPortMalloc/vPortFree) */
#define configSUPPORT_DYNAMIC_ALLOCATION    1

/* Don't use static allocation for now */
#define configSUPPORT_STATIC_ALLOCATION     0

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

/* Cortex-M4 has 4 priority bits (16 levels).
 * ST uses the 4 most significant bits. */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

/* Lowest interrupt priority */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15

/* Maximum priority from which FreeRTOS "FromISR" functions can be called.
 * Interrupts with priority 0-4 CANNOT call FreeRTOS.
 * Interrupts with priority 5-15 CAN. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Conversion to NVIC register format (shifted) */
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
