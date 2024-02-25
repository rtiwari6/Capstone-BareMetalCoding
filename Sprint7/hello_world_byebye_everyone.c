/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Common example project which can run on all Mi-V soft processor variants.
 * Please refer README.TXT in the root folder of this project for more details.
 * This example project was created to drive an 8-digit 7-segment display
 * connected to the Raspberry Pi connector on the PolarFire SoC Icicle kit.
 * The message HELLO... will wrap around on the display.
 * Updated: May 11, 2023
 *
 */

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
uint8_t H = 0x37;
uint8_t E = 0x4F;
uint8_t L = 0xE;
uint8_t O = 0x7e;
uint8_t W = 0x6F;
uint8_t R = 0x5C;
uint8_t D = 0x7C;
uint8_t Y = 0x6C;
uint8_t N = 0x3C;
uint8_t V = 0x73;
uint8_t dp = 0x80;
uint8_t digit = 0x01;
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
uint8_t count_limit = 33;          // max count for low digit. 10 for 0 - 9; 16 for hex

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
            /* Write dp to digit 8; Blank digits 7, 6, 5, 4, 3, 2 and 1 */
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
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 1:
            /* Write dp to digits 8 and 7 */
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 2:
            /* Write dp to digits 8, 7 and 6 */
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 3:
            /* Write O to digit 8, write dp to digits 7, 6 and 5 */
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 4:
            /* Write L to digit 8, write write O dp to digit 7, write dp to digits 6, 5 and 4 */
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 5:
            /* Write L to digits 8 and 7, write write O to digit 6, write dp to digits 5, 4 and 3 */
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 6:
            /* Write E to digit 8, L to digits 7 and 6, write write O digit 5, write dp to digits 4, 3 and 2 */
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 7:
            /* Write H to digit 8, E to digit 7, write L to digits 6 and 5, write write O digit 4, write dp to digits 3, 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 8:
            /* Blank digit 8, write H to digit 7, write E to digit 6, write L to digits 5 and 4, write write O digit 3, write dp to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            //master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 9:
            /* Blank digits 8 and 7, write H to digit 6, write E to digit 5, write L to digits 4 and 3, write write O digit 2, write dp to digit 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 10:
            /* Blank digits 8, 7 and 6, write H to digit 5, write E to digit 4, write L to digits 3 and 2, write write O digit 1 */
            master_tx_frame = ((digit1 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 11:
            /* Blank digits 8, 7, 6 and 5, write H to digit 4, write E to digit 3, write L to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 12:
            /* Blank digits 8, 7, 6, 5 and 4, write H to digit 3, write E to digit 2, write L to digit 1 */
            master_tx_frame = ((digit1 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 13:
            /* Blank digits 8, 7, 6, 5, 4 and 3, write H to digit 2, write E to digit 1 */
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 14:
            /* Blank digits 8, 7, 6, 5, 4, 3 and 2, write H to digit 1 */
            master_tx_frame = ((digit1 << 8) + H);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 15:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 16:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 17:
            /* Write dp to digit 8; Blank digits 7, 6, 5, 4, 3, 2 and 1 */
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
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 18:
            /* Write dp to digits 8 and 7 */
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 19:
            /* Write dp to digits 8, 7 and 6 */
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 20:
            /* Write O to digit 8, write dp to digits 7, 6 and 5 */
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 21:
            /* Write L to digit 8, write write O dp to digit 7, write dp to digits 6, 5 and 4 */
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 22:
            /* Write L to digits 8 and 7, write write O to digit 6, write dp to digits 5, 4 and 3 */
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 23:
            /* Write E to digit 8, L to digits 7 and 6, write write O digit 5, write dp to digits 4, 3 and 2 */
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 24:
            /* Write H to digit 8, E to digit 7, write L to digits 6 and 5, write write O digit 4, write dp to digits 3, 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 25:
            /* Blank digit 8, write H to digit 7, write E to digit 6, write L to digits 5 and 4, write write O digit 3, write dp to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            //master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 26:
            /* Blank digits 8 and 7, write H to digit 6, write E to digit 5, write L to digits 4 and 3, write write O digit 2, write dp to digit 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 27:
            /* Blank digits 8, 7 and 6, write H to digit 5, write E to digit 4, write L to digits 3 and 2, write write O digit 1 */
            master_tx_frame = ((digit1 << 8) + D);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 28:
            /* Blank digits 8, 7, 6 and 5, write H to digit 4, write E to digit 3, write L to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + L);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 29:
            /* Blank digits 8, 7, 6, 5 and 4, write H to digit 3, write E to digit 2, write L to digit 1 */
            master_tx_frame = ((digit1 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 30:
            /* Blank digits 8, 7, 6, 5, 4 and 3, write H to digit 2, write E to digit 1 */
            master_tx_frame = ((digit1 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 31:
            /* Blank digits 8, 7, 6, 5, 4, 3 and 2, write H to digit 1 */
            master_tx_frame = ((digit1 << 8) + W);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 32:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 33:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 34:
            /* Write dp to digit 8; Blank digits 7, 6, 5, 4, 3, 2 and 1 */
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
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 35:
            /* Write dp to digits 8 and 7 */
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 36:
            /* Write dp to digits 8, 7 and 6 */
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 37:
            /* Write O to digit 8, write dp to digits 7, 6 and 5 */
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 38:
            /* Write L to digit 8, write write O dp to digit 7, write dp to digits 6, 5 and 4 */
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 39:
            /* Write L to digits 8 and 7, write write O to digit 6, write dp to digits 5, 4 and 3 */
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 40:
            /* Write E to digit 8, L to digits 7 and 6, write write O digit 5, write dp to digits 4, 3 and 2 */
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 41:
            /* Write H to digit 8, E to digit 7, write L to digits 6 and 5, write write O digit 4, write dp to digits 3, 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 42:
            /* Blank digit 8, write H to digit 7, write E to digit 6, write L to digits 5 and 4, write write O digit 3, write dp to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            //master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 43:
            /* Blank digits 8 and 7, write H to digit 6, write E to digit 5, write L to digits 4 and 3, write write O digit 2, write dp to digit 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 44:
            /* Blank digits 8, 7 and 6, write H to digit 5, write E to digit 4, write L to digits 3 and 2, write write O digit 1 */
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 45:
            /* Blank digits 8, 7, 6 and 5, write H to digit 4, write E to digit 3, write L to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 46:
            /* Blank digits 8, 7, 6, 5 and 4, write H to digit 3, write E to digit 2, write L to digit 1 */
            master_tx_frame = ((digit1 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 47:
            /* Blank digits 8, 7, 6, 5, 4 and 3, write H to digit 2, write E to digit 1 */
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 48:
            /* Blank digits 8, 7, 6, 5, 4, 3 and 2, write H to digit 1 */
            master_tx_frame = ((digit1 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + B);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 49:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 50:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 51:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 52:
            /* Write dp to digit 8; Blank digits 7, 6, 5, 4, 3, 2 and 1 */
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
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 53:
            /* Write dp to digits 8 and 7 */
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 54:
            /* Write dp to digits 8, 7 and 6 */
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 55:
            /* Write O to digit 8, write dp to digits 7, 6 and 5 */
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 56:
            /* Write L to digit 8, write write O dp to digit 7, write dp to digits 6, 5 and 4 */
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 57:
            /* Write L to digits 8 and 7, write write O to digit 6, write dp to digits 5, 4 and 3 */
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 58:
            /* Write E to digit 8, L to digits 7 and 6, write write O digit 5, write dp to digits 4, 3 and 2 */
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 59:
            /* Write H to digit 8, E to digit 7, write L to digits 6 and 5, write write O digit 4, write dp to digits 3, 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit8 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 60:
            /* Blank digit 8, write H to digit 7, write E to digit 6, write L to digits 5 and 4, write write O digit 3, write dp to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit7 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            //master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 61:
            /* Blank digits 8 and 7, write H to digit 6, write E to digit 5, write L to digits 4 and 3, write write O digit 2, write dp to digit 1 */
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit6 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 62:
            /* Blank digits 8, 7 and 6, write H to digit 5, write E to digit 4, write L to digits 3 and 2, write write O digit 1 */
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit5 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 63:
            /* Blank digits 8, 7, 6 and 5, write H to digit 4, write E to digit 3, write L to digits 2 and 1 */
            master_tx_frame = ((digit1 << 8) + N);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit4 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 64:
            /* Blank digits 8, 7, 6, 5 and 4, write H to digit 3, write E to digit 2, write L to digit 1 */
            master_tx_frame = ((digit1 << 8) + O);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit3 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 65:
            /* Blank digits 8, 7, 6, 5, 4 and 3, write H to digit 2, write E to digit 1 */
            master_tx_frame = ((digit1 << 8) + Y);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            master_tx_frame = ((digit2 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 66:
            /* Blank digits 8, 7, 6, 5, 4, 3 and 2, write H to digit 1 */
            master_tx_frame = ((digit1 << 8) + R);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 67:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 68:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + V);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 69:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + E);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 69:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;

        case 69:
            /* Blank digits 8, 7, 6, 5, 4, 3, 2 and 1 */
//            master_tx_frame = ((digit1 << 8) + NOOP);
            master_tx_frame = ((digit1 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit2 << 8) + NOOP);
            master_tx_frame = ((digit2 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit3 << 8) + NOOP);
            master_tx_frame = ((digit3 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit4 << 8) + NOOP);
            master_tx_frame = ((digit4 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit5 << 8) + NOOP);
            master_tx_frame = ((digit5 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit6 << 8) + NOOP);
            master_tx_frame = ((digit6 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit7 << 8) + NOOP);
            master_tx_frame = ((digit7 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
//            master_tx_frame = ((digit8 << 8) + NOOP);
            master_tx_frame = ((digit8 << 8) + dp);
            SPI_transfer_frame(&g_spi0, master_tx_frame);
            break;
        }
        }


    count++;           // increment low digit counter
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
