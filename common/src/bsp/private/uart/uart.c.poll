#include "bsp/private/uart/uart.h"

#include "stm32f1xx.h"
#include "types.h"


/* Baud rate configuration. See section 27.3.4 of the reference manual for 
   equations and rationale. */
#define BAUD        (115200u)
#define DIV         (F_CPU_HZ/(16.0f * BAUD))
#define DIV_WHOLE   ((u32_t) DIV)
#define DIV_FRAC    ((u32_t)((DIV - DIV_WHOLE) * 16))

/* Polling operation attempts */
#define MAX_POLL_ATTEMPTS   (1000)

static void usart_clock_enable(USART_TypeDef* p_uart);

/**
 * @brief Initialize the UART hardware driver.
 */
void uart_init(USART_TypeDef* p_uart)
{
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
    p_uart->CR1 |= (USART_CR1_TE | USART_CR1_RE);
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
    u32_t  dummy;
    size_t attempts_left;
    u32_t  status_reg;

    /* Assume data is not available by default. */
    available = E_FALSE;

    /* Inspect the status register for USART state */
    status_reg = p_uart->SR;

    if (0 != (status_reg & USART_SR_ORE)) {
        /* When a receiver overrun is detected, empty the data register. The
           extra read is to just be sure the overrun has been cleared. */
        dummy = p_uart->DR;
        dummy = p_uart->DR;
        (void)dummy; /* appease the compiler's set but unused warning */
    } else {
        /* Poll the status register until the receive complete bit is set or the
           timeout occurs. */
        attempts_left = MAX_POLL_ATTEMPTS;

        do {
            if (0 != (status_reg & USART_SR_RXNE)) {
                available = E_TRUE;
                break;
            }

            status_reg    = p_uart->SR;
            attempts_left -= 1;
        } while (0 != attempts_left);
    }

    return available;
}

/**
 * @brief Read a byte from the driver's buffer
 *
 * @return The contents of the UART's receiver buffer.
 */
u8_t uart_read(USART_TypeDef* p_uart)
{
    u32_t data;

    /* Do a blind read of the data register. It is the caller's responsiblity to
       ensure there is data in the RX buffer. */
    data = p_uart->DR;

    /* Only return the 8-bit byte. */
    return data & 0xFF;
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
    bool_t success;
    size_t attempts_left;

    /* Assume failure by default. */
    success = E_FALSE;

    /* Poll the transmit data register empty flag until it is set. */
    attempts_left = MAX_POLL_ATTEMPTS;

    do {
        if (0 != (p_uart->SR & USART_SR_TXE)) {
            p_uart->DR = byte;
            success = E_TRUE;
            break;
        }
    } while(0 != attempts_left);

    return success;
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