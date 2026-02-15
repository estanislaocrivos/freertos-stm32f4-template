/*
 * FreeRTOS Example: Semaphores
 *
 * Demonstrates three types of semaphores:
 *
 * 1. BINARY SEMAPHORE: Event signaling
 *    - Event generator signals when "something happened"
 *    - Handler task waits and responds
 *
 * 2. COUNTING SEMAPHORE: Resource pool
 *    - Limited number of "resources" available
 *    - Tasks must acquire before use, release after
 *
 * 3. MUTEX: Protect shared resource
 *    - Only one task can access LED at a time
 *    - Prevents race conditions
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "task.h"

/* ========================================================================== */

#define LED_PORT GPIOA
#define LED_PIN  5

static void led_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    LED_PORT->MODER &= ~(3U << (LED_PIN * 2));
    LED_PORT->MODER |= (1U << (LED_PIN * 2));
    LED_PORT->OTYPER &= ~(1U << LED_PIN);
    LED_PORT->OSPEEDR &= ~(3U << (LED_PIN * 2));
    LED_PORT->PUPDR &= ~(3U << (LED_PIN * 2));
}

static void led_on(void)
{
    LED_PORT->BSRR = (1U << LED_PIN);
}
static void led_off(void)
{
    LED_PORT->BSRR = (1U << (LED_PIN + 16));
}
static void led_toggle(void)
{
    LED_PORT->ODR ^= (1U << LED_PIN);
}

/* ========================================================================== */

SemaphoreHandle_t xBinarySemaphore;   /* Event signaling */
SemaphoreHandle_t xCountingSemaphore; /* Resource counting */
SemaphoreHandle_t xMutex;             /* Mutual exclusion */

/* ========================================================================== */

void vEventGenerator(void* pvParameters)
{
    (void)pvParameters;
    int delay_times[] = {500, 1000, 200, 1500, 300}; /* Varying intervals */
    int index         = 0;

    for (;;)
    {
        /* Wait some time (simulating button debounce/press timing) */
        vTaskDelay(pdMS_TO_TICKS(delay_times[index]));
        index = (index + 1) % 5;

        /* Signal that event occurred */
        xSemaphoreGive(xBinarySemaphore);

        /* In real code with ISR:
         * xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
         * portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
         */
    }
}

void vEventHandler(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* Wait for event (blocks until semaphore is given) */
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE)
        {
            /* Event received! Respond with LED blink */
            led_on();
            vTaskDelay(pdMS_TO_TICKS(100));
            led_off();
        }
    }
}

/* ========================================================================== */

#define NUM_RESOURCES 3
#define NUM_WORKERS   5

void vWorkerTask(void* pvParameters)
{
    int worker_id = (int)(intptr_t)pvParameters;
    (void)worker_id; /* Would use for logging in real app */

    for (;;)
    {
        /* Try to acquire a resource (wait up to 1 second) */
        if (xSemaphoreTake(xCountingSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            /*
             * Got a resource! Do work.
             * Show we're working by keeping LED on longer
             * (more workers = more time LED is on)
             */

            /* Simulate work (use the resource) */
            vTaskDelay(pdMS_TO_TICKS(500));

            /* Release the resource */
            xSemaphoreGive(xCountingSemaphore);

            /* Rest before trying again */
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
        {
            /* Timeout - couldn't get resource
             * In real app: log warning, try alternative, etc.
             */
        }
    }
}

/* ========================================================================== */

/*
 * PART 3: Mutex - Protect Shared Resource
 *
 * Two tasks want to do different LED patterns.
 * Without mutex: patterns would interfere (race condition)
 * With mutex: each task completes its pattern atomically
 */

void vPatternTask1(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* Acquire mutex before touching LED */
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            /* We own the LED now - do pattern: 3 fast blinks */
            for (int i = 0; i < 3; i++)
            {
                led_on();
                vTaskDelay(pdMS_TO_TICKS(100));
                led_off();
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            /* Release mutex so other task can use LED */
            xSemaphoreGive(xMutex);
        }

        /* Wait before next pattern */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vPatternTask2(void* pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* Acquire mutex before touching LED */
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            /* We own the LED now - do pattern: long blink */
            led_on();
            vTaskDelay(pdMS_TO_TICKS(500));
            led_off();

            /* Release mutex */
            xSemaphoreGive(xMutex);
        }

        /* Wait before next pattern */
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

/* ========================================================================== */

/* Change this to select demo: 1, 2, or 3 */
#define DEMO_SELECT 3

int main(void)
{
    led_init();

#if DEMO_SELECT == 1
    /*
     * DEMO 1: Binary Semaphore
     * You'll see: LED blinks at irregular intervals
     * (simulating response to button presses)
     */
    xBinarySemaphore = xSemaphoreCreateBinary();

    xTaskCreate(vEventGenerator, "Generator", 128, NULL, 1, NULL);
    xTaskCreate(vEventHandler, "Handler", 128, NULL, 2, NULL);

#elif DEMO_SELECT == 2
    /*
     * DEMO 2: Counting Semaphore
     * You'll see: LED activity as workers use resources
     * 5 workers competing for 3 resources
     */
    xCountingSemaphore = xSemaphoreCreateCounting(NUM_RESOURCES, NUM_RESOURCES);

    for (int i = 0; i < NUM_WORKERS; i++)
    {
        xTaskCreate(vWorkerTask, "Worker", 128, (void*)(intptr_t)i, 1, NULL);
    }

#elif DEMO_SELECT == 3
    /*
     * DEMO 3: Mutex
     * You'll see: Clean LED patterns (3 fast blinks, then long blink)
     * Never mixed up because mutex protects access
     */
    xMutex = xSemaphoreCreateMutex();

    xTaskCreate(vPatternTask1, "Pattern1", 128, NULL, 1, NULL);
    xTaskCreate(vPatternTask2, "Pattern2", 128, NULL, 1, NULL);

#endif

    vTaskStartScheduler();

    for (;;)
        ;
    return 0;
}

/* ========================================================================== */

void SystemInit(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
#endif
}

/* ========================================================================== */
