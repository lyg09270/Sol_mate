#include "hardware_test.h"

int app_main(void)
{
    /* Initialize hardware tests */
    // ezbsp_gpio_test_init();
    ezbsp_exti_test_init();

    while (1) {
        /* Handle hardware tests */
        // ezbsp_gpio_test_handler();
        ezbsp_exti_test_handler();
    }

    return 0;
}