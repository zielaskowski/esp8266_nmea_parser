#pragma once
#include "esp_common.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portable.h"
#include "freertos/semphr.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "secrets.h"

#ifdef DEBUG
#define DEB(...) os_printf(__VA_ARGS__)
#else
#define DEB(...)
#endif

extern xSemaphoreHandle wifi_ready;

static bool reason_error_auth(uint8_t reason);
static bool reason_error_ap(uint8_t reason);
void wifi_event(System_Event_t *event);
void wifi_init(void);
int send_all(int sock, const char *buf, int len);
