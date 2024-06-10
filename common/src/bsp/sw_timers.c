#include "bsp/sw_timers.h"

#include "stm32f1xx.h"

#include "bsp/bsp.h"
#include "types.h"


#define HW_TIMER                (TIM1)
#define HW_TIMER_RCC_REGISTER   (RCC->APB2ENR)
#define HW_TIMER_RCC_ENABLE     (RCC_APB2ENR_TIM1EN)
#define MAX_SW_TIMERS           (4u)
#define USEC_PER_TIMER_CNT      (500u)
#define TICK_ROLLOVER_OFFSET    (0xFFFF + 1)

typedef struct sw_timer
{
    u32_t sec;
    u32_t msec;
    u32_t usec;
    u32_t  prev_ticks;
} SwTimer_t;


/* Ring buffer infrastructure */
#define PRIVATE_RING_SIZE MAX_SW_TIMERS
#include "utils/private_ring.h"

PRIVATE_RING_DECLARATIONS(SwTimers, SwTimerHandle_t) /* create ring type of timer handles */
PRIVATE_RING_DECLARE(static SwTimers, handle_ring);  /* instances of timer handle ring    */
static SwTimer_t timer_mem[MAX_SW_TIMERS];           /* backing storage for handles       */

/* Readability macros for private ring functions */
#define TIMER_RING_INIT(var_name)       PRIVATE_RING_INIT(SwTimers, var_name)
#define TIMER_RING_IS_EMPTY(var_name)   PRIVATE_RING_IS_EMPTY(SwTimers, var_name)
#define TIMER_RING_IS_FULL(var_name)    PRIVATE_RING_IS_FULL(SwTimers, var_name)
#define TIMER_RING_PUSH(var_name, data) PRIVATE_RING_PUSH(SwTimers, var_name, data)
#define TIMER_RING_POP(var_name)        PRIVATE_RING_POP(SwTimers, var_name)
#define TIMER_RING_PEEK(var_name)       PRIVATE_RING_PEEK(SwTimers, var_name)


static void init_hw_timer(void);

void sw_timer_init(void)
{
    size_t t;

    /* Start the hardware timer */
    init_hw_timer();

    /* Clear out the ring buffer */
    TIMER_RING_INIT(handle_ring);
    
    /* Reset all timer instances and enqueue their handles in the ring */
    for (t = 0; t < MAX_SW_TIMERS; t += 1) {
        sw_timer_reset(&timer_mem[t]);
        TIMER_RING_PUSH(handle_ring, &timer_mem[t]);
    }
}

void sw_timer_task(void)
{
    size_t t;

    /* This task needs to be called as often as possible since our timer is only
       8-bits. It is very likely that overflows will be missed and time keeping
       will be inaccurate if this function is delayed for too long. 
       
       Since this function calls a function with multiplication and division
       with values wider than the native processor width, this can get very
       expensive very fast. The number of software timers must be balanced with 
       execution time of this refresh function. */
    for (t = 0; t < MAX_SW_TIMERS; t += 1) {
        (void)sw_timer_sec(&timer_mem[t]);
    }
}

SwTimerHandle_t sw_timer_acquire(void)
{
    SwTimerHandle_t handle;

    if (E_TRUE == TIMER_RING_IS_EMPTY(handle_ring)) {
        handle  = SW_TIMER_NO_TIMER;
    } else {
        handle = TIMER_RING_POP(handle_ring);
        sw_timer_reset(handle);
    }

    return handle;
}

void sw_timer_reset(SwTimerHandle_t t)
{
    if (SW_TIMER_NO_TIMER != t) {
        /* Initialize non-null handles to 0. Previous ticks is set to properly
           compute deltas from reset. */
        t->msec       = 0u;
        t->sec        = 0u;
        t->usec       = 0u;
        t->prev_ticks = HW_TIMER->CNT;
    }
}

u32_t sw_timer_sec(SwTimerHandle_t t)
{
    /* It is expected that second timers are used for very coarse timings that 
       do not require high accuracy. */
    t->sec = (sw_timer_msec(t) / 1000);
    return t->sec;
}

u32_t sw_timer_msec(SwTimerHandle_t t)
{
    /* It is expected that millsecond timers are used for very coarse timings that 
       do not require high accuracy. */
    t->msec = (sw_timer_usec(t) / 1000);
    return t->msec;
}

u32_t sw_timer_usec(SwTimerHandle_t t)
{
    u32_t usec;
    u32_t lapsed_ticks;
    u32_t curr_ticks;

    if (SW_TIMER_NO_TIMER == t) {
        usec = 0u;
    } else {

        curr_ticks = HW_TIMER->CNT;

        /* When the current ticks is less than the previous ticks, a rollover 
           occurred and an offset of 0xFFFF (ticks to get back to 0) needs to be
           accounted for in the lapsed ticks. */
        if (curr_ticks < t->prev_ticks) {
            lapsed_ticks = (curr_ticks + TICK_ROLLOVER_OFFSET) - t->prev_ticks;
        } else {
            lapsed_ticks = curr_ticks - t->prev_ticks;
        }

        /* Update the previous ticks for next time */
        t->prev_ticks = curr_ticks;

        /* Each tick of the timer is 64 usec. This makes it hard to get accurate
           timings, but for the software timers this is ok. If finer timing
           control is required, a hardware timer will need to be used. */
        t->usec += (lapsed_ticks * USEC_PER_TIMER_CNT);
        
        usec = t->usec;
    }

    return usec;
}

static void init_hw_timer(void)
{
    /*
     * Enable the clock to the timer's IP block
     */
    HW_TIMER_RCC_REGISTER |= HW_TIMER_RCC_ENABLE;

    /*
     * Use TIM1 in up count mode with the timer tick set to 1 tick per 500 usec.
     */
    const u32_t TIMER_FREQ = 2000; /* 2KHz or 500usec period */
    const u16_t PRESCALER  = (F_CPU_HZ / TIMER_FREQ) - 1;
    
    /* Verify the above calculation didn't overflow or cause a zero prescaler */
    const u32_t TIMER_FREQ_VERIFICATION = F_CPU_HZ / (PRESCALER + 1);
    if (TIMER_FREQ_VERIFICATION != TIMER_FREQ) {
        bsp_error_trap();
    }

    HW_TIMER->PSC = PRESCALER;

    /*
     * Configure the timer to overflow at the maximum timer count.
     */
    HW_TIMER->CNT = 0;              /* ensure the count starts at 0          */
    HW_TIMER->ARR = 0xFFFF;         /* maximum counter value                 */
    HW_TIMER->EGR |= TIM_EGR_UG;    /* force update event to load new config */
    HW_TIMER->CR1 = TIM_CR1_CEN;    /* enable timer's counter                */
}