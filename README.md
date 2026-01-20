# esp8266_nmea_parser

nmea parser for esp8266. Reading raw NMEA from uart and presenting as web page.
Whole parsing is on client side, using gps.js lib [https://github.com/rawify/GPS.js]
Web page show raw NMEA stream and some basic info:

- longitude / latitude. as per NMEA the format recived is ddmm.mmmm | dddmm.mmmm.
Saved internally as (lat|lon)*10^4
- fix no|yes|dgps
- number of satelites used for fix
- horizontal dilution of position (saved internally as htop*10e2)
- number of satelites in view for each system (GPS|GLONASS|BEIDOU|GNSS|GALILEO)

## techincalities

- compiled with platformio
- framework esp8266 rtos-sdk
- developed on data from G-NiceRF chip
- data comming expected on UART0
- use -DDEBUG compile flag to allow debug messages on UART1
- web server working in STATION mode (so AP with net is necessery)
- web serve simpply /index.html with minimum js.
- Heavy parsing is done by gps.js lib (downloaded from CDN).
- js in index.html reconects every few second asking for more data on /nmea
