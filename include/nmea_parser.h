#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef DEBUG
#define DEB(...) os_printf(__VA_ARGS__)
#else
#define DEB(...)
#endif

#define NMEA_MAX_LEN 82
#define NMEA_QUEUE_LEN 10

typedef struct {
  uint8_t fix;          // 0 = no fix, 1 = fix, 2 = dgps fix
  uint8_t sats_use;     // number of satelites used for fix
  uint8_t sats_view_GP; // number of satelites view from GPS
  uint8_t sats_view_GL; // number of satelites view from GLONAS
  uint8_t sats_view_GB; // number of satelites view from BEIDOU
  uint8_t sats_view_BD; // number of satelites view from BEIDOU
  uint8_t sats_view_GN; // number of satelites view from GNSS
  uint8_t sats_view_GA; // number of satelites view from GALILEO
  int32_t lat;          // latitute *1e4
  int32_t lon;          // longitude *1e4
  uint16_t hdop_x100;   // Horizontal Dilution of Precision x100
} gnss_state_t;

typedef struct {
  char line[NMEA_MAX_LEN];
} nmea_msg_t;

extern xSemaphoreHandle gnss_mutex;
extern gnss_state_t gnss;
extern xQueueHandle nmea_queue;

uint32_t float_decimal(const char *s);
uint16_t float_fraction_e4(const char *s);
int32_t degmin_to_e4(const char *s);
uint16_t float_to_e2(const char *s);
void parseGGA(nmea_msg_t *msg);
uint8_t parseGSV(nmea_msg_t *msg);
