#include "miv_rv32_hal.h"
#include "hal.h"
#include "hw_platform.h"
#include "core_gpio.h"
#include "core_uart_apb.h"
#include "core_spi.h"
#include "string.h"
#include "stdio.h"

const char * g_hello_msg =
        "\r\n***********************************************************************************\r\n\n\
********************    Mi-V PolarFire SoC Discovery Kit Demo    ***********************\r\n\n\
****************************************************************************************\r\n\r\n\
Drives 8-digit 7-segment display and the LEDs on the PolarFire SoC Discovery kit. \
The LED pattern changes when a system timer interrupt occurs. The 7-segment display will scroll \
HELLO...\r\n";


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

/*-----------------------------------------------------------------------------
 * CoreSPI instance data
 */
spi_instance_t g_spi0;
uint16_t master_tx_frame;
uint16_t master_tx_frame_temp;
uint8_t master_tx_low_frame;
uint8_t master_tx_high_frame;

/*-----------------------------------------------------------------------------
 * 7 segment display instance data
 */
uint16_t display_init = 0x0C01;
uint8_t NOOP = 0x00;
uint8_t B = 0x1F;
uint8_t Y = 0x3B;
uint8_t E = 0x4F;
uint8_t dp = 0x80;
uint8_t nine = 0x7B;
uint8_t eight = 0x7F;
uint8_t seven = 0x70;
uint8_t six = 0x5F;
uint8_t five = 0x5B;
uint8_t four = 0x33;
uint8_t three = 0x79;
uint8_t two = 0x6D;
uint8_t one = 0x30;
uint8_t digit0 = 0x01;
uint8_t digit1 = 0x01;
uint8_t digit2 = 0x02;
uint8_t digit3 = 0x03;
uint8_t digit4 = 0x04;
uint8_t digit5 = 0x05;
uint8_t digit6 = 0x06;
uint8_t digit7 = 0x07;
uint8_t digit8 = 0x08;
uint8_t decode_mode = 0x09;
uint8_t intensity = 0x0A;
uint8_t scan_limit = 0x0B;
uint8_t shutdown = 0x0C;
uint8_t disp_test = 0x0F;

/*-----------------------------------------------------------------------------
 * Digit counters
 */
uint8_t count = 0;                 // track value to write to the displays
uint8_t count_limit = 2;          // max count for low digit. 10 for 0 - 9; 16 for hex

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
    static volatile uint32_t val = 9u;
    val ^= 0xFu;
    GPIO_set_outputs(&g_gpio_out, val);
    UART_polled_tx_string(&g_uart,
            (const uint8_t *)"\r\nInternal System Timer Interrupt");

    /* Drive 7-segment display
     * count0 drives the display value
     * digit selects the display    */
    if (count < count_limit)
    {
        switch (count)
        {

        case 0:

            master_tx_frame = ((digit8 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        }
        count++;
    }
    else
    {
        count=0;

    }
}

/*-------------------------------------------------------------------------//**
  main() function.
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

    /* The CoreGPIO IP instantiated in the reference designs provided on github
     * have is configured to have GPIO_0 to GPIO_3 ports as outputs.
     * This configuration can not be changed by the firmware since it is fixed in
     * the CoreGPIO instance. In your Libero design if you do not make
     * the GPIO configurations 'fixed', then you will need to configure them
     * using GPIO_config() function*/

    /* set the output value */
    GPIO_set_outputs(&g_gpio_out, 0x0u);

    /* Enable the global external interrupt bit.
       This must be done for all Mi-V cores to enable interrupts globally. */

    /* Initialize CoreSPI   */
    SPI_init( &g_spi0, CORESPI_BASE_ADDR, 1 );

    /* Configure CoreSPI for Master mode  */
    SPI_configure_master_mode (&g_spi0);

    /* Set SPI slave select  */
    SPI_set_slave_select(&g_spi0, SPI_SLAVE_0);

    /* Initialize the display by writing to the shutdown register */
    SPI_transfer_frame(&g_spi0, display_init);

    /* Write decode mode register - set code B for digits 0 - 7 */
    master_tx_frame = 0x09FF;
    SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Blank display */
    master_tx_frame = ((digit1 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit2 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit3 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit4 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit5 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit6 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit7 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);
    master_tx_frame = ((digit8 << 8) + NOOP);
    SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Write decode mode register - set no decode for digits 0 - 7 */
    master_tx_frame = 0x0900;
    SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Write scan limit register - display all digits */
    master_tx_frame = 0x0B07;
    SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Write Intensity register - select 25/32 intensity - no displays illuminate */
    //     master_tx_frame = 0x0C0A;
    //     SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Write Intensity register - select max intensity) */
    master_tx_frame = 0x0F0A;
    SPI_transfer_frame(&g_spi0, master_tx_frame);

    /* Enter normal mode */
    master_tx_frame = 0x0F00;
    SPI_transfer_frame(&g_spi0, master_tx_frame);

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

    // specify systick timer interrupt interval. Determines speed of LEDs and 7-seg display changing
    //    MRV_systick_config(SYS_CLK_FREQ/4);
    MRV_systick_config(SYS_CLK_FREQ/2);

    /**************************************************************************
     * Loop
     *************************************************************************/
    do
    {

    } while (1);

    return 0u;
}
