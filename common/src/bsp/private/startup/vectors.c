
#include "bsp/private/startup/vectors.h"

#include "stm32f1xx.h"

#include "bsp/private/startup/crt0.h"
#include "types.h"

/* The CPU fetches the top-of-stack value from address 0x00000000 (start of the
   ISR vectors section), then starts code execution starting from 0x0000 0004
   (Reset_Handler). See section 3.4 of the processor reference manual for more
   information. */
extern u32_t ld__estack;

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
*/
void Reset_Handler(void)
{
    startup_c__();
}


/**
 * @brief Unimplemented ISR handler
 *
 * This is the handler that gets called when the processor receives an
 * unexpected interrupt.  This simply enters an infinite loop, preserving
 * the system state for examination by a debugger.
 */
void Default_Handler(void)
{
    while(1) {
        __BKPT(0);
    }
}

/** Convenience define for creating weak aliases to the Default_Handler */
#define DEFAULT __attribute__ ((weak, alias ("Default_Handler")))

/* Create a weak alias to all possible ISRs. User applications will override
   these aliases. */
DEFAULT void NMI_Handler(void);
DEFAULT void HardFault_Handler(void);
DEFAULT void MemManage_Handler(void);
DEFAULT void BusFault_Handler(void);
DEFAULT void UsageFault_Handler(void);
DEFAULT void SVC_Handler(void);
DEFAULT void DebugMon_Handler(void);
DEFAULT void PendSV_Handler(void);
DEFAULT void SysTick_Handler(void);
DEFAULT void WWDG_IRQHandler(void);
DEFAULT void PVD_IRQHandler(void);
DEFAULT void TAMPER_IRQHandler(void);
DEFAULT void RTC_IRQHandler(void);
DEFAULT void FLASH_IRQHandler(void);
DEFAULT void RCC_IRQHandler(void);
DEFAULT void EXTI0_IRQHandler(void);
DEFAULT void EXTI1_IRQHandler(void);
DEFAULT void EXTI2_IRQHandler(void);
DEFAULT void EXTI3_IRQHandler(void);
DEFAULT void EXTI4_IRQHandler(void);
DEFAULT void DMA1_Channel1_IRQHandler(void);
DEFAULT void DMA1_Channel2_IRQHandler(void);
DEFAULT void DMA1_Channel3_IRQHandler(void);
DEFAULT void DMA1_Channel4_IRQHandler(void);
DEFAULT void DMA1_Channel5_IRQHandler(void);
DEFAULT void DMA1_Channel6_IRQHandler(void);
DEFAULT void DMA1_Channel7_IRQHandler(void);
DEFAULT void ADC1_2_IRQHandler(void);
DEFAULT void USB_HP_CAN1_TX_IRQHandler(void);
DEFAULT void USB_LP_CAN1_RX0_IRQHandler(void);
DEFAULT void CAN1_RX1_IRQHandler(void);
DEFAULT void CAN1_SCE_IRQHandler(void);
DEFAULT void EXTI9_5_IRQHandler(void);
DEFAULT void TIM1_BRK_IRQHandler(void);
DEFAULT void TIM1_UP_IRQHandler(void);
DEFAULT void TIM1_TRG_COM_IRQHandler(void);
DEFAULT void TIM1_CC_IRQHandler(void);
DEFAULT void TIM2_IRQHandler(void);
DEFAULT void TIM3_IRQHandler(void);
DEFAULT void TIM4_IRQHandler(void);
DEFAULT void I2C1_EV_IRQHandler(void);
DEFAULT void I2C1_ER_IRQHandler(void);
DEFAULT void I2C2_EV_IRQHandler(void);
DEFAULT void I2C2_ER_IRQHandler(void);
DEFAULT void SPI1_IRQHandler(void);
DEFAULT void SPI2_IRQHandler(void);
DEFAULT void USART1_IRQHandler(void);
DEFAULT void USART2_IRQHandler(void);
DEFAULT void USART3_IRQHandler(void);
DEFAULT void EXTI15_10_IRQHandler(void);
DEFAULT void RTC_Alarm_IRQHandler(void);
DEFAULT void USBWakeUp_IRQHandler(void);

/* Datasheet says these exist, but the CMSIS IRQ definitions stop before these
   IRQ definitions. */
#if 0
DEFAULT void TIM8_BRK_IRQHandler_IRQHandler(void);
DEFAULT void TIM8_UP_IRQHandler(void);
DEFAULT void TIM8_TRG_COM_IRQHandler(void);
DEFAULT void TIM8_CC_IRQHandler(void);
DEFAULT void ADC3_IRQHandler(void);
DEFAULT void FSMC_IRQHandler(void);
DEFAULT void SDIO_IRQHandler(void);
DEFAULT void TIM5_IRQHandler(void);
DEFAULT void SPI3_IRQHandler(void);
DEFAULT void UART4_IRQHandler(void);
DEFAULT void UART5_IRQHandler(void);
DEFAULT void TIM6_IRQHandler(void);
DEFAULT void TIM7_IRQHandler(void);
DEFAULT void DMA2_Channel1_IRQHandler(void);
DEFAULT void DMA2_Channel2_IRQHandler(void);
DEFAULT void DMA2_Channel3_IRQHandler(void);
DEFAULT void DMA2_Channel4_5_IRQHandler(void);
#endif

