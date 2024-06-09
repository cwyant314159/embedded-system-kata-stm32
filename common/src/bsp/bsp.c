#include "bsp/bsp.h"

#include "stm32f1xx.h"

#include "bsp/sw_timers.h"
#include "bsp/private/gpio/gpio.h"
#include "bsp/private/sys_tick/sys_tick.h"
#include "bsp/private/uart/uart.h"

/* TODO come up with table driven scheme for GPIO */

/* GPIO definitions for builtin LED */
#define LED_PORT        (GPIOC)
#define LED_PIN         (GPIO_PIN_13)
#define LED_PIN_MODE    (GPIO_MODE_OUTPUT_50MHZ)
#define LED_PIN_CONF    (GPIO_CONF_OUT_GENERAL_PUSH_PULL)
#define LED_ON_BIT      (E_BIT_0)
#define LED_OFF_BIT     (E_BIT_1)

/* Serial port definitions. */
#define SERIAL_DEV          (USART1)

/* GPIO definitions for USART 1 */
#define SERIAL_PORT         (GPIOA)
#define SERIAL_TX_PIN       (GPIO_PIN_9)
#define SERIAL_TX_PIN_MODE  (GPIO_MODE_OUTPUT_50MHZ)
#define SERIAL_TX_PIN_CONF  (GPIO_CONF_OUT_ALT_PUSH_PULL)
#define SERIAL_RX_PIN       (GPIO_PIN_10)
#define SERIAL_RX_PIN_MODE  (GPIO_MODE_INPUT)
#define SERIAL_RX_PIN_CONF  (GPIO_CONF_IN_PUP_PUD)



/* Conversions for sys_tick configuration */
#define SEC_PER_SEC     (1U)
#define MSEC_PER_SEC    (1000U)
#define USEC_PER_SEC    (1000000U)

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
    /* Configure the builtin LED GPIO (default to LED off) */
    gpio_init(LED_PORT);
    gpio_set_mode(LED_PORT, LED_PIN, LED_PIN_MODE,
        LED_PIN_CONF);
    bsp_set_builtin_led(E_OFF);

    /* Configure the USART1 (serial port) GPIO. Enable the pull-up on the USART
       RX pin. */
    gpio_init(SERIAL_PORT);
    gpio_set_mode(SERIAL_PORT, SERIAL_TX_PIN, SERIAL_TX_PIN_MODE,
        SERIAL_TX_PIN_CONF);
    gpio_set_mode(SERIAL_PORT, SERIAL_RX_PIN, SERIAL_RX_PIN_MODE,
        SERIAL_RX_PIN_CONF);
    gpio_write_pin(SERIAL_PORT, SERIAL_RX_PIN, E_BIT_1);
    
    /* Initialize the UART hardware. */
    uart_init(SERIAL_DEV);

    /* Initialize the system tick hardware */
    sys_tick_init();

    /* Initialize the software timer facility */
    sw_timer_init();
}

/**
 * @brief Enable interrupts on the processor
 *
 * To insulate the application from details about the processor, the global
 * interrupt enable is put behind this interface function.
 */
void bsp_enable_interrupts(void)
{
    /* NOP for ARM processors */
}

/**
 * @brief Toggle the board's builtin LED.
 */
void bsp_toggle_builtin_led(void)
{
    if (LED_ON_BIT == gpio_read_pin(LED_PORT, LED_PIN)) {
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
    const bit_t led_bit_state = (E_ON == led_state) ? LED_ON_BIT  :
                                                      LED_OFF_BIT ;
    gpio_write_pin(LED_PORT, LED_PIN, led_bit_state);
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
    if ((NULL_PTR != byte) && (E_TRUE == uart_data_available(SERIAL_DEV))) {
        *byte = uart_read(SERIAL_DEV);
        result = E_TRUE;
    }

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
    return uart_write(SERIAL_DEV, byte);
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
    const char *p_c;
    bool_t      result;

    p_c    = c_str;
    result = E_TRUE;

    while('\0' != *p_c) {
        result = bsp_serial_write(*p_c);
        p_c += 1;

        /* If an error occurs, don't bother sending the rest of the string. */
        if (E_FALSE == result) {
            break;
        }
    }

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