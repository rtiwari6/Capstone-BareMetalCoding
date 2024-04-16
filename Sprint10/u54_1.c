#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "drivers/mss/mss_rtc/mss_rtc.h"
#include "inc/uart_mapping.h"
#include "drivers/fpga_ip/CoreSPI/core_spi.h"
extern struct mss_uart_instance* p_uartmap_u54_1;

/* Constant used for setting RTC control register. */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

/* Constant used for setting RTC control register. */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

/*------------------------------------------------------------------------------
  command line interface defines.
 */
#define INVALID_USER_INPUT  -1
#define ENTER               0x0D

/*------------------------------------------------------------------------------
  Typedefs.
 */
typedef struct menu_item
{
    uint32_t min;
    uint32_t max;
    const char * message;
} menu_item_t;


/*------------------------------------------------------------------------------
  Local functions.
 */
static void display_greeting(void);
static void display_time(mss_rtc_calender_t *seconds_count);
static void get_time_from_user(void);
static int32_t get_number_from_user(void);

uint8_t display_buffer[100];


spi_instance_t g_7_seg_core_spi;
#define SPI_INSTANCE            &g_7_seg_core_spi
uint16_t master_tx_frame;
uint8_t digit1 = 0x01;
uint8_t digit2 = 0x02;
uint8_t digit3 = 0x03;
uint8_t digit4 = 0x04;
uint8_t digit5 = 0x05;
uint8_t digit6 = 0x06;
uint8_t digit7 = 0x07;
uint8_t digit8 = 0x08;
uint8_t zero = 0x7e;
uint8_t one = 0x30;
uint8_t two = 0x6D;
uint8_t three = 0x79;
uint8_t sec_units;
uint8_t sec_tens;
uint8_t min_units;
uint8_t min_tens;
uint8_t hour_units;
uint8_t hour_tens;
uint8_t sec_units_disp;
uint8_t sec_tens_disp;
uint8_t min_units_disp;
uint8_t min_tens_disp;
uint8_t hour_units_disp;
uint8_t hour_tens_disp;

void init_CoreSPI(void);
void init_7_seg(void);

#define RX_BUFF_SIZE    64U

uint8_t g_rx_buff[RX_BUFF_SIZE] = {0};
volatile uint8_t g_rx_size = 0U;

void u54_1(void)
{
   mss_rtc_calender_t calendar_count;
   size_t rx_size;
   uint8_t rx_buff[1];

    /* Clear pending software interrupt in case there was any.
     * Enable only the software interrupt so that the E51 core can bring this
     * core out of WFI by raising a software interrupt In case of external,
     * bootloader not present
     */

    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

#if (IMAGE_LOADED_BY_BOOTLOADER == 0)

    /*Put this hart into WFI.*/

    do
    {
        __asm("wfi");
    }while(0 == (read_csr(mip) & MIP_MSIP));

    /* The hart is out of WFI, clear the SW interrupt. Hear onwards Application
     * can enable and use any interrupts as required */
    clear_soft_interrupt();
#endif

    PLIC_init();
    __enable_irq();

    PLIC_SetPriority(RTC_WAKEUP_PLIC, 2);

    (void)mss_config_clk_rst(MSS_PERIPH_MMUART_U54_1, (uint8_t) MPFS_HAL_LAST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_RTC, (uint8_t) MPFS_HAL_LAST_HART, PERIPHERAL_ON);

  /* initialize CoreSPI - added TM    */
    init_CoreSPI();
    init_7_seg();

   /* mmuart1 initialization */

    MSS_UART_init( p_uartmap_u54_1,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    SYSREG->RTC_CLOCK_CR &= ~BIT_SET;
    SYSREG->RTC_CLOCK_CR = LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK / LIBERO_SETTING_MSS_RTC_TOGGLE_CLK;
    SYSREG->RTC_CLOCK_CR |= BIT_SET;

     /* Initialize RTC. */
     MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_CALENDAR_MODE, RTC_PERIPH_PRESCALER );

     /* Enable RTC to start incrementing. */
     MSS_RTC_start();

     /* Display greeting message. */
     display_greeting();

     /* Display time over UART. */

     for (;;)
     {
         volatile uint32_t rtc_count_updated;

         /* Update displayed time if value read from RTC changed since last read.*/
         rtc_count_updated = MSS_RTC_get_update_flag();
         if(rtc_count_updated)
         {
             MSS_RTC_get_calendar_count(&calendar_count);
             display_time(&calendar_count);
             MSS_RTC_clear_update_flag();
         }

         /* Start command line interface if any key is pressed. */
          rx_size = MSS_UART_get_rx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff));
          if(rx_size > 0)
          {
              if('t' == rx_buff[0])
              {
                  get_time_from_user();
              }
          }
     }
     /* never return*/
}


static void display_greeting(void)
{
    MSS_UART_polled_tx_string(p_uartmap_u54_1,(const uint8_t*)"\n\r\n\r**********************************************************************\n\r");
    MSS_UART_polled_tx_string(p_uartmap_u54_1,(const uint8_t*)"*************** PolarFire SoC Real Time Clock ***************\n\r");
    MSS_UART_polled_tx_string(p_uartmap_u54_1,(const uint8_t*)"**********************************************************************\n\r");
    MSS_UART_polled_tx_string(p_uartmap_u54_1,(const uint8_t*)"  - Set the RTC time by pressing \"t\" \n\r");
    MSS_UART_polled_tx_string(p_uartmap_u54_1,(const uint8_t*)"----------------------------------------------------------------------\n\r\n\r");
}

