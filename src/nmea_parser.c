#include "nmea_parser.h"

xSemaphoreHandle gnss_mutex;
gnss_state_t gnss;

uint32_t float_decimal(const char *s) {
  // return decimal part of float (5 digits max)
  // string must be NULL terminated
  uint32_t val = 0;
  uint8_t val_len = 0;

  // decimal part
  while (*s >= '0' && *s <= '9' && val_len < 5) {
    val = val * 10 + (*s - '0');
    s++;
    val_len++;
  }
  return val;
}

uint16_t float_fraction_e4(const char *s) {
  // return fractional part of float, 4 digits
  // 4 digits gives resolution of 11m to 4m for latitude or longitude
  // string must be NULL terminated
  uint16_t val = 0;
  uint8_t val_len = 0;
  while (*s != '.')
    s++;
  s++;
  while (*s >= '0' && *s <= '9' && val_len < 4) {
    val = val * 10 + (*s - '0');
    s++;
    val_len++;
  }
  // round to 4 digits if missing
  while (val_len < 4) {
    val *= 10;
    val_len++;
  }
  return val;
}

int32_t degmin_to_e4(const char *s) {
  // store lon/lat as 10th-mili degress (Decimal Degrees actaully)
  // avoid floating numbers
  // GNSS dat is provided acc. to NMEA-0.183
  // Latitude is represented as   ddmm.mmmm
  // longitude is represented as dddmm.mmmm
  return (float_decimal(s) * 10e4) + (float_fraction_e4(s));
}

uint16_t float_to_e2(const char *s) {
  // convert float to (int) num*10^2
  // no floating numbers
  // two digits of fraction
  uint16_t dec = (uint16_t)(float_decimal(s) * 100);
  uint16_t frac = (uint16_t)float_fraction_e4(s) / 100;
  return dec + frac;
}

void parseGGA(nmea_msg_t *msg) {
  // $GPGGA,time,lat,NS,lon,EW,qual_fix,sats_use,hdop,....
  char *p = msg->line;
  uint8_t field = 0;
  uint8_t field_start = 0;
  uint8_t field_end = 0;
  char field_txt[16];
  uint8_t field_len;

  while (*p) {
    if (*p == ',') {
      field_len = field_end - field_start - 1;
      if (field_len >= sizeof(field_txt)) {
        field_len = sizeof(field_txt) - 1;
      }
      field_txt[field_len] = '\0';
      if (field_len > 0) {
        memcpy(field_txt, &msg->line[field_start + 1], field_len);
        if (field == 2) {
          // latitude
          gnss.lat = degmin_to_e4(field_txt);
        } else if (field == 3) {
          // South|North
          if (*(p + 1) == 'S')
            gnss.lat = -gnss.lat;
        } else if (field == 4) {
          // longitude
          gnss.lon = degmin_to_e4(field_txt);
        } else if (field == 5) {
          // East|West
          if (*(p + 1) == 'W')
            gnss.lon = -gnss.lon;
        } else if (field == 6) {
          // fix?
          gnss.fix = atoi(field_txt);
        } else if (field == 7) {
          // number of satelits
          gnss.sats_use = atoi(field_txt);
        } else if (field == 8) {
          // HDOP
          gnss.hdop_x100 = float_to_e2(field_txt);
          return;
        }
      }

      field_start = field_end;
      field++;
    }
    p++;
    field_end++;
  }
}

uint8_t parseGSV(nmea_msg_t *msg) {
  // $GPGSV,tot_msg,msg_no,sats_view,.....
  uint8_t sats_view = 0;
  char *p = msg->line;
  uint8_t field = 0;
  uint8_t field_start = 0;
  uint8_t field_end = 0;
  char field_txt[16];
  uint8_t field_len;

  while (*p) {
    if (*p == ',') {
      field_len = field_end - field_start - 1;
      if (field_len >= sizeof(field_txt)) {
        field_len = sizeof(field_txt) - 1;
      }
      field_txt[field_len] = '\0';
      if (field_len > 0) {
        memcpy(field_txt, &msg->line[field_start + 1], field_len);
        if (field == 3) {
          // sats in view
          sats_view = atoi(field_txt);
          return sats_view;
        }
      }
      field_start = field_end;
      field++;
    }
    p++;
    field_end++;
  }
  return 0;
}
