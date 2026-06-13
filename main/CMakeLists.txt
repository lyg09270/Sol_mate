cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE TEST_SOURCES "tests/src/*.c")
file(GLOB_RECURSE APP_SOURCES "app/src/*.c")
file(GLOB MAIN_SOURCES "*.c")

set(ALL_SOURCES 
    "${MAIN_SOURCES}"
    "${APP_SOURCES}"
    "${TEST_SOURCES}"
)

# 3. 标准注册组件
idf_component_register(
    SRCS "${ALL_SOURCES}"
    PRIV_REQUIRES 
        esp_driver_uart 
        vfs 
        esp_wifi 
        esp_pm 
        nvs_flash
        easy_bsp
    INCLUDE_DIRS 
        "."
        "app/inc"
        "tests/inc"
)