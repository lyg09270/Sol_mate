#ifndef _HARDWARE_TEST_H_
#define _HARDWARE_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

void ezbsp_gpio_test_init(void);
void ezbsp_gpio_test_handler(void);

void ezbsp_exti_test_init(void);
void ezbsp_exti_test_handler(void);

void ezbsp_i2c_test_init(void);
void ezbsp_i2c_test_handler(void);

void ezdev_als_test_init(void);
void ezdev_als_test_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* _HARDWARE_TEST_H_ */