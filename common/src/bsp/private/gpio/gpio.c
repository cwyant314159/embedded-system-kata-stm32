#include "bsp/private/gpio/gpio.h"

#include "bsp/bsp.h"
#include "stm32f1xx.h"
#include "types.h"

static void enable_gpio_clk(const GPIO_TypeDef * const p_gpio);

void gpio_init(GPIO_TypeDef * const p_gpio)
{
    enable_gpio_clk(p_gpio);
}

void gpio_set_mode(GPIO_TypeDef * const p_gpio, GpioPin_t pin, GpioMode_t mode, GpioConfig_t conf)
{
    /* Each GPIO pin mode setting field has 4 bits */
    const u32_t BITS_PER_PIN = 4;

    /* Mask for a pin's field */
    const u32_t PIN_SETTINGS_MASK = 0xF;

    /* Combine the mode and configuration data per the reference manual. The
       masking is not necessary if the caller passes the enums, but we do it 
       just to be explicit. */
    const u32_t pin_settings = ((conf & 0x3) << 2) | (mode & 0x3);

    /* When updating configuration for pins 0 - 7, the bit offsets are treated
       normally, but when updating pins 8 - 15, pin 8 is treated as pin 0 and 
       so on. */
    const GpioPin_t pin_offset = (pin <= GPIO_PIN_7) ? pin              :
                                                       pin - GPIO_PIN_8 ;

    /* Shift the pin configuration settings to the appropriate bit position. */
    const u32_t new_config = (pin_settings << (pin_offset * BITS_PER_PIN));

    /* Create the mask for this pins configuration */
    const u32_t new_config_mask = (PIN_SETTINGS_MASK << (pin_offset * BITS_PER_PIN));

    /* Configure the appropriate register based on the pin number. */
    if (pin <= GPIO_PIN_7) {
        p_gpio->CRL &= ~(new_config_mask);
        p_gpio->CRL |= new_config;
    } else {
        p_gpio->CRH &= ~(new_config_mask);
        p_gpio->CRH |= new_config;
    }
}

void gpio_write_port(GPIO_TypeDef * const p_gpio, GpioData_t data)
{
    const u32_t set_bits = data;
    const u32_t clr_bits = (~set_bits) & 0xFFFF; /* Only 16 bits in GPIO */

    p_gpio->BSRR = (clr_bits << GPIO_BSRR_BR0_Pos) |
                   (set_bits << GPIO_BSRR_BS1_Pos) ;
}

void gpio_write_pin(GPIO_TypeDef * const p_gpio, GpioPin_t pin, bit_t bit)
{
    const u32_t bit_offset = (E_BIT_1 == bit) ? GPIO_BSRR_BS0_Pos :
                                                GPIO_BSRR_BR0_Pos ;

    p_gpio->BSRR = (1 << (pin + bit_offset));
}

GpioData_t gpio_read_port(const GPIO_TypeDef * const p_gpio)
{
    return p_gpio->IDR;
}

bit_t gpio_read_pin(const GPIO_TypeDef * const p_gpio, GpioPin_t pin)
{
    const GpioData_t pin_mask  = (1 << pin);
    const GpioData_t port_data = gpio_read_port(p_gpio);

    return (0 == (pin_mask & port_data)) ? E_BIT_0 :
                                           E_BIT_1 ;
}

/*
 * Enable the given GPIO port peripheral clock in the RCC module.
 */
static void enable_gpio_clk(const GPIO_TypeDef * const port)
{
    const u32_t en = port == GPIOA ? RCC_APB2ENR_IOPAEN :
                     port == GPIOB ? RCC_APB2ENR_IOPBEN :
                     port == GPIOC ? RCC_APB2ENR_IOPCEN :
                     port == GPIOD ? RCC_APB2ENR_IOPDEN :
                     port == GPIOE ? RCC_APB2ENR_IOPEEN :
                                     0                  ;

    RCC->APB2ENR |= en;
}