#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Keypad.h>

#include "wifi_config.h"

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/



constexpr uint8_t kScreenWidth = 128;
constexpr uint8_t kScreenHeight = 32;
constexpr int8_t kOledResetPin = -1;
constexpr uint8_t kOledI2cAddress = 0x3C;
constexpr int kOledSdaPin = 21;
constexpr int kOledSclPin = 22;


// Declaration for 4x4 keypad
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[COLS] = {12, 14, 27, 26}; 
byte colPins[ROWS] = {25, 33, 32, 18}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Global variables for status and timer
String flameStatus = "On";
unsigned long armedTime = 0; // To store when the 'Armed' state began
const int timerDuration = 10; // 10 seconds


constexpr uint8_t kMaxConnectionAttempts = 20;
constexpr uint16_t kRetryDelayMs = 500;

Preferences preferences;
WebServer server(80);

String endpointConfig;

String htmlEscape(const String &input)
{
    String escaped;
    escaped.reserve(input.length());
    for (uint16_t i = 0; i < input.length(); ++i)
    {
        const char c = input.charAt(i);
        switch (c)
        {
        case '&':
            escaped += F("&amp;");
            break;
        case '<':
            escaped += F("&lt;");
            break;
        case '>':
            escaped += F("&gt;");
            break;
        case 39:
            escaped += F("&#39;");
            break;
        case '"':
            escaped += F("&quot;");
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

void sendConfigurationPage(const String &message = String())
{
    String html = F("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Device Configuration</title><style>body{font-family:Arial,sans-serif;margin:40px;}label{display:block;margin-bottom:8px;font-weight:bold;}input[type=text]{width:100%;max-width:480px;padding:8px;margin-bottom:12px;}button{padding:10px 16px;margin-right:10px;font-size:14px;cursor:pointer;}form{margin-bottom:20px;}p.status{color:green;}</style></head><body><h1>Device Configuration</h1>");
    if (message.length() > 0)
    {
        html += F("<p class='status'>");
        html += htmlEscape(message);
        html += F("</p>");
    }
    html += F("<form method='POST' action='/save'><label for='endpoint'>Endpoint URL</label><input type='text' id='endpoint' name='endpoint' value='");
    html += htmlEscape(endpointConfig);
    html += F("' placeholder='https://example.com/api'><button type='submit'>Save</button></form><form method='POST' action='/restart'><button type='submit'>Restart Device</button></form></body></html>");
    server.send(200, "text/html", html);
}

void handleRoot()
{
    sendConfigurationPage();
}

void handleSave()
{
    if (server.hasArg("endpoint"))
    {
        endpointConfig = server.arg("endpoint");
        preferences.putString("endpoint", endpointConfig);
        sendConfigurationPage(F("Configuration saved."));
    }
    else
    {
        server.send(400, "text/plain", "Missing endpoint parameter");
    }
}

void handleRestart()
{
    server.send(200, "text/html", F("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><title>Restarting</title></head><body><h1>Device Restarting</h1><p>The device will restart to apply the new settings.</p></body></html>"));
    delay(500);
    ESP.restart();
}

void setupServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/restart", HTTP_POST, handleRestart);
    server.begin();
}
void connectToWifi()
{
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < kMaxConnectionAttempts)
    {
        delay(kRetryDelayMs);
        Serial.print('.');
        const String attemptsMessage = String("Attempt ") + String(attempts + 1) + String("/") + String(kMaxConnectionAttempts);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("Failed to connect to WiFi. Check credentials and signal strength.");

    }
}


/*Change to your screen resolution*/
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */



/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX = 0, touchY = 0;

    bool touched = false;//tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    preferences.begin("config", false);
    endpointConfig = preferences.getString("endpoint", String());

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    connectToWifi();
    setupServer();


    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 2 ); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );


    ui_init();

    Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);

    server.handleClient();

    // Placeholder loop. Future functionality (LED control, touchscreen handling, etc.)
    // will be implemented here.
    if (WiFi.status() != WL_CONNECTED)
    {
        connectToWifi();
        delay(1500);
    }
        // --- Keypad Input Handling ---
    char customKey = customKeypad.getKey();
    if (customKey) {
    if (customKey == 'A') {
      flameStatus = "Armed";
      armedTime = millis(); // Start the timer
    } else if (customKey == 'B') {
      flameStatus = "Defused";
    } else if (customKey == 'C') {
      flameStatus = "Exploded";
    } else {
      flameStatus = "On"; // Default case for other keys
    }
    
    Serial.print("Key Pressed: ");
    Serial.println(customKey);
    // Force immediate display update after a key press

}   
  

  // A small delay to keep the loop from running too fast if no keys are pressed
  delay(100);
}

