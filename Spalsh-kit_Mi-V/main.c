#include "miv_rv32_hal.h"
#include "hal.h"
#include "hw_platform.h"
#include "core_gpio.h"
#include "core_uart_apb.h"
#include "core_timer.h"

const char * g_hello_msg =

/*-----------------------------------------------------------------------------
 * UART instance data.
 */
UART_instance_t g_uart;
#define RX_BUFF_SIZE                64u
uint8_t g_rx_buff[RX_BUFF_SIZE] =   {0u};
volatile uint8_t g_rx_size      =   0u;

/*-----------------------------------------------------------------------------
 * GPIO instance data.
 */
gpio_instance_t g_gpio_out;
timer_instance_t g_timer0;

/*-----------------------------------------------------------------------------
 * Interrupt handlers
 */
uint8_t External_30_IRQHandler()
{
    return(EXT_IRQ_KEEP_ENABLED);
}

uint8_t External_31_IRQHandler()
{
    return(EXT_IRQ_KEEP_ENABLED);
}

void Software_IRQHandler()
{
    MRV_clear_soft_irq();
}

void External_IRQHandler()
{
	UART_polled_tx_string(&g_uart,								
			(const uint8_t *)"\r\nExternal Timer Interrupt");  
	TMR_clear_int(&g_timer0);									
}

void MGEUI_IRQHandler(void)
{
}

void MGECI_IRQHandler(void)
{
}

void MSYS_EI0_IRQHandler(void)
{
}

void MSYS_EI1_IRQHandler(void)
{
}

void MSYS_EI2_IRQHandler(void)
{
}

void MSYS_EI3_IRQHandler(void)
{
}

void MSYS_EI4_IRQHandler(void)
{
}

void MSYS_EI5_IRQHandler(void)
{
}

void OPSRV_IRQHandler(void)
{
}

void SysTick_Handler(void)
{
    static volatile uint32_t val = 3u;		//ml was 0u
    val ^= 0xFu;
    GPIO_set_outputs(&g_gpio_out, val);
    UART_polled_tx_string(&g_uart,
                        (const uint8_t *)"\r\nInternal System Timer Interrupt");
}

/*-------------------------------------------------------------------------//**
  given main() function.
*/
int main(void)
{
    uint8_t rx_char;
    uint8_t rx_count;
    uint32_t switches;

    UART_init(&g_uart,
              COREUARTAPB0_BASE_ADDR,
              BAUD_VALUE_115200,
              (DATA_8_BITS | NO_PARITY));

    UART_polled_tx_string(&g_uart, (const uint8_t *)g_hello_msg);

    /* Initializing GPIOs */
    GPIO_init(&g_gpio_out, COREGPIO_OUT_BASE_ADDR, GPIO_APB_32_BITS_BUS);

    GPIO_set_outputs(&g_gpio_out, 0x0u);

    //ml
    TMR_init(&g_timer0,
    		CORETIMER0_BASE_ADDR,
			TMR_CONTINUOUS_MODE,
			PRESCALER_DIV_1024,
			488280);				// 488280 is ~10sec
    TMR_enable_int(&g_timer0);
    TMR_start(&g_timer0);
    //ml end
    
    HAL_enable_interrupts();

#ifndef MIV_LEGACY_RV32

    MRV_enable_local_irq(MRV32_EXT_IRQn |
                         MRV32_MSYS_EIE0_IRQn |
                         MRV32_MSYS_EIE1_IRQn |
                         MRV32_MSYS_EIE2_IRQn |
                         MRV32_MSYS_EIE3_IRQn |
                         MRV32_MSYS_EIE4_IRQn |
                         MRV32_MSYS_EIE5_IRQn);

#endif

    MRV_systick_config(SYS_CLK_FREQ);

    /**************************************************************************
    * Loop
    *************************************************************************/
    do
    {
        g_rx_size = UART_get_rx(&g_uart, g_rx_buff, sizeof(g_rx_buff));
        if (g_rx_size > 0u)
        {           
            UART_polled_tx_string(&g_uart, (const uint8_t *)g_rx_buff);
            g_rx_size = 0u;
        }
    } while (1);

    return 0u;
}