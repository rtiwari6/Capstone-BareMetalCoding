/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file u54_1.c
 *
 * @author Microchip FPGA Embedded Systems Solutions
 *
 * @brief Application code running on U54_1.
 * PolarFire SoC MSS RTC Time example project
 *
 */

#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "drivers/mss/mss_rtc/mss_rtc.h"
#include "inc/uart_mapping.h"
extern struct mss_uart_instance* p_uartmap_u54_1;

/* Constant used for setting RTC control register. */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

uint8_t display_buffer[100];

/* Function prototypes */
void handle_user_input(void);
void set_time(void);
void set_date(void);
uint8_t get_user_input(void);

/******************************************************************************
 *  Greeting messages displayed over the UART.
 */
const uint8_t g_greeting_msg[] =
        "\r\n\r\n\t  ******* PolarFire SoC RTC Time Example *******\n\n\n\r\
The example project demonstrates the RTC time mode. \r\n\n\n\
";

/* Main function for the hart1(U54_1 processor).
 * Application code running on hart1 is placed here.
 */
void u54_1(void)
{
    mss_rtc_calender_t calendar_count;

    /* Clear pending software interrupt in case there was any.
       Enable only the software interrupt so that the E51 core can bring this
       core out of WFI by raising a software interrupt. */
    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

#if (IMAGE_LOADED_BY_BOOTLOADER == 0)
    /*Put this hart into WFI.*/
    do
    {
        __asm("wfi");
    } while (0 == (read_csr(mip) & MIP_MSIP));

    /* The hart is out of WFI, clear the SW interrupt. Here onwards Application
     * can enable and use any interrupts as required */
    clear_soft_interrupt();
#endif

    PLIC_init();
    __enable_irq();

    PLIC_SetPriority(RTC_WAKEUP_PLIC, 2);

    (void)mss_config_clk_rst(MSS_PERIPH_MMUART_U54_1, (uint8_t)MPFS_HAL_LAST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_RTC, (uint8_t)MPFS_HAL_LAST_HART, PERIPHERAL_ON);

    MSS_UART_init(p_uartmap_u54_1,
                  MSS_UART_115200_BAUD,
                  MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
    );

    MSS_UART_polled_tx_string(p_uartmap_u54_1, g_greeting_msg);

    SYSREG->RTC_CLOCK_CR &= ~BIT_SET;
    SYSREG->RTC_CLOCK_CR = 50000000 / 1000000;
    SYSREG->RTC_CLOCK_CR |= BIT_SET;

    /* Initialize RTC. */
    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_CALENDAR_MODE, RTC_PERIPH_PRESCALER);


    /* Enable RTC to start incrementing. */
    MSS_RTC_start();

    for (;;)
    {
        volatile uint32_t rtc_count_updated;

        /* Update displayed time if value read from RTC changed since last read. */
        rtc_count_updated = MSS_RTC_get_update_flag();
        if (rtc_count_updated)
        {
            MSS_RTC_get_calendar_count(&calendar_count);
            snprintf((char *)display_buffer, sizeof(display_buffer),
                     "Seconds: %02d",(int)(calendar_count.second));

            MSS_UART_polled_tx_string(p_uartmap_u54_1, display_buffer);
            MSS_UART_polled_tx_string(p_uartmap_u54_1, "\r\n");
            MSS_RTC_clear_update_flag();
        }

        /* Check for user input to set time or date. */
        handle_user_input();
    }
    /* never return*/
}

/* Handle user input for setting time or date. */
/* Handle user input for setting time or date. */
void handle_user_input(void)
{
    uint8_t rx_buff[1];
    size_t rx_size;

    rx_size = MSS_UART_get_rx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff));
    if (rx_size > 0)
    {
        MSS_UART_polled_tx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff));  // Echo back received character
        switch (rx_buff[0])
        {
            case 't':
                MSS_RTC_stop();
                set_time();
                MSS_RTC_start(); // Restart RTC after setting time
                break;
            case 'd':
                MSS_RTC_stop();
                set_date();
                MSS_RTC_start(); // Restart RTC after setting date
                break;
            default:
                break;
        }
    }
}

/* Set the time based on user input. */
void set_time(void)
{
    mss_rtc_calender_t new_calendar_time;
    uint8_t user_input;

    /* Get hours from user. */
    MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter hours (0-23): ");
    user_input = get_user_input();
    if (user_input <= 23)
    {
        new_calendar_time.hour = user_input;

        /* Get minutes from user. */
        MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter minutes (0-59): ");
        user_input = get_user_input();
        if (user_input <= 59)
        {
            new_calendar_time.minute = user_input;

            /* Get seconds from user. */
            MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter seconds (0-59): ");
            user_input = get_user_input();
            if (user_input <= 59)
            {
                new_calendar_time.second = user_input;

                /* Set the new time. */
                MSS_RTC_set_calendar_count(&new_calendar_time);
                MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rTime set successfully.\n\r");
            }
            else
            {
                MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid seconds input.\n\r");
            }
        }
        else
        {
            MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid minutes input.\n\r");
        }
    }
    else
    {
        MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid hours input.\n\r");
    }
}

/* Set the date based on user input. */
void set_date(void)
{
    mss_rtc_calender_t new_calendar_time;
    uint8_t user_input;

    /* Get day from user. */
    MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter day (1-31): ");
    user_input = get_user_input();
    if (user_input >= 1 && user_input <= 31)
    {
        new_calendar_time.day = user_input;

        /* Get month from user. */
        MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter month (1-12): ");
        user_input = get_user_input();
        if (user_input >= 1 && user_input <= 12)
        {
            new_calendar_time.month = user_input;

            /* Get year from user. */
            MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rEnter year (0-255): ");
            user_input = get_user_input();
            if (user_input >= 0 && user_input <= 255)
            {
                new_calendar_time.year = user_input;

                /* Set the new date. */
                MSS_RTC_set_calendar_count(&new_calendar_time);
                MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rDate set successfully.\n\r");
            }
            else
            {
                MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid year input.\n\r");
            }
        }
        else
        {
            MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid month input.\n\r");
        }
    }
    else
    {
        MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\rInvalid day input.\n\r");
    }
}

uint8_t get_user_input(void)
{
    uint8_t rx_buff[2];  // Increase buffer size to handle two characters: one for input and one for '\0'
    size_t rx_size = 0;

    // Wait until user input is available and '\r' is not received
    while (rx_size == 0 || rx_buff[0] != '\r')
    {
        rx_size = MSS_UART_get_rx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff) - 1);  // Read into buffer, leave space for '\0'
    }

    // Null-terminate the string
    rx_buff[rx_size] = '\0';

    // Convert ASCII to number
    uint8_t user_input = 0;
    sscanf((char *)rx_buff, "%hhu", &user_input);

    return user_input;
}
