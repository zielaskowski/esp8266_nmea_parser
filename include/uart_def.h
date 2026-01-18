#include "esp8266/uart_register.h"
#include "freertos/FreeRTOS.h"
#include "uart.h"

/*
UART0 (connectet to USB)
both recive and transmit, will use only RX for GNSS data
TXDO will transmit boot message, but we not use
  - PIN1 TXDO
  - PIN3 RXDO
UART1
only transmit, for debuging purpose
redirect logs here
  - PIN2 TXD1 (D4 on board)
 */
#define GNSS_UART UART0
#define DEBUG_UART UART1

void uart_init(uint8_t uart_ch);
