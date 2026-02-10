/*
 * FreeRTOS Example: Practical Queue Usage
 *
 * Simulates a real-world scenario:
 * - Sensor task: reads "temperature" (simulated) and sends to queue
 * - Processor task: receives readings and controls LED based on thresholds
 * - Monitor task: checks for alerts and blinks rapidly if temp is critical
 *
 * This demonstrates:
 * - Struct data in queues
 * - Multiple consumers
 * - Timeout handling
 * - Real-world patterns
 */

/* ========================================================================== */

#include "FreeRTOS.h"
#include "queue.h"
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

/* ========================================================================== */

/* Sensor reading with metadata */
typedef struct
{
    int16_t  temperature; /* Temperature in tenths of degree (255 = 25.5°C) */
    uint8_t  sensor_id;   /* Which sensor (for multi-sensor systems) */
    uint32_t timestamp;   /* Tick count when reading was taken */
} SensorReading_t;

/* Temperature thresholds */
#define TEMP_COLD     150 /* 15.0°C - LED off */
#define TEMP_NORMAL   250 /* 25.0°C - LED slow blink */
#define TEMP_HOT      350 /* 35.0°C - LED on solid */
#define TEMP_CRITICAL 400 /* 40.0°C - LED fast blink (alert!) */

QueueHandle_t xSensorQueue; /* Sensor -> Processor */
QueueHandle_t xAlertQueue;  /* Processor -> Monitor */

/* ========================================================================== */

/*
 * Simulated sensor reading
 *
 * In real life this would read ADC, I2C sensor, etc.
 * Here we simulate temperature that slowly oscillates.
 */
static int16_t read_simulated_sensor(void)
{
    static int16_t temp      = 200; /* Start at 20.0°C */
    static int8_t  direction = 5;   /* Change by 0.5°C each reading */

    temp += direction;

    /* Oscillate between 10°C and 45°C */
    if (temp >= 450)
    {
        direction = -5;
    }
    else if (temp <= 100)
    {
        direction = 5;
    }

    return temp;
}

/* ========================================================================== */

/*
 * Task 1: Sensor Reader
 *
 * Reads sensor periodically and sends data to queue.
 * This task doesn't care what happens with the data.
 */
static void vSensorTask(void* pvParameters)
{
    (void)pvParameters;
    SensorReading_t reading;

    for (;;)
    {
        /* Take a reading */
        reading.temperature = read_simulated_sensor();
        reading.sensor_id   = 1;
        reading.timestamp   = xTaskGetTickCount();

        /* Send to queue
         * If queue is full, we wait up to 100ms then drop the reading.
         * In real systems you might want different behavior. */
        if (xQueueSend(xSensorQueue, &reading, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            /* Queue full - reading dropped
             * In production: log this, increment error counter, etc. */
        }

        /* Read sensor every 200ms */
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* ========================================================================== */

/*
 * Task 2: Data Processor
 *
 * Receives sensor readings, processes them, and controls LED.
 * Also sends alerts to monitor task if temperature is critical.
 */
void vProcessorTask(void* pvParameters)
{
    (void)pvParameters;
    SensorReading_t reading;
    uint8_t         alert;

    for (;;)
    {
        /* Wait for a reading (block indefinitely) */
        if (xQueueReceive(xSensorQueue, &reading, portMAX_DELAY) == pdTRUE)
        {
            /* Process based on temperature */
            if (reading.temperature >= TEMP_CRITICAL)
            {
                /* CRITICAL: Send alert and turn LED on */
                led_on();
                alert = 1;
                xQueueSend(xAlertQueue, &alert, 0); /* Non-blocking */
            }
            else if (reading.temperature >= TEMP_HOT)
            {
                /* HOT: LED solid on */
                led_on();
            }
            else if (reading.temperature >= TEMP_NORMAL)
            {
                /* NORMAL: LED indicates "working" with slow toggle */
                /* We just turn it on here; blinking handled by timing */
                led_on();
                vTaskDelay(pdMS_TO_TICKS(100));
                led_off();
            }
            else
            {
                /* COLD: LED off */
                led_off();
            }
        }
    }
}

/* ========================================================================== */

/*
 * Task 3: Alert Monitor
 *
 * Waits for critical alerts and responds with rapid blinking.
 * This could also trigger a buzzer, send notification, etc.
 */
void vMonitorTask(void* pvParameters)
{
    (void)pvParameters;
    uint8_t alert;

    for (;;)
    {
        /* Wait for an alert (block indefinitely) */
        if (xQueueReceive(xAlertQueue, &alert, portMAX_DELAY) == pdTRUE)
        {
            /* Critical alert received! Blink rapidly for 2 seconds */
            for (int i = 0; i < 20; i++)
            {
                led_on();
                vTaskDelay(pdMS_TO_TICKS(50));
                led_off();
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
    }
}

/* ========================================================================== */

int main(void)
{
    led_init();

    /* Create queues */
    xSensorQueue = xQueueCreate(10, sizeof(SensorReading_t));
    xAlertQueue  = xQueueCreate(5, sizeof(uint8_t));

    if (xSensorQueue == NULL || xAlertQueue == NULL)
    {
        /* Failed to create queues - error blink */
        for (;;)
        {
            led_on();
            for (volatile int i = 0; i < 50000; i++)
                ;
            led_off();
            for (volatile int i = 0; i < 50000; i++)
                ;
        }
    }

    /* Create tasks with different priorities.Sensor: Priority 2 (higher) - we
     * don't want to miss readings Processor: Priority 1 - processes when it can
     * Monitor: Priority 3 (highest) - alerts are important! */
    xTaskCreate(vSensorTask, "Sensor", 128, NULL, 2, NULL);
    xTaskCreate(vProcessorTask, "Processor", 128, NULL, 1, NULL);
    xTaskCreate(vMonitorTask, "Monitor", 128, NULL, 3, NULL);

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
