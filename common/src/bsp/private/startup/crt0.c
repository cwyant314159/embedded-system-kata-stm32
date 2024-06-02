#include "bsp/private/startup/crt0.h"

#include "stm32f1xx.h"

#include "bsp/bsp.h"
#include "types.h"

#ifndef F_CPU_HZ
    #error "Define F_CPU_HZ with the system's CPU frequency"
#endif

#ifndef F_HSE_HZ
    #error "Define F_HSE_HZ with the HSE oscillator frequency"
#endif

#ifndef F_HSI_HZ
    #error "Define F_HSI_HZ with the HSI oscillator frequency"
#endif

#ifndef F_LSE_HZ
    #error "Define F_LSE_HZ with the LSE oscillator frequency"
#endif

#ifndef F_LSI_HZ
    #error "Define F_LSI_HZ with the LSI oscillator frequency"
#endif

#if F_HSE_HZ != 8000000
    #error "HSE oscillator changed. Verify clock configuration"
#endif

extern u32_t ld__sdata;     /* RAM DATA section first entry */
extern u32_t ld__edata;     /* RAM DATA section one past last entry */
extern u32_t ld__sidata;    /* flash DATA section first entry */

extern u32_t ld__sbss;      /* BSS section first entry */
extern u32_t ld__ebss;      /* BSS section one past last entry */

extern u32_t __preinit_array_start;
extern u32_t __preinit_array_end;

extern u32_t __init_array_start;
extern u32_t __init_array_end;

typedef void (*fpCtor)(void);       /* pointer to C++ constructor */

extern int main(void);              /* the main function */

static void clock_init(void);
static void sys_control_block_init(void);

/**
 * @brief Startup C routine
 *
 * This routine is called from the assembly startup code and expects that prior
 * to being called the R1 register is 0, the SREG has been cleared, and that the
 * stack pointer has been configured.
 *
 * The startup C routine is responsible for copying data from flash to the DATA
 * section of RAM, clearing (zeroing) the BSS section, and calling global/static
 * C++ constructors.
 *
 * Once the RAM and constructors are initialized this routine will call the main
 * function. If the main function returns, execution is trapped in an infinite
 * loop.
 */
void startup_c__(void)
{
    /*
     * Initialize the DATA section with data from flash.
     */
    u32_t * const data_begin      = (u32_t*)&ld__sdata;
    u32_t * const data_end        = (u32_t*)&ld__edata;
    const u32_t* load_data = (u32_t*)&ld__sidata;

    for (u32_t* data_ptr = data_begin; data_ptr != data_end; data_ptr += 1) {
        *data_ptr = *load_data;
        load_data++;
    }

    /* Zero initialize the BSS section */
    u32_t * const bss_begin = (u32_t*)&ld__sbss;
    u32_t * const bss_end   = (u32_t*)&ld__ebss;

    for (u32_t* bss_ptr = bss_begin; bss_ptr != bss_end; bss_ptr += 1) {
        *bss_ptr = 0u;
    }

    /*
     * With the RAM initialized, we can do some low level initialization that
     * may or may not rely on values in the BSS or DATA segment.
     */
    clock_init();
    sys_control_block_init();

    /*
     * Iterate over the preinit array and the init array (C++ constructors)
     * in flash and call them.
     */
    
    u32_t * const preinit_begin = (u32_t*)&__preinit_array_start;
    u32_t * const preinit_end   = (u32_t*)&__preinit_array_end;
    for (u32_t* c = preinit_begin; c != preinit_end; c += 1) {
        fpCtor ctor = (fpCtor)(*c);
        ctor();
    }

    u32_t * const init_begin = (u32_t*)&__init_array_start;
    u32_t * const init_end   = (u32_t*)&__init_array_end;
    for (u32_t* c = init_begin; c != init_end; c += 1) {
        fpCtor ctor = (fpCtor)(*c);
        ctor();
    }

    /* Application main */
    main();

    /* Trap execution if main ever returns */
    while(1) {  }
}

/**
 * @brief Configure the system clock tree
 * 
 * This system uses the HSE (8MHz) to feed the PLL which will be used as the 
 * system clock.
 * 
 * The application will run at 72MHz once clock configuration is complete.
 * All busses in the design will be configured to run at at their full
 * speed. The table below summarizes the clock sources, clock source
 * dividers, resultant clock speed, and resultant clock names.
 *
 * bus  | source | div | MHz | Name
 * -----+--------+-----+-----+-------
 * AHB  | SYSCLK |  1  | 72  | HCLK
 * APB1 | HCLK   |  2  | 36  | PCLK1
 * APB2 | HCLK   |  1  | 72  | PCLK2
 * ADC  | PCLK2  |  6  | 12  | ADCCLK
 * 
 * Based on the above table, the clock to timers 2-7 and 12-14 is 72Mhz
 * (PCLK1 x 2). The clock to timers 1 and 8-11 is 72MHz (PCLK2 x 1).
 *
 * Even though USB may not be used, the USB prescaler is configured to give the
 * appropriate USB clock (48MHz).
 */