/*------------------------------------------------------------------------------
  Display time.
 */
static void display_time(mss_rtc_calender_t *calendar_count) {
    uint8_t display_buffer[128];
    snprintf((char *)display_buffer, sizeof(display_buffer), " Current Time: %02d:%02d:%02d\r\n",
             (int)calendar_count->hour,
             (int)calendar_count->minute,
             (int)calendar_count->second);
    MSS_UART_polled_tx_string(p_uartmap_u54_1, display_buffer);

    uint8_t digit_to_disp[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B};

    uint8_t sec_units_disp = digit_to_disp[calendar_count->second % 10];
    uint8_t sec_tens_disp = digit_to_disp[calendar_count->second / 10];
    uint8_t min_units_disp = digit_to_disp[calendar_count->minute % 10];
    uint8_t min_tens_disp = digit_to_disp[calendar_count->minute / 10];
    uint8_t hour_units_disp = digit_to_disp[calendar_count->hour % 10];
    uint8_t hour_tens_disp = digit_to_disp[calendar_count->hour / 10];

    uint16_t master_tx_frame;
    const uint8_t dp = 0x80;

    master_tx_frame = ((digit1 << 8) + sec_units_disp); // Write sec_units
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit2 << 8) + sec_tens_disp);   // Write sec_tens
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit3 << 8) + dp);  // Write min_units
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit5 << 8) + min_tens_disp);   // Write min_tens
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit4 << 8) + min_units_disp);  // Write min_units
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit6 << 8) + dp);  // Write min_units
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit8 << 8) + hour_tens_disp);  // Write hour_tens
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    master_tx_frame = ((digit7 << 8) + hour_units_disp); // Write hour_units
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
}



/*------------------------------------------------------------------------------
  Top level menu.
 */
static void get_time_from_user(void)
{
    int32_t user_input;
    mss_rtc_calender_t new_calendar_time;

    MSS_RTC_get_calendar_count(&new_calendar_time);

    MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\r\n\rChange time:\n\r Hours: ");

    user_input = get_number_from_user();
    if((INVALID_USER_INPUT != user_input) && (user_input < 24))
    {
        new_calendar_time.hour = (uint8_t)user_input;
        MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\r Minutes: ");
        user_input = get_number_from_user();
        if((INVALID_USER_INPUT != user_input) && (user_input  < 60))
        {
            new_calendar_time.minute = (uint8_t)user_input;
            MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\r Seconds: ");
            user_input = get_number_from_user();
            if((INVALID_USER_INPUT != user_input) && (user_input < 60))
            {
                new_calendar_time.second = (uint8_t)user_input;
                MSS_RTC_set_calendar_count(&new_calendar_time);
            }
        }
    }
    MSS_UART_polled_tx_string(p_uartmap_u54_1, (const uint8_t *)"\n\r\n\r");
}

/*------------------------------------------------------------------------------
  Retrieve a number typed by the user.
 */
static int32_t get_number_from_user(void)
{
    int32_t user_input = 0;
    uint8_t rx_buff[1];
    uint8_t complete = 0;
    size_t rx_size;

    while(!complete)
    {
        rx_size = MSS_UART_get_rx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
            MSS_UART_polled_tx(p_uartmap_u54_1, rx_buff, sizeof(rx_buff));
            if(ENTER == rx_buff[0])
            {
                complete = 1;
            }
            else if((rx_buff[0] >= '0') && (rx_buff[0] <= '9'))
            {
                user_input = (user_input * 10) + (rx_buff[0] - '0');
            }
            else
            {
                user_input = INVALID_USER_INPUT;
                complete = 1;
            }
        }
    }
    return user_input;
}

void init_CoreSPI(void){
    /* Initialize CoreSPI   */
    SPI_init( SPI_INSTANCE, 0x40000400, 1 );

    /* Configure CoreSPI for Master mode  */
    SPI_configure_master_mode (SPI_INSTANCE);

    /* Set SPI slave select  */
    SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE_0);
}

void init_7_seg(void){
    static uint16_t display_array[8] = {0};
    uint16_t display_init = 0x0C01;
    uint8_t NOOP = 0x00;
    uint8_t H = 0x37;
    uint8_t E = 0x4F;
    uint8_t L = 0xE;
    uint8_t O = 0x7e;
    uint8_t P = 0x67;
    uint8_t dp = 0x80;
    uint8_t digit;
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

    /* Initialize the display by writing to the shutdown register */
    SPI_transfer_frame(SPI_INSTANCE, display_init);

    /* Write decode mode register - set code B for digits 0 - 7 */
    master_tx_frame = 0x09FF;
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    /* Blank display */
    master_tx_frame = ((digit1 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit2 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit3 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit4 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit5 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit6 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit7 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
    master_tx_frame = ((digit8 << 8) + NOOP);
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    /* Write decode mode register - set no decode for digits 0 - 7 */
    master_tx_frame = 0x0900;
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    /* Write scan limit register - display all digits */
    master_tx_frame = 0x0B07;
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);

    /* Enter normal mode */
    master_tx_frame = 0x0F00;
    SPI_transfer_frame(SPI_INSTANCE, master_tx_frame);
}
