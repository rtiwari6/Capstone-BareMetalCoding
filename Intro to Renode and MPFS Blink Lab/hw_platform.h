#ifndef HW_PLATFORM_H
#define HW_PLATFORM_H
#ifndef SYS_CLK_FREQ
#define SYS_CLK_FREQ                    80000000UL
#endif

/***************************************************************************//**
 * Non-memory Peripheral base addresses
 * Format of define is:
 * <corename>_<instance>_BASE_ADDR
 * The <instance> field is optional if there is only one instance of the core
 * in the design
 */
#define COREUARTAPB0_BASE_ADDR          0x70000000UL
#define COREGPIO_IN_BASE_ADDR           0x70001000UL
#define CORETIMER0_BASE_ADDR            0x70002000UL
#define CORETIMER1_BASE_ADDR            0x70004000UL
#define COREGPIO_OUT_BASE_ADDR          0x70001000UL
#define FLASH_CORE_SPI_BASE             0x70006000UL
#define CORE16550_BASE_ADDR             0x70007000UL
#define BAUD_VALUE_115200               ((SYS_CLK_FREQ / (16 * 115200)) - 1)
#define BAUD_VALUE_57600                ((SYS_CLK_FREQ / (16 * 57600)) - 1)
#ifdef MSCC_STDIO_THRU_CORE_UART_APB

#define MSCC_STDIO_UART_BASE_ADDR COREUARTAPB0_BASE_ADDR

#ifndef MSCC_STDIO_UART_BASE_ADDR
#error MSCC_STDIO_UART_BASE_ADDR not defined- e.g. #define MSCC_STDIO_UART_BASE_ADDR COREUARTAPB1_BASE_ADDR
#endif

#ifndef MSCC_STDIO_BAUD_VALUE

#define MSCC_STDIO_BAUD_VALUE           115200
#endif  

#endif  /* end of MSCC_STDIO_THRU_CORE_UART_APB */
/*******************************************************************************
 * 
 */
#endif 


