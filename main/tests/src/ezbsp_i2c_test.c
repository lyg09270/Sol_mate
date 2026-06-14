#include "hardware_test.h"
#include "ezhal/ezhal_i2c.h"
#include "ezbsp_log.h"
#include "ezbsp_time.h"

// 选用测试的 I2C 物理总线
#define TEST_I2C_BUS        EZHAL_I2C_BUS_0
#define SCAN_INTERVAL_MS    3000   // 每隔 3 秒自动全局扫描一次

// 内部扫描状态机管理
static uint32_t s_last_scan_time = 0;
static bool s_i2c_ready = false;

/**
 * @brief I2C 扫描测试初始化
 */
void ezbsp_i2c_test_init(void)
{
    ezbsp_log_set_level(EZBSP_LOG_LEVEL_DEBUG);
    ezbsp_log_set_auto_newline(true);

    ezbsp_logi("Initializing I2C Bus Scan hardware test...");

    // 1. 初始化物理总线
    if (ezhal_i2c_bus_init(TEST_I2C_BUS) == 0) {
        ezbsp_logi("I2C Bus %d initialized successfully.", TEST_I2C_BUS);
        s_i2c_ready = true;
    } else {
        ezbsp_loge("Failed to initialize I2C Bus %d!", TEST_I2C_BUS);
        s_i2c_ready = false;
        return;
    }

    ezbsp_logi("I2C Test ready. Will scan the bus every %d ms.", SCAN_INTERVAL_MS);
}

/**
 * @brief I2C 扫描测试处理器
 * @note  放入主循环或任务循环中调用，采用非阻塞的定时扫描机制
 */
void ezbsp_i2c_test_handler(void)
{
    if (!s_i2c_ready) {
        ezbsp_delay_ms(100);
        return;
    }

    uint32_t current_time = ezbsp_time_ms();

    // 定时触发器：每隔指定时间自动扫描一次
    if ((current_time - s_last_scan_time) >= SCAN_INTERVAL_MS) {
        s_last_scan_time = current_time;

        ezbsp_logi("-----------------------------------------");
        ezbsp_logi("[%u ms] Launching I2C Address Scan...", current_time);
        ezbsp_logi("Scanning active 7-bit addresses (0x03 - 0x77)...");

        uint8_t devices_found = 0;

        // 遍历标准的 7 位 I2C 有效从机地址区间 (0x03 ~ 0x77)
        for (uint16_t addr = 0x03; addr <= 0x77; addr++) {
            
            // 直接传递总线号和地址进行硬件探针测试，无需频繁动态分配/释放设备槽位
            if (ezhal_i2c_probe(TEST_I2C_BUS, addr) == 0) {
                ezbsp_logi("  => [ACK] Device detected at address: 0x%02X", addr);
                devices_found++;
            }
            
            // 每次探针通信后稍微让出一点时间（2ms），避免连续高频寻址挂死部分老旧外设
            ezbsp_delay_ms(2);
        }

        ezbsp_logi("Scan finished. Total devices found: %d", devices_found);
        ezbsp_logi("-----------------------------------------");
    }

    // 释放控制权，防止 FreeRTOS 任务饿死或看门狗复位
    ezbsp_delay_ms(10);
}