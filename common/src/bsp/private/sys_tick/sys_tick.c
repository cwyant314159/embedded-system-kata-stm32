#include "bsp/private/sys_tick/sys_tick.h"

#include "stm32f1xx.h"

#include "bsp/private/startup/vectors.h"
#include "types.h"

/* The maximum value of the load register is the maximum tick value minus 1. See
   comments in the "set ticks" function for more information. */
#define MAX_LOAD_VALUE  (SYS_TICK_MAX_TICK_VAL-1)

/* Default number of ticks required for sys_tick interrupt */
#define DEFAULT_TICKS (MAX_LOAD_VALUE)

/* Application supplied system tick interrupt callback */
static volatile IsrCallback_t sys_tick_callback = NULL_PTR;

void sys_tick_init(void)
{
    /*
     * Set the SysTick interrupt to the lowest priority (highest numerical
     * value).
     */
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

    /* 
     * Enable the tick interrupt and ensure the clock source for the sys_tick is
     * the system clock divided by 8 if the SYS_TICK_CLK_DIV is set to 8.
     */
    SysTick->CTRL  = 0;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

#if 1 == SYS_TICK_CLK_DIV
    // SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; /* CPU Clk / 1 */
#endif
    
}

void sys_tick_set_ticks(u32_t ticks)
{
    /* To compensate for the zero crossing cclock tick, the ticks value computed
       to achieve a certain period must have a minus 1 */
    const u32_t load = ticks - 1;

    /* Clamp the load register to its maximum value. */
    if (load > MAX_LOAD_VALUE) {
        SysTick->LOAD = MAX_LOAD_VALUE;
    } else {
        SysTick->LOAD = load;
    }

    /* The counter register must be cleared just in case the sys_tick was
       running while the load register was updating.*/
    SysTick->VAL = 0;
}

void sys_tick_enable(enable_t en)
{
    if (E_ENABLE == en) {
        SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
    } else {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    }
}

void sys_tick_set_callback(IsrCallback_t cb)
{
    sys_tick_callback = cb;
}

void SysTick_Handler(void)
{
    if (NULL_PTR != sys_tick_callback) {
        sys_tick_callback();
    }
}