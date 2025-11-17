# Project Overview
This is a "digital flame". A prop used in a game of laser tag that will be armed, planted and/or defused by the teams in order to win rounds.


# Project Architecture

This is a PlatformIO project for an embedded system, an ESP32-based device with a display, often referred to as a "Cheap Yellow Display" (CYD).

## Architecture Breakdown

*   **Core:** It runs on an ESP32 using the Arduino framework. The main application logic is in `src/main.cpp`.
*   **Display & UI:** It uses the `lvgl` graphics library to create a user interface on a color display, driven by the `TFT_eSPI` library. Touch input is handled by the `XPT2046_Touchscreen` library. The custom UI components (screens, widgets) are defined in the `lib/ui` directory, which is a common structure for UIs designed with tools like SquareLine Studio.
*   **Connectivity:** The project includes `WiFi` and `WebServer` libraries, indicating it can connect to a Wi-Fi network and host a web server, for remote control or data monitoring. The prop will need to make a POST request to an API endpoint and send its status via a simple JSON payload.
*   **Input:** Besides the touchscreen, it uses the `Keypad` library to use run a 4x4 matrix keypad that wil be used to input the defuse code. It will also have two monentary switches that will have to be held down to arm the prop.
*   **Effects:** The prop will use a speaker connected to the CYD and ws2812b led strips for sound and color effects.
*   **File System:** It includes `LittleFS` and `SPIFFS`, which are file systems for the ESP32's flash memory, allowing it to store configuration files, assets, or web pages.
*   **Configuration:** The `platformio.ini` file defines the project's dependencies, build flags, and board configuration. The `include/wifi_config.h` file holds Wi-Fi credentials.

In summary, this is a feature-rich embedded application for an ESP32 device with a touchscreen, providing a local graphical interface and remote web-based interaction.

## Latest maintenance notes

* Code is split into focused modules under `src/`:
  * `display.*` sets up LVGL, the CYD display, and the touch stub.
  * `wifi_manager.*` handles Wi-Fi connection and reconnection loops.
  * `http_server.*` hosts the configuration UI and persists the endpoint setting.
  * `input.*` reads keypad input and converts it to flame status values.
  * `lighting.*` and `audio.*` are ready-made extension points for WS2812 and speaker behavior.
  * `utils.*` holds small shared helpers like HTML escaping.
* `src/main.cpp` now orchestrates the system by calling into the modules above so features can evolve independently.
* Keep future changes similarly well-commented so new contributors can navigate the firmware quickly.
