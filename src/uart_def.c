#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "uart_def.h"

xQueueHandle nmea_queue;

void uart_init(uint8_t uart_ch) {
  UART_ConfigTypeDef cfg = {.baud_rate = BIT_RATE_115200,
                            .data_bits = UART_WordLength_8b,
                            .parity = USART_Parity_None,
                            .stop_bits = USART_StopBits_1,
                            .flow_ctrl = USART_HardwareFlowControl_None};
  UART_ParamConfig(uart_ch, &cfg);
  UART_ResetFifo(uart_ch);
}

static STATUS uart_rx_one_char(uint8_t *ch) {
  // ESP8266 Technical Reference, p.109
  // UART_STATUS reg [7:0] bits: number of data in uart rx fifo
  if (READ_PERI_REG(UART_STATUS(GNSS_UART)) & UART_RXFIFO_CNT) {
    // UART_FIFO reg [31:8]: UART FIFO, length 128
    // [7:0]: rxfifo_rd_byte; R/W share the same address
    *ch = READ_PERI_REG(UART_FIFO(GNSS_UART)) & 0xFF;
    return OK;
  } else {
    return FAIL;
  }
}
