#include <stdio.h>
#include <string.h>
#include "inc/common.h"
#include "testing_common.h"



const uint8_t g_message1[] =
        "\r\n\r\n\r\n **** PolarFire SoC MSS MMUART example ****\r\n\r\n\r\n";

const uint8_t g_message2[] =
        "This program is run from u54_1\r\n\
        \r\n\
Type 0  Show hart 1 debug message\r\n\
Type 1  Show this menu\r\n\
Type 2  Send message using polled method\r\n\
Type 3  send message using interrupt method\r\n\
";

const uint8_t polled_message1[] =
        "This message has been transmitted using polled method. \r\n";

const uint8_t intr_message1[] =
        " This message has been transmitted using local interrupt method. \r\n";


#define RX_BUFF_SIZE    16U
uint8_t g_rx_buff1[RX_BUFF_SIZE] = { 0 };
volatile uint32_t count_sw_ints_h1 = 0U;
volatile uint8_t g_rx_size1 = 0U;
static volatile uint32_t irq_cnt = 0;
uint8_t info_string1[100];

uint8_t tx_buffer[516];
uint16_t transfer_size;
uint16_t idx = 0;

/* Main function for the hart1(U54 processor).
 * Application code running on hart1 is placed here.
 * MMUART1 local interrupt is enabled on hart1.
 * In the respective U54 harts, local interrupts of the corresponding MMUART
 * are enabled. e.g. in U54_1.c local interrupt of MMUART1 is enabled. */

void u54_1(void) 
{
    uint64_t mcycle_start = 0U;
    uint64_t mcycle_end = 0U;
    uint64_t delta_mcycle = 0U;
    uint64_t hartid = read_csr(mhartid);

    init_system();

    /* All clocks ON */

//    test_uart();
   test_pwm();
//    test_gpios();
   test_7_seg();
//    test_leds();
    test_click_7_seg();
//
//    test_sd();

    MSS_UART_polled_tx_string(MAIN_UART, "\tBoard testing complete!\r\n");
    MSS_UART_polled_tx_string(MAIN_UART, "\tHaving fun ;)\r\n");
    having_fun = 1;

    // Loop blinking LEDs and printing messages :)

    mcycle_start = readmcycle();


    while (1u) 
    {
        g_rx_size1 = MSS_UART_get_rx(&g_mss_uart0_lo, g_rx_buff1, sizeof(g_rx_buff1));
        if (g_rx_size1 > 0u) 
        {
            switch (g_rx_buff1[0u]) 
            {
            case '0':

                mcycle_end = readmcycle();
                delta_mcycle = mcycle_end - mcycle_start;
                sprintf(info_string1, "hart %ld, %ld delta_mcycle \r\n", hartid,
                        delta_mcycle);
                MSS_UART_polled_tx(&g_mss_uart0_lo, info_string1,
                        strlen(info_string1));
                break;
            case '1':
                /* show menu */
                MSS_UART_polled_tx(&g_mss_uart0_lo, g_message2,
                        sizeof(g_message2));
                break;
            case '2':

                /* polled method of transmission */
                MSS_UART_polled_tx(&g_mss_uart0_lo, polled_message1,
                        sizeof(polled_message1));

                break;
            case '3':

                /* interrupt method of transmission */
                MSS_UART_polled_tx(&g_mss_uart0_lo, intr_message1,
                        sizeof(intr_message1));
                break;

            default:
                MSS_UART_polled_tx(&g_mss_uart0_lo, g_rx_buff1,
                        g_rx_size1);
                break;
            }

            g_rx_size1 = 0u;
        }


    }
}

/* hart1 Software interrupt handler */

uint8_t GPIO_STATES[8] = {0};

uint32_t looper = 0;
uint16_t dual_7_seg_counter = 0;

void U54_1_sysTick_IRQHandler(void)
{
    looper += 1;
    if (looper % (systick_loop_divider * 1) == 0){
        if (GPIO_STATES[0] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_17, 1u);
            GPIO_STATES[0] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_17, 0u);
            GPIO_STATES[0] = 0;
        }
        if (having_fun == 1){
            pwm_fun();
        }
    }

    if (looper % (systick_loop_divider * 2) == 0){
        if (GPIO_STATES[1] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_18, 1u);
            GPIO_STATES[1] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_18, 0u);
            GPIO_STATES[1] = 0;
        }
    }

    if (looper % (systick_loop_divider * 4) == 0){
        if (GPIO_STATES[2] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_19, 1u);
            GPIO_STATES[2] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_19, 0u);
            GPIO_STATES[2] = 0;
        }
        if (having_fun == 1){
            dual_7_seg_counter += 1;
            shift_into_click_7_seg(dual_7_seg_counter);
        }
    }

    if (looper % (systick_loop_divider * 8) == 0){
        if (GPIO_STATES[3] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_20, 1u);
            GPIO_STATES[3] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_20, 0u);
            GPIO_STATES[3] = 0;
        }
        print_value(looper);
    }

    if (looper % (systick_loop_divider * 16) == 0){
        if (GPIO_STATES[4] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_21, 1u);
            GPIO_STATES[4] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_21, 0u);
            GPIO_STATES[4] = 0;
        }
    }

    if (looper % (systick_loop_divider * 32) == 0){
        if (GPIO_STATES[5] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_22, 1u);
            GPIO_STATES[5] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_22, 0u);
            GPIO_STATES[5] = 0;
        }
    }

    if (looper % (systick_loop_divider * 64) == 0){
        if (GPIO_STATES[6] == 0){
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_23, 1u);
            GPIO_STATES[6] = 1;
        } else {
            MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_23, 0u);
            GPIO_STATES[6] = 0;
        }
    }

    if (looper % (systick_loop_divider * 128) == 0){
        if (GPIO_STATES[7] == 0){
            MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 1u);
            GPIO_STATES[7] = 1;
        } else {
            MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 0u);
            GPIO_STATES[7] = 0;
        }
    }
}
