file(GLOB_RECURSE TEST_SOURCES "tests/src/*.c")
file(GLOB_RECURSE APP_SOURCES "app/src/*.h")
file(GLOB_RECURSE MAIN_SOURCES "*.c")

idf_component_register(SRCS 
                        ${MAIN_SOURCES}
                        ${APP_SOURCES}
                        ${TEST_SOURCES}
                    PRIV_REQUIRES 
                        esp_driver_uart 
                        vfs esp_wifi 
                        esp_pm 
                        nvs_flash
                    INCLUDE_DIRS 
                        "."
                        "app/inc"
                        "tests/inc")
