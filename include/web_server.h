#pragma once
#include "esp_common.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "secrets.h"
#include "uart_def.h"

#ifdef DEBUG
#define DEB(...) os_printf(__VA_ARGS__)
#else
#define DEB(...)
#endif

#define RECV_BUF_LEN 512
#define HDR_LEN 160

extern xSemaphoreHandle wifi_ready;
extern xQueueHandle nmea_queue;

static bool reason_error_auth(uint8_t reason);
static bool reason_error_ap(uint8_t reason);
void wifi_event(System_Event_t *event);
void wifi_init(void);
int send_file(int sock, const unsigned char *buf, int len);
int send_queue(int sock);
void handle_client(int client);
void send_hdr(int client, int hdr_type, int cont_len);
