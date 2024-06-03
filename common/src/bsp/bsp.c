#include "bsp/bsp.h"

#include "stm32f1xx.h"

#include "bsp/private/sys_tick/sys_tick.h"

// #include <avr/interrupt.h>
// #include "bsp/private/processor/reg_io.h"
// #include "bsp/private/timer/timer.h"
// #include "bsp/private/uart/uart.h"

/* GPIO definitions for builtin LED */
#define LED_PORT        (GPIOC)
#define LED_PIN         (13u)
#define LED_PIN_MASK    (1 << LED_PIN)

/* Conversions for sys_tick configuration */
#define SEC_PER_SEC     (1U)
#define MSEC_PER_SEC    (1000U)
#define USEC_PER_SEC    (1000000U)

static void enable_gpio_clk(const GPIO_TypeDef * const port);
static void configure_gpio_pin_output(GPIO_TypeDef * const port, u32_t pin);
static bool_t update_sys_tick_period(u32_t duration, u32_t conversion_factor);

/**
 * @brief BSP initialization
 * 
 * This function is responsible for initializing low level hardware and any
 * board specific services needed by the application.
 * 
 * @note This function does not enable interrupts.
 */
void bsp_init(void)
{
    enable_gpio_clk(LED_PORT);                      /* enable clock to GPIO   */
    configure_gpio_pin_output(LED_PORT, LED_PIN);   /* set LED as output      */
    bsp_set_builtin_led(E_OFF);                     /* LED is off on startup  */
    sys_tick_init();                                /* setup the sys tick     */
    // uart_init();
}

/**
 * @brief Enable interrupts on the processor
 * 
 * To insulate the application from details about the processor, the global
 * interrupt enable is put behind this interface function.
 */
void bsp_enable_interrupts(void)
{
    // sei(); /* interrupt enable from the AVR header */
}

/**
 * @brief Toggle the board's builtin LED.
 */
void bsp_toggle_builtin_led(void)
{
    if ((LED_PORT->ODR & LED_PIN_MASK) == 0x00) {
        bsp_set_builtin_led(E_OFF);
    } else {
        bsp_set_builtin_led(E_ON);
    }
}

/**
 * @brief Turn the board's builtin LED on or off
 * 
 * @param[in] led_state desired on/off state
 */
void bsp_set_builtin_led(on_off_t led_state)
{
    u32_t bit_offset;

    /*
     * The BSRR is used to atomically set and reset the pins of the port. The
     * upper 16 bits reset the corresponding pin when written with a 1. The 
     * lower 16 bits set the corresponding pin.
     */
    if (E_OFF == led_state) {
        bit_offset = 0;
    } else {
        bit_offset = 16;
    }

    LED_PORT->BSRR |= 1 << (LED_PIN + bit_offset);
}

/**
 * @brief Read a byte from the serial driver
 * 
 * @param[out] byte output read from serial port
 * 
 * @retval E_TRUE  - successfully read byte
 * @retval E_FALSE - unable to read byte
 */
bool_t bsp_serial_read(u8_t * const byte)
{
    bool_t result;

    result = E_FALSE;
    // if ((NULL_PTR != byte) && (E_TRUE == uart_data_available())) {
    //     *byte = uart_read();
    //     result = E_TRUE;
    // }

    return result;
}

/**
 * @brief Write a byte to the serial driver
 * 
 * @param[in] byte input to write to UART
 * 
 * @retval E_TRUE  - successfully write byte
 * @retval E_FALSE - unable to write byte
 */
bool_t bsp_serial_write(u8_t byte)
{
    // return uart_write(byte);
    return E_TRUE;
}

/**
 * @brief Write a C-style string out the serial port.
 * 
 * @param[in] c_str null terminated string to output over serial
 * 
 * @retval E_TRUE  - successfully wrote string to serial driver
 * @retval E_FALSE - serial driver encountered an error during write
 */
bool_t bsp_serial_write_c_str(const char* c_str)
{
    // const char *p_c;
    bool_t      result;

    // p_c    = c_str;
    result = E_TRUE;
    
    // while('\0' != *p_c) {
    //     result = bsp_serial_write(*p_c);
    //     p_c += 1;

    //     /* If an error occurs, don't bother sending the rest of the string. */
    //     if (E_FALSE == result) {
    //         break;
    //     }
    // }

    return result;
}

/**
 * @brief Set the BSP's system tick interrupt callback.
 * 
 * @param[in] cb user supplied callback to handle BSP system tick interrupts.
 */
void bsp_register_sys_tick_callback(IsrCallback_t cb)
{
    sys_tick_set_callback(cb);
}

/**
 * @brief Set the BSP timer's period in microseconds and start running.
 * 
 * @param[in] usec timer interval in microseconds
 * 
 * @retval E_TRUE  - successfully configured the timer
 * @retval E_FALSE - failed to set timer's interval
 */
bool_t bsp_set_sys_tick_period_uses(u32_t usec)
{
    return update_sys_tick_period(usec, USEC_PER_SEC);
}

