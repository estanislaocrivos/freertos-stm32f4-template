/*
 * FreeRTOS STM32F4 Template
 *
 * Ejemplo básico con dos tareas que parpadean el LED.
 * LED onboard del Nucleo-F401RE: PA5
 */

#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

/*-----------------------------------------------------------
 * Configuración del LED (PA5 en Nucleo-F401RE)
 *----------------------------------------------------------*/

#define LED_PORT GPIOA
#define LED_PIN 5

static void led_init(void) {
  /* Habilitar clock de GPIOA */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

  /* Configurar PA5 como salida */
  LED_PORT->MODER &= ~(3U << (LED_PIN * 2)); /* Limpiar bits */
  LED_PORT->MODER |= (1U << (LED_PIN * 2));  /* 01 = Output */

  /* Push-pull, sin pull-up/down, velocidad baja (suficiente para LED) */
  LED_PORT->OTYPER &= ~(1U << LED_PIN);
  LED_PORT->OSPEEDR &= ~(3U << (LED_PIN * 2));
  LED_PORT->PUPDR &= ~(3U << (LED_PIN * 2));
}

static void led_toggle(void) { LED_PORT->ODR ^= (1U << LED_PIN); }

static void led_on(void) { LED_PORT->BSRR = (1U << LED_PIN); }

static void led_off(void) {
  LED_PORT->BSRR = (1U << (LED_PIN + 16)); /* Reset bit */
}

/*-----------------------------------------------------------
 * Tareas de FreeRTOS
 *----------------------------------------------------------*/

/*
 * Tarea 1: Toggle del LED cada 500ms
 *
 * pvParameters permite pasar datos a la tarea al crearla.
 * En este caso no lo usamos.
 */
void vTask1(void *pvParameters) {
  (void)pvParameters; /* Evitar warning de parámetro no usado */

  for (;;) /* Las tareas nunca deben retornar */
  {
    led_toggle();

    /* Bloquear la tarea por 500ms.
     * Durante este tiempo, otras tareas pueden ejecutarse.
     * pdMS_TO_TICKS convierte milisegundos a ticks del RTOS. */
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/*
 * Tarea 2: Parpadeo rápido (100ms) por 1 segundo, luego pausa 2 segundos
 *
 * Esto demuestra que ambas tareas corren "simultáneamente".
 * En realidad el scheduler las alterna muy rápido.
 */
void vTask2(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    /* Parpadeo rápido por 1 segundo (10 toggles) */
    for (int i = 0; i < 10; i++) {
      led_toggle();
      vTaskDelay(pdMS_TO_TICKS(100));
    }

    /* Pausa de 2 segundos con LED apagado */
    led_off();
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

/*-----------------------------------------------------------
 * Main
 *----------------------------------------------------------*/

int main(void) {
  /* Inicializar hardware */
  led_init();

  /* Crear tareas
   *
   * xTaskCreate(
   *     función,           - Puntero a la función de la tarea
   *     "nombre",          - Nombre para debugging
   *     stack_size,        - Tamaño del stack en words
   *     parámetros,        - Dato a pasar a la tarea (o NULL)
   *     prioridad,         - 0 = más baja, configMAX_PRIORITIES-1 = más alta
   *     &handle            - Handle para controlar la tarea (o NULL)
   * );
   */

  /* Solo creamos Task1 por ahora.
   * Descomenta Task2 para ver cómo interactúan. */
  xTaskCreate(vTask1, "LED_Blink", 128, NULL, 1, NULL);
  /* xTaskCreate(vTask2, "LED_Fast", 128, NULL, 1, NULL); */

  /* Iniciar el scheduler.
   * Esta función NUNCA retorna si todo está bien.
   * A partir de aquí, FreeRTOS toma control del CPU. */
  vTaskStartScheduler();

  /* Si llegamos aquí, hubo un error (ej: no hay memoria para idle task) */
  for (;;) {
    /* Blink de error muy rápido */
    led_toggle();
    for (volatile int i = 0; i < 100000; i++)
      ;
  }

  return 0; /* Nunca se alcanza */
}

/*-----------------------------------------------------------
 * System functions requeridas
 *----------------------------------------------------------*/

/*
 * SystemInit es llamada por el startup code antes de main().
 * Aquí podrías configurar el PLL para correr a 84MHz.
 * Por ahora dejamos el default: HSI @ 16MHz.
 */
void SystemInit(void) {
/* FPU habilitada (Cortex-M4F) */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |=
      ((3UL << 10 * 2) | (3UL << 11 * 2)); /* CP10 y CP11 Full Access */
#endif
}
