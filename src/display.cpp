#include "display.h"

#include <Arduino.h>

namespace
{
/* Change to your screen resolution */
constexpr uint16_t kScreenWidth = 240;
constexpr uint16_t kScreenHeight = 320;

lv_disp_draw_buf_t drawBuf;
lv_color_t colorBuf[kScreenWidth * kScreenHeight / 10];

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/**
 * Touch input handler. Hardware support may not be wired in yet, so we keep
 * this stub in place; once touch is available, set `touched` using the
 * appropriate driver call (for example, `tft.getTouch`).
 */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX = 0, touchY = 0;

    const bool touched = false; // Replace with actual touch read when hardware is present.

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /* Set the coordinates */
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print("Touch X: ");
        Serial.println(touchX);
        Serial.print("Touch Y: ");
        Serial.println(touchY);
    }
}

#if LV_USE_LOG != 0
void my_print(const char *buf)
{
    Serial.printf("%s", buf);
    Serial.flush();
}
#endif
} // namespace

TFT_eSPI tft = TFT_eSPI(kScreenWidth, kScreenHeight);

void initializeDisplay()
{
    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    tft.begin();
    tft.setRotation(2);

    lv_disp_draw_buf_init(&drawBuf, colorBuf, NULL, kScreenWidth * kScreenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = kScreenWidth;
    disp_drv.ver_res = kScreenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &drawBuf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

void showStartupStatus(const String &wifiStatus)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);

    tft.setCursor(12, 40);
    tft.println("Starting up...");

    tft.setTextSize(1);
    tft.setCursor(12, 80);
    tft.println("WiFi status:");
    tft.setTextSize(2);
    tft.setCursor(12, 100);
    tft.println(wifiStatus);
}
