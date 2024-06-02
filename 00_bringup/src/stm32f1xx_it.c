#include "stm32f1xx_it.h"

#include "stm32f1xx.h"

/* non-maskable interrupt handler */
void NMI_Handler(void)
{
    while (1) {
        __BKPT(0);
    }
}

/* hard fault interrupt handler */
void HardFault_Handler(void)
{
    while (1) {
        __BKPT(0);
    }
}

/* memory management fault interrupt handler */
void MemManage_Handler(void)
{
    while (1) {
        __BKPT(0);
    }
}

/* bus fault interrupt handler */
void BusFault_Handler(void)
{
    while (1) { 
        __BKPT(0);
    }
}

/* usage fault interrupt handler */
void UsageFault_Handler(void)
{
    while (1) { 
        __BKPT(0);
    }
}

/* system service call interrupt handler */
void SVC_Handler(void)
{
}

/* debug monitor interrupt handler */
void DebugMon_Handler(void)
{
}

/* pendable request for system service interrupt handler */
void PendSV_Handler(void)
{
}

/* system tick interrupt handler */
void SysTick_Handler(void)
{

}
