# esp8266_nmea_parser

nmea parser for esp8266. Parsing only GGS and GSV messages and extracts
following data:

- longitude / latitude. as per NMEA the format recived is ddmm.mmmm | dddmm.mmmm.
Saved internally as (lat|lon)*10^4
- fix no|yes|dgps
- number of satelites used for fix
- horizontal dilution of position (saved internally as htop*10e2)
- number of satelites in view for each system (GPS|GLONASS|BEIDOU|GNSS|GALILEO)

Data presented as web server

## techincalities

- compiled with platformio
- framework esp8266 rtos-sdk
- developed on data from G-NiceRF chip
- data comming expected on UART0
- use -DDEBUG compile flag to allow debug messages on UART1
