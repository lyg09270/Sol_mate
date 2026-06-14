#include "hardware_test.h"
#include "als/ezdev_als.h"
#include "ezbsp_log.h"
#include "ezbsp_time.h"

#define TAG "TEST_ALS"

#define LOG_POLL_INTERVAL_MS   1000   /* Read out light values every 1 second */
#define MODE_SW_INTERVAL_MS    9000   /* Dynamically change modes/configs every 9 seconds */

static uint32_t s_last_poll_time = 0;
static uint32_t s_last_mode_time = 0;
static bool s_als_ready = false;
static uint8_t s_test_state = 0;      /* Internal state tracker for dynamic changes */

/**
 * @brief Initialize the generic ALS system hardware test
 */
void ezdev_als_test_init(void)
{
    ezbsp_log_set_level(EZBSP_LOG_LEVEL_DEBUG);
    ezbsp_log_set_auto_newline(true);

    ezbsp_logi("Initializing ALS Device Framework test runner...");

    /* 1. Initialize generic device 0 (Invokes the tied internal VEML7700 init routines) */
    if (ezdev_als_init(EZDEV_ALS_DEVICE_0) == 0) {
        ezbsp_logi("ALS Device 0 framework context built successfully.");
    } else {
        ezbsp_loge("Failed to initialize ALS Device 0!");
        s_als_ready = false;
        return;
    }

    /* 2. Configure baseline optical parameters: Normal Gain, Normal Integration Time */
    if (ezdev_als_set_config(EZDEV_ALS_DEVICE_0, EZDEV_ALS_GAIN_NORMAL, EZDEV_ALS_INTEGRATION_NORMAL) != 0) {
        ezbsp_loge("Failed to program initial optical configuration!");
        return;
    }

    /* 3. Wake up the chip into continuous tracking mode */
    if (ezdev_als_set_power_mode(EZDEV_ALS_DEVICE_0, EZDEV_ALS_MODE_CONTINUOUS) != 0) {
        ezbsp_loge("Failed to set power mode to Continuous!");
        return;
    }

    s_als_ready = true;
    uint32_t boot_time = ezbsp_time_ms();
    s_last_poll_time = boot_time;
    s_last_mode_time = boot_time;

    ezbsp_logi("ALS Hardware Test Ready. Sampling every %d ms.", LOG_POLL_INTERVAL_MS);
}

/**
 * @brief ALS Test State Machine Handler
 * @note Drop this into the main loop or a dedicated background task thread.
 */
void ezdev_als_test_handler(void)
{
    if (!s_als_ready) {
        ezbsp_delay_ms(100);
        return;
    }

    uint32_t current_time = ezbsp_time_ms();

    /* --- TIMED ACTION 1: Dynamic Config & Power Mode Rotator (Every 9s) --- */
    if ((current_time - s_last_mode_time) >= MODE_SW_INTERVAL_MS) {
        s_last_mode_time = current_time;
        s_test_state = (s_test_state + 1) % 3;

        ezbsp_logi("=================================================================");
        switch (s_test_state) {
            case 0:
                ezbsp_logi("[%u ms] [CONFIG CHANGE] Setting High Sensitivity (Dark Environment)", current_time);
                ezdev_als_set_config(EZDEV_ALS_DEVICE_0, EZDEV_ALS_GAIN_HIGH, EZDEV_ALS_INTEGRATION_SLOW);
                ezdev_als_set_power_mode(EZDEV_ALS_DEVICE_0, EZDEV_ALS_MODE_CONTINUOUS);
                break;

            case 1:
                ezbsp_logi("[%u ms] [CONFIG CHANGE] Switching to Low Power Saving Mode (PSM)", current_time);
                ezdev_als_set_config(EZDEV_ALS_DEVICE_0, EZDEV_ALS_GAIN_NORMAL, EZDEV_ALS_INTEGRATION_NORMAL);
                ezdev_als_set_power_mode(EZDEV_ALS_DEVICE_0, EZDEV_ALS_MODE_POWER_SAVE);
                break;

            case 2:
                ezbsp_logi("[%u ms] [CONFIG CHANGE] Shifting to Deep Shutdown (ADC Off, Sleep)", current_time);
                ezdev_als_set_power_mode(EZDEV_ALS_DEVICE_0, EZDEV_ALS_MODE_SHUTDOWN);
                break;

            default:
                break;
        }
        ezbsp_logi("=================================================================");
    }

    /* --- TIMED ACTION 2: Fast Polling, Non-Blocking Sensor Update (Every 1s) --- */
    if ((current_time - s_last_poll_time) >= LOG_POLL_INTERVAL_MS) {
        s_last_poll_time = current_time;

        /* Trigger non-blocking I2C sampling and flush data into local framework cache */
        int update_ret = ezdev_als_update(EZDEV_ALS_DEVICE_0);

        if (update_ret == 0) {
            ezdev_als_data_t light_metrics = {0};

            /* Extract cached structures containing structural physical quantities and timestamps */
            if (ezdev_als_get_data(EZDEV_ALS_DEVICE_0, &light_metrics) == 0) {
                ezbsp_logi("[%u ms] ALS Data Out => Ambient: %.3f Lux | White Spectrum: %.3f Lux | Tick: %u us", 
                           current_time, 
                           light_metrics.lux, 
                           light_metrics.white_lux, 
                           light_metrics.timestamp_us);
            } else {
                ezbsp_logw("[%u ms] Fetch skipped: Cache invalid (Device initialized but data invalidated)", current_time);
            }
        } else {
            /* If state is Shutdown, reading registers will fail or return invalid/stale parameters */
            if (s_test_state == 2) {
                ezbsp_logi("[%u ms] Core is sleeping. Update intentionally returned NACK/Failure.", current_time);
            } else {
                ezbsp_loge("[%u ms] I2C Bus Error or sensor disconnect detected during update!", current_time);
            }
        }
    }

    /* Keep background execution clean for lower priority threads and RTOS task control */
    ezbsp_delay_ms(10);
}