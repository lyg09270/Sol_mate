#include "hardware_test.h"
#include "ezhal/ezhal_gpio.h"
#include "ezbsp_log.h"
#include "ezbsp_time.h"

// Define test configuration
#define TEST_LED_PIN        2
#define TOGGLE_INTERVAL_MS  500

void ezbsp_gpio_test_init(void)
{
    /* Configure log system for the test environment */
    ezbsp_log_set_level(EZBSP_LOG_LEVEL_DEBUG);
    ezbsp_log_set_auto_newline(true);

    ezbsp_logi("Initializing GPIO test (Delay Version)...");

    /* Initialize GPIO pin as output pull-push */
    if (ezhal_gpio_init(TEST_LED_PIN, EZHAL_GPIO_MODE_OUTPUT_PP) == 0) {
        ezbsp_logi("GPIO %d initialized successfully.", TEST_LED_PIN);
    } else {
        ezbsp_loge("Failed to initialize GPIO %d!", TEST_LED_PIN);
    }
}

void ezbsp_gpio_test_handler(void)
{
    static bool led_state = false;
    uint32_t current_time;

    /* 1. Write to hardware */
    ezhal_gpio_write(TEST_LED_PIN, led_state);
    
    /* 2. Get timestamp and log the state changes */
    current_time = ezbsp_time_ms();
    if (led_state) {
        ezbsp_logd("[%u ms] LED state changed to: HIGH", current_time);
    } else {
        ezbsp_logd("[%u ms] LED state changed to: LOW", current_time);
    }

    /* 3. Toggle state for the next cycle */
    led_state = !led_state;

    /* 4. Use RTOS blocking delay instead of non-blocking timestamp comparison.
          This yields the CPU to the FreeRTOS IDLE task and prevents WDT starvation. */
    ezbsp_delay_ms(TOGGLE_INTERVAL_MS);
}