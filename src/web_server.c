#include "web_server.h"

// import web page
ICACHE_FLASH_ATTR
#include "embed_web/index.html.hex" 
// unsigned char web_index_html[]
// unsigned int web_index_html_len

ICACHE_FLASH_ATTR
#include "embed_web/style.css.hex" 
// unisgned char web_style_css[]
// unisgned int web_style_css


ICACHE_FLASH_ATTR
#include "embed_web/app.min.js.hex" 
// unsigned char web_app_min_js[]
// unsigned int web_app_min_js

xSemaphoreHandle wifi_ready = NULL;

static bool reason_error_auth(uint8_t reason) {
  return (reason == REASON_AUTH_FAIL ||
          reason == REASON_4WAY_HANDSHAKE_TIMEOUT ||
          reason == REASON_ASSOC_FAIL || reason == REASON_AUTH_EXPIRE);
}
static bool reason_error_ap(uint8_t reason) {
  return (reason == REASON_NO_AP_FOUND || reason == REASON_BEACON_TIMEOUT ||
          reason == REASON_ASSOC_TOOMANY || reason == REASON_HANDSHAKE_TIMEOUT);
}

void wifi_event(System_Event_t *event) {
  struct ip_info info;
  uint8_t reason = event->event_info.disconnected.reason;
  wifi_get_ip_info(STATION_IF, &info);

  switch (event->event_id) {
  case EVENT_STAMODE_GOT_IP:
    xSemaphoreGive(wifi_ready);
    break;
  case EVENT_STAMODE_DISCONNECTED:
    if (reason_error_auth(reason)) {
      os_printf("authentication failed\n");
      wifi_station_disconnect();
      xSemaphoreTake(wifi_ready, portMAX_DELAY);
      break;
    }
    if (reason_error_ap(reason)) {
      os_printf("AP not found\n");
      wifi_station_disconnect();
      xSemaphoreTake(wifi_ready, portMAX_DELAY);
      break;
    }
    os_printf("reason %d\n", reason);
    os_printf("check esp_wifi.h; REASON_* enum\n");
    break;
  default:
    break;
  }
}

void wifi_init(void) {
  WIFI_MODE mode = STATION_MODE;
  struct station_config cfg = {
      .ssid = SSID, .password = PASSWD, .bssid_set = 0, .bssid = "0"};

  wifi_set_event_handler_cb(wifi_event);

  if (wifi_set_opmode(mode)) {
    DEB("Set STATION mode\n");
  } else {
    DEB("Failed to set STATION mode\n");
    system_restart();
  };

  if (wifi_station_set_config(&cfg)) {
    DEB("Set cfg: SSID=%s; PASSWD=(see include/secret.h)\n", SSID);
  } else {
    DEB("Failed to set cfg\n");
    system_restart();
  };

  if (wifi_station_dhcpc_status() != DHCP_STARTED) {
    if (!wifi_station_dhcpc_start()) {
      DEB("Failed to start DHCP\n");
      system_restart();
    }
  } else {
    DEB("Started DHCP\n");
  }
  wifi_station_connect();
  xSemaphoreTake(wifi_ready, portMAX_DELAY);
  // will be released by EVENT_STAMODE_GOT_IP
}

int send_all(int sock, const char *buf, int len) {
  int sent = 0;
  while (sent < len) {
    int r = lwip_send(sock, buf + sent, len - sent, 0);
    if (r <= 0)
      return r;
    sent += r;
  }
  return sent;
}
