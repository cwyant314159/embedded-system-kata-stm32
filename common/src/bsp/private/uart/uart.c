#include "bsp/private/uart/uart.h"

#include <string.h>

#include "bsp/bsp.h"
#include "bsp/private/startup/vectors.h"
#include "stm32f1xx.h"
#include "types.h"

/* Baud rate configuration. See section 27.3.4 of the reference manual for 
   equations and rationale. */
#define BAUD        (115200u)
#define DIV         (F_CPU_HZ/(16.0f * BAUD))
#define DIV_WHOLE   ((u32_t) DIV)
#define DIV_FRAC    ((u32_t)((DIV - DIV_WHOLE) * 16))

/* Ring buffer infrastructure */
#define BYTE_RING_MAX_SIZE  (256u)
#define PRIVATE_RING_SIZE   BYTE_RING_MAX_SIZE  /* set ring size */
#define PRIVATE_RING_VOLATILE_DECL              /* byte rings in this module are volatile */
#include "utils/private_ring.h"

PRIVATE_RING_DECLARATIONS(ByteRing, u8_t)                /* create ring type of bytes */
PRIVATE_RING_DECLARE(static volatile ByteRing, rx_ring); /* bytes to read             */
PRIVATE_RING_DECLARE(static volatile ByteRing, tx_ring); /* bytes to write            */

/* Readability macros for private ring functions */
#define BYTE_RING_INIT(var_name)       PRIVATE_RING_INIT(ByteRing, var_name)
#define BYTE_RING_IS_EMPTY(var_name)   PRIVATE_RING_IS_EMPTY(ByteRing, var_name)
#define BYTE_RING_IS_FULL(var_name)    PRIVATE_RING_IS_FULL(ByteRing, var_name)
#define BYTE_RING_PUSH(var_name, data) PRIVATE_RING_PUSH(ByteRing, var_name, data)
#define BYTE_RING_POP(var_name)        PRIVATE_RING_POP(ByteRing, var_name)
#define BYTE_RING_PEEK(var_name)       PRIVATE_RING_PEEK(ByteRing, var_name)

static void usart_clock_enable(USART_TypeDef* p_uart);

/**
 * @brief Initialize the UART hardware driver.
 */
void uart_init(USART_TypeDef* p_uart)
{
    /* Since this module relies on global data, we are going to limit the usage
       of this driver to USART1 only. */
    if (USART1 != p_uart) {
        bsp_error_trap();
    }

    /* Initialize byte rings */
    BYTE_RING_INIT(rx_ring);
    BYTE_RING_INIT(tx_ring);

    /* Configure the USART interrupt for about middle of the range of available
       interrupt priorities.
     */
    const u32_t usart_irq_prio = (1UL << __NVIC_PRIO_BITS) / 2;
    NVIC_SetPriority(USART1_IRQn, usart_irq_prio);

    /* Enable the USART interrupt in the NVIC */
    NVIC_EnableIRQ(USART1_IRQn);

    /* Enable the USART's clock in the RCC */
    usart_clock_enable(p_uart);

    /* Reset the USART control registers */
    p_uart-> CR1 = 0;
    p_uart-> CR2 = 0;
    p_uart-> CR3 = 0;

    /* Enable the USART for configuration */
    p_uart->CR1 |= (USART_CR1_UE);

    /* Set the baud rate as calculated from the #defines */
    const u32_t div_mantissa = DIV_WHOLE << USART_BRR_DIV_Mantissa_Pos;
    const u32_t div_frac     = DIV_FRAC  << USART_BRR_DIV_Fraction_Pos;

    p_uart->BRR = div_mantissa | div_frac;  

    /* 8 data bits, 1 stop bit, and no parity are selected by default after
       zero'ing the control registers. */
 
    /* Enable USART transmitter and receiver. */
    p_uart->CR1 |= (USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE);    
}

/**
 * @brief Check if the driver's receiver has data bytes.
 *
 * @retval E_TRUE  - the driver's buffer has data bytes to read
 * @retval E_FALSE - the driver has no data to read
 */
bool_t uart_data_available(USART_TypeDef* p_uart)
{
    bool_t available;

    if (E_TRUE == BYTE_RING_IS_EMPTY(rx_ring)) {
        available = E_FALSE;
    } else {
        available = E_TRUE;
    }

    return available;
}

/**
 * @brief Read a byte from the driver's buffer
 *
 * @return A byte received over the UART or NULL ('\0') if there are no bytes
 * left.
 */
u8_t uart_read(USART_TypeDef* p_uart)
{
    u8_t byte;

    if (E_TRUE == uart_data_available(p_uart)) {
        byte = BYTE_RING_POP(rx_ring);
    } else {
        byte = '\0';
    }

    return byte;
}

/**
 * @brief Write a byte to the driver's buffer
 *
 * @param[in] byte the data byte to transmit over the UART
 *
 * @retval E_TRUE  - byte successfully handled by driver
 * @retval E_FALSE - an error occurred during transmission
 */
bool_t uart_write(USART_TypeDef* p_uart, u8_t byte)
{
    bool_t result;

    /* If the ring is not full, add the byte to the buffer. */
    if (E_FALSE == BYTE_RING_IS_FULL(tx_ring)) {
        BYTE_RING_PUSH(tx_ring, byte);
        result = E_TRUE;
    } else {
        result = E_FALSE;
    }

    /* Regardless of the buffer state, we need to enable the transmitter to
       empty the byte we just added (or the back up of bytes preventing the
       ring push) */
    p_uart->CR1 |= USART_CR1_TXEIE;

    return result;
}

void USART1_IRQHandler(void)
{
    u32_t status_reg;
    u32_t data;

    /* Inspect the status register for USART state */
    status_reg = USART1->SR;

    if (0 != (status_reg & USART_SR_TXE)) {
        /* Transmitter empty interrupt */
        if (E_TRUE == BYTE_RING_IS_EMPTY(tx_ring)) {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        } else {
            data = BYTE_RING_POP(tx_ring);
            USART1->DR = data;
        }
    } 
    
    if (0 != (status_reg & USART_SR_RXNE)) {
        data = USART1->DR;

        if (E_FALSE == BYTE_RING_IS_FULL(rx_ring)) {
            data &= 0xFF;
            BYTE_RING_PUSH(rx_ring, data);
        }
    }
}

static void usart_clock_enable(USART_TypeDef* p_uart)
{
    if (USART1 == p_uart) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    }
    else {
        const u32_t en = (USART2 == p_uart) ? RCC_APB1ENR_USART2EN :
                         (USART3 == p_uart) ? RCC_APB1ENR_USART3EN :
                                              0                    ;

        RCC->APB1ENR |= en;
    }
}