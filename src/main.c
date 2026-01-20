#include "board_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "nmea_parser.h"
#include "uart_def.h"
#include "web_server.h"

#ifdef DEBUG
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
          //  DEB("recived NMEA line: %s\n", msg.line);
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

/*static void parse_gnss_task(void *arg) {*/
/*  nmea_msg_t msg;*/
/**/
/*  while (1) {*/
/*    if (xQueueReceive(nmea_queue, &msg, portMAX_DELAY) == pdPASS) {*/
/*      // take NMEA msg from queue*/
/*      if (xSemaphoreTake(gnss_mutex, portMAX_DELAY) == pdPASS) {*/
/*        // block gnss state struct from others access*/
/**/
/*        // GNA message (Global Positioning Fix Data)*/
/*        if (!strncmp(&msg.line[3], "GGA", 3)) {*/
/*          parseGGA(&msg);*/
/*        }*/
/*        // GSV message (GPS Satelites in View)*/
/*        if (!strncmp(msg.line, "$GPGSV", 6)) { // GPS*/
/*          gnss.sats_view_GP = parseGSV(&msg);*/
/*        }*/
/*        if (!strncmp(msg.line, "$GLGSV", 6)) { // GLONASS*/
/*          gnss.sats_view_GL = parseGSV(&msg);*/
/*        }*/
/*        if (!strncmp(msg.line, "$GBGSV", 6)) { // BEIDOU*/
/*          gnss.sats_view_GB = parseGSV(&msg);*/
/*        }*/
/*        if (!strncmp(msg.line, "$BDGSV", 6)) { // BEODOU*/
/*          gnss.sats_view_BD = parseGSV(&msg);*/
/*        }*/
/*        if (!strncmp(msg.line, "$GNGSV", 6)) { // GNSS*/
/*          gnss.sats_view_GN = parseGSV(&msg);*/
/*        }*/
/*        if (!strncmp(msg.line, "$GAGSV", 6)) { // GALILEO*/
/*          gnss.sats_view_GA = parseGSV(&msg);*/
/*        }*/
/**/
/*        DEB("parsing: %s\n", msg.line);*/
/*        DEB("parsed: fix=%d sats=%d lat=%d lon=%d hdop=%d sats_v=%d\n",*/
/*            gnss.fix, gnss.sats_use, gnss.lat, gnss.lon, gnss.hdop_x100,*/
/*            gnss.sats_view_BD);*/
/*      }*/
/*      xSemaphoreGive(gnss_mutex);*/
/*    }*/
/*  }*/
/*}*/

static void webserver_task(void *arg) {
  if (xSemaphoreTake(wifi_ready, portMAX_DELAY) == pdPASS) {
    // run only if wifi_ready released
    DEB("setting webserver..\n");
    int sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {addr.sin_family = AF_INET,
                               addr.sin_port = htons(80),
                               addr.sin_addr.s_addr = INADDR_ANY};
    lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    lwip_listen(sock, 1);

    DEB("waiting client..\n");
    while (1) {
      int client = lwip_accept(sock, NULL, NULL);
      if (client >= 0)
        handle_client(client);
    }
  }
}

void user_init(void) {
  static const signed char uartTaskName[] = "uart_gnss";
  static const signed char parseTaskName[] = "parse_gnss";
  static const signed char webTaskName[] = "web_serv";

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

  /*gnss_mutex = xSemaphoreCreateMutex();*/
  /*if (gnss_mutex == NULL) {*/
  /*  DEB("Failed to create GNSS semaphore\n");*/
  /*  system_restart();*/
  /*}*/

  vSemaphoreCreateBinary(wifi_ready);
  if (wifi_ready == NULL) {
    DEB("Failed to create wifi_ready semaphore\n");
    system_restart();
  }

  // get GNSS string from incoming UART
  xTaskCreate(uart_gnss_task, uartTaskName, 2048, NULL, 5, NULL);
  DEB("UART GNSS reading task registered\n");
  // parse recived NMEA sentence
  /*xTaskCreate(parse_gnss_task, parseTaskName, 2048, NULL, 3, NULL);*/
  /*DEB("NMEA parser task registered\n");*/
  // web server
  wifi_init();
  xTaskCreate(webserver_task, webTaskName, 2048, NULL, 2, NULL);
  DEB("webserver task registered\n");
}
