#include "bsp/bsp.h"
#include "types.h"

/**
 * @brief Application system tick interrupt callback
 * 
 * This callback toggles the builtin LED every system tick period.
 */
static void bsp_sys_tick_isr_callback(void)
{
    bsp_toggle_builtin_led();  
}

/**
 * @brief Blinky using timer application
 * 
 * This blinks an LED once a second (500ms on/500ms off) using a hardware timer.
 */
int main(void)
{
    /* Initialize the board support (e.g. the LED) */
    bsp_init();
    
    /* Set the LED's initial state to off */
    bsp_set_builtin_led(E_OFF);

    /* 
     * Register interrupt callbacks and enable processor interupts.
     *
     * Interrupts are enabled now, so when the timer is configured
     * and started the first interrupt is not missed 
     */
    bsp_register_sys_tick_callback(bsp_sys_tick_isr_callback);
    bsp_enable_interrupts();

    /* Set the timers interval so it will start. */
    if (E_FALSE == bsp_set_sys_tick_period_msec(500u)) {
        bsp_error_trap();
    }
    
    while (1) {
        /* Nothing to do */
    }

    return 0; /* Satisfy compiler. Should never get here */
}


