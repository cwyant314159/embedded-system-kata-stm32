#ifndef SYS_TICK_H
#define SYS_TICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* The clock for the system tick can either be the CPU clock frequency or the
   CPU clock frequency divided by 8. In most cases, the divide by 8 is desired
   to get a higher dynamic range on the possible periods of the system tick.
   This #define can be used by calling code for tick calculations to abstract
   this detail away. */
#define SYS_TICK_CLK_DIV    (8U)

/* The only valid values for the SYS_TICK_CLK_DIV is 8 and 1 */
#if 8 != SYS_TICK_CLK_DIV && 1 != SYS_TICK_CLK_DIV
    #error "Invalid SYS_TICK_CLK_DIV value"
#endif

/* The system tick's load register is only 24 bits wide. This limits the maximum
   possible tick (load value) to 0xFFFFFF + 1. The plus one accounts for the
   minus 1 needed for updating the load register */
#define SYS_TICK_MAX_TICK_VAL (0xFFFFFF + 1)

void sys_tick_init(void);
void sys_tick_set_ticks(u32_t ticks);
void sys_tick_enable(enable_t en);
void sys_tick_set_callback(IsrCallback_t cb);

#ifdef __cplusplus
}
#endif

#endif /* SYS_TICK_H */