static void clock_init(void)
{
    /*
     * Configure the RCC control register to turn on HSE oscillator. To prevent
     * system glitches while setting up the clocks, the HSI (default)
     * oscillator is also enabled. The default value for the HSI trim (0x10) is
     * also added to prevent potential glitches. See section 7.3.1 in the
     * processor reference manual for more info.
     */
    RCC->CR = RCC_CR_HSEON | RCC_CR_HSION | (0x10 << RCC_CR_HSITRIM_Pos);

    /*
     * Wait for the HSERDY bit to be set in the RCC control register. Normally
     * it is bad form to do sit in a while loop monitoring a register without
     * any kind of timeout, but for this demonstration project, it will be good
     * enough.
     */
    while((RCC->CR & RCC_CR_HSERDY) == 0) { }

    /*
     * Configure the bus clock dividers in the RCC configuration register. To
     * be extra safe, bus clock dividers are configured separate from PLL
     * configuration. This is done to ensure that bus frequencies are for sure
     * below f_MAX.
     */
    RCC->CFGR = RCC_CFGR_USBPRE | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV6;

    /*
     * Configure the PLL parameters in the RCC configuration register. This
     * sets the PLL source to the undivided HSE clock (8Mhz). The PLL
     * multiplier is set to 9 to give the maximum clock speed of 72MHz.
     */
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    /*
     * Turn on the PLL by setting the PLLON bit of the RCC control register.
     */
    RCC->CR |= RCC_CR_PLLON;

    /*
     * Wait for the PLLRDY bit to be set in the RCC control register.
     */
    while((RCC->CR & RCC_CR_PLLRDY) == 0) { }

    /*
     * Since the SYSCLK is being changed, the latency bits in the flash access
     * control register must be modified. With a SYSCLK of 72MHz, two wait
     * states are required for flash access. See section 3.3.3 in the processor
     * reference manual for more information.
     *
     * Since the default configuration has the prefetch buffer enabled, the
     * buffer enable is set as well.
     *
     * There is a bug in the HAL FLASH_ACR_LATENCY_2 is shifted 1 too many
     * bits.
     */
    FLASH->ACR = FLASH_ACR_PRFTBE | (FLASH_ACR_LATENCY_2 >> 1);

    /*
     * Now that the bus dividers and PLL are configured, the system clock can
     * be changed from the HSI to the PLL output in the RCC configuration
     * register.
     *
     * Care must be taken not to clear any previously configured bits.
     */
    RCC->CFGR |= (RCC->CFGR & RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    /*
     * Wait for the clock switch status to indicate the PLL is the system
     * clock.
     */
    while((RCC->CR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) { }

    /*
     * Disable the HSI since it is no longer in use.
     */
    RCC->CR &= ~(RCC_CR_HSION);
}

/** 
 * @brief System control block initial configuration
 * 
 * This function configures the System Control Block to give the maximum amount
 * of bits to the interrupt group priority. See section 4.4.5 of the processor
 * programming manual for more information.
 *
 * To get the appropriate priority group bits, the value 0x3 must be
 * written to bits [10:8]
 *
 * A special value of 0x05FA must be written to the upper 16 bits for the
 * register to update.
 */
static void sys_control_block_init(void)
{
    NVIC_SetPriorityGrouping(0x3);
}

/*
 * This function is called by the Reset_Handler after initializing the data
 * and BSS sections. The static constructors have not been called yet, so it
 * is not safe to access any global C++ objects.
 *
 * When not using the HAL, this function is the perfect place for clock setup,
 * IO configuration/assignments, and peripheral enables. Peripheral
 * initialization and configuration is best left to the main application. This
 * function is very application specific and will likely be changed for every
 * application. What this funciton lacks in flexibility, it makes up for it in
 * readability since we can use the constants and helpers defined in the Bsp
 * namespace.
 * 
 * For the template application not using the HAL, SystemInit will do the
 * following:
 * 
 *      1. Set the system clock frequency to the 72MHz maximum.
 *      2. Disable the HSI oscillator after configuring the system clock.
 *      3. Configure the IRQ priority bit usage.
 *      4. Configure (but not enable) the SysTick to tick at 1ms.
 *      5. Configure the LED's GPIO port.
 * 
 * The SysTick enable is left to the main application since the SysTick ISR may
 * or may not reference global C++ objects.
 */
void SystemInit (void)
{
    /*
     * Set the SysTick interrupt to the lowest priority (highest numerical 
     * value).
     */
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

    /*
     * Configure the SysTick to trigger an interrupt every 1 ms. The system
     * clock is 72MHz. To get a get a 1ms pulse, we need to compute the number
     * of clock ticks in 1ms.
     *
     *  72MHz = 72000000 ticks * 1 s       = 72000
     *          --------------   ---------   -----
     *          1 s              1000 ms     1 ms
     *
     * This means we need to set the SysTick to trigger an interrupt every
     * 72000 clock ticks. To compensate for the zero crossing tick, an extra
     * minus 1 is need.
     */
    const uint32_t ms_ticks = 72000000 / 1000;
    const uint32_t load     = (ms_ticks - 1) & 0xFFFFFF;
    SysTick->LOAD = load;

    /*
     * Write to the VAL register to clear it and the COUNTFLAG in the control
     * register.
     */
    SysTick->VAL = 0;

    /*
     * Configure the control register to enable both the counter and the tick
     * interrupt. The clock source for the SysTick is configured as the
     * processor (AHB) clock.
     */
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
}

