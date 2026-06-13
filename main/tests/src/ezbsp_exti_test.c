    #include "hardware_test.h"
    #include "ezhal/ezhal_gpio.h"
    #include "ezhal/ezhal_exti.h"
    #include "ezbsp_log.h"
    #include "ezbsp_time.h"

    // Define test configurations
    #define TEST_BUTTON_PIN     1   // Common BOOT button pin for ESP32-C6
    #define TEST_LED_PIN        2   // Status LED pin
    #define DEBOUNCE_TIME_MS    200

    // Shared flag between ISR and Main thread
    static volatile bool s_button_pressed = false;

    /**
     * @brief EXTI interrupt callback handler
     * @note  Runs in ISR context. Keep execution short and fast.
     */
    static void ezbsp_button_exti_callback(uint32_t pin)
    {
        if (pin == TEST_BUTTON_PIN) {
            s_button_pressed = true;
        }
    }

    void ezbsp_exti_test_init(void)
    {
        ezbsp_log_set_level(EZBSP_LOG_LEVEL_DEBUG);
        ezbsp_log_set_auto_newline(true);

        ezbsp_logi("Initializing EXTI hardware test...");

        // 1. Initialize LED indicator
        if (ezhal_gpio_init(TEST_LED_PIN, EZHAL_GPIO_MODE_OUTPUT_PP) != 0) {
            ezbsp_loge("Failed to initialize indicator LED.");
            return;
        }
        ezhal_gpio_write(TEST_LED_PIN, EZHAL_GPIO_LEVEL_LOW);

        // 2. Initialize Button with External Interrupt on Falling Edge
        if (ezhal_exti_init(TEST_BUTTON_PIN, EZHAL_EXTI_TRIGGER_FALLING, EZHAL_EXTI_PULL_UP, ezbsp_button_exti_callback) == 0) {
            ezbsp_logi("EXTI on Pin %d initialized successfully.", TEST_BUTTON_PIN);
        } else {
            ezbsp_loge("Failed to initialize EXTI on Pin %d!", TEST_BUTTON_PIN);
            return;
        }

        // 3. Enable the interrupt channel
        ezhal_exti_enable(TEST_BUTTON_PIN);
        ezbsp_logi("EXTI Test ready. Press the button to trigger interrupt.");
    }

    void ezbsp_exti_test_handler(void)
    {
        static uint32_t last_trigger_time = 0;
        static bool led_state = false;

        // Check if the ISR has flipped the execution flag
        if (s_button_pressed) {
            // Clear flag immediately
            s_button_pressed = false;

            uint32_t current_time = ezbsp_time_ms();

            // Simple software debounce verification
            if ((current_time - last_trigger_time) >= DEBOUNCE_TIME_MS) {
                last_trigger_time = current_time;

                // Toggle LED state to visually show the interrupt caught successfully
                led_state = !led_state;
                ezhal_gpio_write(TEST_LED_PIN, led_state);

                ezbsp_logi("[%u ms] EXTI Triggered! LED status -> %s", 
                        current_time, led_state ? "HIGH" : "LOW");
            }
        }

        // Yield control safely to let FreeRTOS tasks / Watchdog process smoothly
        ezbsp_delay_ms(10);
    }