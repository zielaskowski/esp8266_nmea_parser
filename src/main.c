#include "board_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_def.h"
#include "web_server.h"

#ifdef DEBUG
#define LWIP_DEBUG 1
#define SOCKETS_DEBUG LWIP_DBG_ON

#define DEB(...) os_printf(__VA_ARGS__)
#else
#define DEB(...)
#endif

static void uart_gnss_task(void *arg) {
  uint8_t ch;
  char line[NMEA_MAX_LEN];
  int idx = 0;

  while (1) {
    if (uart_rx_one_char(&ch) == OK) {
      if (ch == '\n') {
        // end line found
        line[idx] = '\0';
        // if string longer then 6chars and starts with $
        if (idx > 6 && line[0] == '$') {
          nmea_msg_t msg;
          strcpy(msg.line, line);
          xQueueSend(nmea_queue, &msg, 0);
          // DEB("recived NMEA line: %s\n", msg.line);
        }
        // else start from zero
        idx = 0;
        // add next char if less then max len
      } else if (ch != '\r') {
        if (idx < (NMEA_MAX_LEN - 1)) {
          line[idx++] = ch;
        } else {
          // else start from zero
          idx = 0;
        }
      }
    } else {
      // wait another for char after \r
      vTaskDelay(1); // 1ms
    }
  }
}

static void webserver_task(void *arg) {
  if (xSemaphoreTake(wifi_ready, portMAX_DELAY) == pdPASS) {
    // run only if wifi_ready released
    DEB("setting webserver..\n");
    int sock = lwip_socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = INADDR_ANY;

    int bind = lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    lwip_listen(sock, 2);

    DEB("waiting client..\n");
    while (1) {
      int client = lwip_accept(sock, NULL, NULL);
      DEB("Client: %d\n", client);
      if (client >= 0)
        handle_client(client);
    }
  }
}

void user_init(void) {

  // init uarts
  uart_init(GNSS_UART);
  uart_init(DEBUG_UART);
  UART_SetPrintPort(UART1); // send logs to UART1

  printf("UART GNSS demo\n");

  nmea_queue = xQueueCreate(NMEA_QUEUE_LEN, sizeof(nmea_msg_t));
  if (nmea_queue == 0) {
    DEB("Failed to create NMEA queue\n");
    system_restart(); // restart kernel
  }

  vSemaphoreCreateBinary(wifi_ready);
  if (wifi_ready == NULL) {
    DEB("Failed to create wifi_ready semaphore\n");
    system_restart();
  }

  // get GNSS string from incoming UART
  xTaskCreate(uart_gnss_task, (signed char *)"uart_gnss", 2048, NULL, 5, NULL);
  DEB("UART GNSS reading task registered\n");
  // web server
  wifi_init();
  xTaskCreate(webserver_task, (signed char *)"web_serv", 2048, NULL, 2, NULL);
  DEB("webserver task registered\n");
}