/**
 * @brief Set the BSP timer's period in milliseconds and start running.
 * 
 * @param[in] msec timer interval in milliseconds
 * 
 * @retval E_TRUE  - successfully configured the timer
 * @retval E_FALSE - failed to set timer's interval
 */
bool_t bsp_set_sys_tick_period_msec(u32_t msec)
{
    return update_sys_tick_period(msec, MSEC_PER_SEC);
}

/**
 * @brief Set the BSP timer's period in seconds and start running.
 * 
 * @param[in] sec timer interval in seconds
 * 
 * @retval E_TRUE  - successfully configured the timer
 * @retval E_FALSE - failed to set timer's interval
 */
bool_t bsp_set_sys_tick_period_sec(u32_t sec)
{
    return update_sys_tick_period(sec, SEC_PER_SEC);
}

/**
 * @brief Busy loop (blocking) delay
 * 
 * Using two loops this function is able to produce a human perceptable delay.
 * There has been no measurement to determine the amount of delay time. Even
 * with an iteration count 1, the delay is observable.
 * 
 * @param[in] iter number of times to sit in the spin loop.
 */
void bsp_spin_delay(size_t iter)
{
    for (volatile size_t i = iter; i != 0u; i -= 1u) {
        for (volatile size_t j = (size_t)0xFFFFu; j != 0u; j -= 1u);
    }
}

/**
 * @brief Fatal error trap
 * 
 * When the processor encounters a fatal error there isn't really anything that
 * can be done. For this project, a while loop flashing the LED is the path
 * forward. The intent is that fatal errors are programming bugs and developer
 * intervention/reset are required to remediate the situation.
 * 
 * @param[in] iter number of times to sit in the spin loop.
 */
void bsp_error_trap(void)
{
    while(1) {
        bsp_toggle_builtin_led();
        bsp_spin_delay(5);
    }
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

/*
 * Configure the given GPIO pin as a push-pull fast output.
 */
static void configure_gpio_pin_output(GPIO_TypeDef * const port, u32_t pin)
{
    /*
     * Exit the function early if the pin number is too high. 
     */
    if (pin > 15) return;

    /*
     * The GPIO pin configuration is distributed as 4 bits per pin in two
     * registers. For GPIO pins 0-7 the CRL register is used. Pins 8-15 are
     * configured in CRH. The offset for a given GPIO pins configuration
     * register can be computed with the following formula:
     *
     *  offset = (pin % 8) * 4
     * 
     * or without using divides and multiplies
     * 
     *  offset = (pin & 7) << 2;
     */
    const u32_t offset = (pin & 7) << 2;

    /* 
     * For push-pull outputs, the upper 2 configuration bits are 0x00. To
     * enable fast mode on a pin the lower 2 configuration bits are 0x11;
     */
    const u32_t pin_cfg = 0x0011;
    
    /*
     * Configure either the high or low configuration register based on the pin
     * number.
     */
    const u32_t mask = 0xFFFF  << offset;
    const u32_t cfg  = pin_cfg << offset;
    if (pin <= 7) {
        port->CRL = (port->CRL & ~mask) | cfg;
    } else {
        port->CRH = (port->CRH & ~mask) | cfg;
    }

    /*
     * Default the newly configured pins to low. The atomic set and clear
     * registers has the IO clear bits in the upper 16 bits.
     */
    port->BSRR = 1 << (pin + 16);
}

/* 
 * Update the system tick to the desired duration given the duration conversion
 * factor (e.g. milliseconds per second, microseconds per second, etc).
 */
static bool_t update_sys_tick_period(u32_t duration, u32_t conversion_factor)
{
    bool_t result;
    u32_t  conversion;
    u32_t  ticks;

    /* Compute the conversion from CPU frequency to ticks for the provided 
       conversion factor. To prevent a divide by 0 error, that case forces the
       conversion to evaluate to 0, so later logic can terminate gracefully. */
    if (0 == conversion_factor) {
        conversion = 0;
    } else {
        conversion = (F_CPU_HZ / SYS_TICK_CLK_DIV) / conversion_factor;
    }

    /* Convert the duration into CPU ticks */
    ticks = duration * conversion;

    /* If the number of ticks is within the system tick's limits, turn off the
       sys_tick and update the period. Otherwise, leave the system tick alone
       and report a failure. 
       
       The first conditional handles the case where ticks is computed to be 0.
       This can happen for several reasons:
        - The conversion factor argument was 0
        - The integer division of the conversion evalutated to 0
        - The caller passed in a duration argument of 0
       */
    if (0 == ticks) {
        result = E_FALSE;
    } else if (SYS_TICK_MAX_TICK_VAL >= ticks) {
        sys_tick_enable(E_DISABLE);
        sys_tick_set_ticks(ticks);
        result = E_TRUE;
    } else {
        result = E_FALSE;
    }

    /* Even if the period update failed, re-enable the system tick. If the it is
       already running, this will do nothing. */
    sys_tick_enable(E_ENABLE);
    return result;
}