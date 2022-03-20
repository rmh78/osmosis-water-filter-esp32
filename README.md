# Osmosis Water Filter with ESP32

This project contains the program controlling my osmotic water filter using an ESP32.

## Components

- Heltec WiFi LoRa 32 (V2)
- HX711 + scale sensor
- TDS meter sensor
- 4 channel relay module

## TODOs

- [x] Flush membrane and standing water before filtering water to prevent contamination.
- [x] Flush membrane after filtering water to prevent membrane from calcification.
- [x] Only flush when necessary by storing last flush time.
- [x] Flush membrane and standing water every 4 hours to prevent contamination.
- [ ] Flush unlimited time for filter change.
- [x] Disinfection program (filter small time amount, rest for 15 minutes, repeat 10 times, flush membrane, filter water).
- [ ] Add button to trigger disinfection program.
- [ ] HTTP REST call to send event-status to backend.

## Configuration

Create the file `platformio.ini`:

```ini
[env:heltec_wifi_lora_32_V2]
platform = espressif32
board = heltec_wifi_lora_32_V2
build_flags = 
	-DWIFI_SSID='"change-me"'
	-DWIFI_PASSWORD='"change-me"'
framework = arduino
lib_deps = 
	heltecautomation/Heltec ESP32 Dev-Boards@^1.1.0
	bogde/HX711@^0.7.5
	arkhipenko/TaskScheduler@^3.6.0
	robtillaart/StopWatch@^0.3.2
monitor_speed = 115200
```
