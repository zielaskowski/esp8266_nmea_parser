#include "web_server.h"

// import web page
ICACHE_FLASH_ATTR
#include "embed_web/index.html.hex"
// unsigned char web_index_html[]
#define WEB_INDEX_HTML_LEN (sizeof(web_index_html) - 1)

ICACHE_FLASH_ATTR
#include "embed_web/style.css.hex"
// unisgned char web_style_css[]
#define WEB_STYLE_CSS_LEN (sizeof(web_style_css) - 1)

ICACHE_FLASH_ATTR
#include "embed_web/app.min.js.hex"
// unsigned char web_app_min_js[]
#define WEB_APP_MIN_JS_LEN (sizeof(web_app_min_js) - 1)

typedef struct {
  const char *path;
  const char *type;
  const unsigned char *content;
  const unsigned int len;
} web_file_t;

static const web_file_t files[] = {
    {"GET / ", "text/html", web_index_html, WEB_INDEX_HTML_LEN},
    {"GET /app.js", "application/javascript", web_app_min_js,
     WEB_APP_MIN_JS_LEN},
    {"GET /style.css", "text/css", web_style_css, WEB_STYLE_CSS_LEN},
    {"", "", (unsigned char *)" ", 1}};

enum { HTML, JS, CSS, ERROR };

// headers
ICACHE_FLASH_ATTR
#include "embed_web/header_ok.hex"
// unsigned char web_header_ok[]

ICACHE_FLASH_ATTR
#include "embed_web/header_error.hex"
// unsigned char web_header_error[]

typedef struct {
  const char *hdr;
} hdr_t;

static const hdr_t hdrs[] = {(const char *)web_header_ok,
                             (const char *)web_header_error};

enum { OK_200, ERROR_404 };

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

int send_all(int sock, const unsigned char *buf, int len) {
  int sent = 0;
  while (sent < len) {
    int r = lwip_send(sock, buf + sent, len - sent, 0);
    if (r <= 0)
      return r;
    sent += r;
  }
  return sent;
}

void handle_client(int client) {
  char rx[RECV_BUF_LEN];
  int rx_len = lwip_recv(client, rx, sizeof(rx), 0);
  if (rx_len <= 0) {
    lwip_close(client);
    return;
  }
  rx[rx_len] = '\0';
  DEB("recived: %s\n", rx);
  if (strncmp(rx, files[HTML].path, strlen(files[HTML].path)) == 0) {
    send_content(client, HTML, OK_200); // send HTML page
  } else if (strncmp(rx, files[JS].path, strlen(files[JS].path)) == 0) {
    send_content(client, JS, OK_200); // send java script
  } else if (strncmp(rx, files[CSS].path, strlen(files[CSS].path)) == 0) {
    send_content(client, CSS, OK_200); // send styles CSS
  } else {
    send_content(client, ERROR, ERROR_404); // 404 error
  }
  lwip_shutdown(client, SHUT_WR);
  lwip_close(client);
}

void send_content(int client, int file_type, int hdr_type) {
  char hdr[HDR_LEN];
  int hdr_len;
  hdr_len = snprintf(hdr, HDR_LEN, hdrs[OK_200].hdr, files[file_type].type,
                     files[file_type].len);
  send_all(client, (unsigned char *)hdr, hdr_len);
  send_all(client, files[file_type].content, files[file_type].len);
}