/* The pedantic warning flags do not like that the ISR function addresses are
   being casted to void pointers. These pragmas temporarily silence the pedantic
   warnings. This is the CMSIS standard way of defining the vector table. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/** NVIC vector table as defined in Section 10 Table 63 of the reference manual. */
static const void* system_vector_table[75] __attribute__ ((used, section(".isr_vector"))) = {
    /* 0x000 */ &ld__estack,
    /* 0x004 */ Reset_Handler,
    /* 0x008 */ NMI_Handler,
    /* 0x00C */ HardFault_Handler,
    /* 0x010 */ MemManage_Handler,
    /* 0x014 */ BusFault_Handler,
    /* 0x018 */ UsageFault_Handler,
    /* 0x01C */ (void*)0,
    /* 0x020 */ (void*)0,
    /* 0x024 */ (void*)0,
    /* 0x028 */ (void*)0,
    /* 0x02C */ SVC_Handler,
    /* 0x030 */ DebugMon_Handler,
    /* 0x034 */ (void*)0,
    /* 0x038 */ PendSV_Handler,
    /* 0x03C */ SysTick_Handler,
    /* 0x040 */ WWDG_IRQHandler,
    /* 0x044 */ PVD_IRQHandler,
    /* 0x048 */ TAMPER_IRQHandler,
    /* 0x04C */ RTC_IRQHandler,
    /* 0x050 */ FLASH_IRQHandler,
    /* 0x054 */ RCC_IRQHandler,
    /* 0x058 */ EXTI0_IRQHandler,
    /* 0x05C */ EXTI1_IRQHandler,
    /* 0x060 */ EXTI2_IRQHandler,
    /* 0x064 */ EXTI3_IRQHandler,
    /* 0x068 */ EXTI4_IRQHandler,
    /* 0x06C */ DMA1_Channel1_IRQHandler,
    /* 0x070 */ DMA1_Channel2_IRQHandler,
    /* 0x074 */ DMA1_Channel3_IRQHandler,
    /* 0x078 */ DMA1_Channel4_IRQHandler,
    /* 0x07C */ DMA1_Channel5_IRQHandler,
    /* 0x080 */ DMA1_Channel6_IRQHandler,
    /* 0x084 */ DMA1_Channel7_IRQHandler,
    /* 0x088 */ ADC1_2_IRQHandler,
    /* 0x08C */ USB_HP_CAN1_TX_IRQHandler,
    /* 0x090 */ USB_LP_CAN1_RX0_IRQHandler,
    /* 0x094 */ CAN1_RX1_IRQHandler,
    /* 0x098 */ CAN1_SCE_IRQHandler,
    /* 0x09C */ EXTI9_5_IRQHandler,
    /* 0x0A0 */ TIM1_BRK_IRQHandler,
    /* 0x0A4 */ TIM1_UP_IRQHandler,
    /* 0x0A8 */ TIM1_TRG_COM_IRQHandler,
    /* 0x0AC */ TIM1_CC_IRQHandler,
    /* 0x0B0 */ TIM2_IRQHandler,
    /* 0x0B4 */ TIM3_IRQHandler,
    /* 0x0B8 */ TIM4_IRQHandler,
    /* 0x0BC */ I2C1_EV_IRQHandler,
    /* 0x0C0 */ I2C1_ER_IRQHandler,
    /* 0x0C4 */ I2C2_EV_IRQHandler,
    /* 0x0C8 */ I2C2_ER_IRQHandler,
    /* 0x0CC */ SPI1_IRQHandler,
    /* 0x0D0 */ SPI2_IRQHandler,
    /* 0x0D4 */ USART1_IRQHandler,
    /* 0x0D8 */ USART2_IRQHandler,
    /* 0x0DC */ USART3_IRQHandler,
    /* 0x0E0 */ EXTI15_10_IRQHandler,
    /* 0x0E4 */ RTC_Alarm_IRQHandler,
    /* 0x0E8 */ USBWakeUp_IRQHandler,

/* Datasheet says these exist, but the CMSIS IRQ definitions stop before these
   IRQ definitions. */
#if 0
    /* 0x0EC */ TIM8_BRK_IRQHandler_IRQHandler,
    /* 0x0F0 */ TIM8_UP_IRQHandler,
    /* 0x0F4 */ TIM8_TRG_COM_IRQHandler,
    /* 0x0F8 */ TIM8_CC_IRQHandler,
    /* 0x0FC */ ADC3_IRQHandler,
    /* 0x100 */ FSMC_IRQHandler,
    /* 0x104 */ SDIO_IRQHandler,
    /* 0x108 */ TIM5_IRQHandler,
    /* 0x10C */ SPI3_IRQHandler,
    /* 0x110 */ UART4_IRQHandler,
    /* 0x114 */ UART5_IRQHandler,
    /* 0x118 */ TIM6_IRQHandler,
    /* 0x11C */ TIM7_IRQHandler,
    /* 0x120 */ DMA2_Channel1_IRQHandler,
    /* 0x124 */ DMA2_Channel2_IRQHandler,
    /* 0x128 */ DMA2_Channel3_IRQHandler,
    /* 0x12C */ DMA2_Channel4_5_IRQHandler,
#endif
};
#pragma GCC diagnostic pop
