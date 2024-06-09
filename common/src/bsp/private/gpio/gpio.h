#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx.h"

#include "types.h"

typedef u16_t GpioData_t;

typedef enum gpio_pin
{
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
    GPIO_NUM_PINS,
} GpioPin_t;

typedef enum gpio_mode
{
    GPIO_MODE_INPUT        = 0x0,
    GPIO_MODE_OUTPUT_2MHZ  = 0x2,
    GPIO_MODE_OUTPUT_10MHZ = 0x1,
    GPIO_MODE_OUTPUT_50MHZ = 0x3,
} GpioMode_t;

typedef enum gpio_configuration
{
    GPIO_CONF_IN_ANALOG_MODE = 0x0,
    GPIO_CONF_IN_FLOATING    = 0x1,
    GPIO_CONF_IN_PUP_PUD     = 0x2,
    GPIO_CONF_IN_RESERVED    = 0x3,
    GPIO_CONF_OUT_GENERAL_PUSH_PULL  = 0x0,
    GPIO_CONF_OUT_GENERAL_OPEN_DRAIN = 0x1,
    GPIO_CONF_OUT_ALT_PUSH_PULL      = 0x2,
    GPIO_CONF_OUT_ALT_OPEN_DRAIN     = 0x3,
} GpioConfig_t;

void gpio_init(GPIO_TypeDef * const p_gpio);
void gpio_set_mode(GPIO_TypeDef * const p_gpio, GpioPin_t pin, GpioMode_t mode, GpioConfig_t conf);
void gpio_write_port(GPIO_TypeDef * const p_gpio, GpioData_t data);
void gpio_write_pin(GPIO_TypeDef * const p_gpio, GpioPin_t pin, bit_t bit);
GpioData_t gpio_read_port(const GPIO_TypeDef * const p_gpio);
bit_t gpio_read_pin(const GPIO_TypeDef * const p_gpio, GpioPin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
