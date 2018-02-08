#include "XPT2046_Touchscreen.h"
#include <ILI9341_t3.h>

// Modify the following lines to match your hardware
#define TFT_DC   15
#define TFT_CS   10
#define TFT_RST  4
//#define TFT_MOSI 11
//#define TFT_MISO 12
//#define TFT_CLK  13

//Some Displays need an extra Pin for the LED-Backlight. I used a I/O-Pin also to dim the Backlight.
#define TFT_LED  19

//And also the Touch-Pins for the XPT2046
#define TOUCH_CS  20
#define TOUCH_IRQ 21

#define BLACK   0x0000
#define WHITE   0xFFFF

ILI9341_t3 display = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST);
//ILI9341_t3 display = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_CLK, TFT_MISO);

XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

void setup() {
  //Activate backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  delay(1000);

  //Start display and touch
  display.begin();
  touch.begin(display.width(), display.height(), touch.getEEPROMCalibration());

  //Draw background
  display.fillScreen(BLACK);
}

void loop() {
  //If touched than draw
  if (touch.touched()) {
    //Get touched and calibrated position
    TS_Point p = touch.getPixel();

    display.fillCircle(p.x, p.y, 2, WHITE);
  }
}
