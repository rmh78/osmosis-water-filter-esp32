# Osmosis Water Filter with ESP32

This project contains the program controlling my osmotic water filter using an ESP32.

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/cf0d0675-a4cf-40d4-b81a-a10c9e8bd8d5" width="400"/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/ecc0cbdd-011f-45b6-871b-4c5f7b65c3a1" width="400"/>

<br/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/a7340757-c1f6-49b8-85c1-667cc4f1c164" width="800"/>

<br/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/23d2618b-3ec2-42b9-96fe-23af8ee97a89" width="400"/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/28cdc3b1-99bc-404a-bbaa-b2231584a57f" width="400"/>

<br/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/a85c56ce-077c-4368-91e6-4d0fd9aaec03" width="800"/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/795140ad-6fbc-4879-9a73-3eb7211ca349" width="800"/>

<br/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/16824b0a-f303-46b3-ab3d-1b29961d2013" width="400"/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/dab51afa-1b45-4f0d-a840-ff9fc813e5d4" width="400"/>

<img src="https://github.com/rmh78/osmosis-water-filter-esp32/assets/28454466/8bac14c5-4791-43b9-836b-32f0ffd4fa74" width="800"/>


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
- [x] Disinfection program (filter small time amount, rest for 15 minutes, repeat 10 times, flush membrane, filter water).
- [x] Add button to trigger disinfection program.
- [x] HTTP REST call to send event-status to backend.

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
