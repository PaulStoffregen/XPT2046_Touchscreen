/*
   Copyright (c) 2017 Timo Meyer
*/

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
  touch.begin(display.width(), display.height());

  //Draw background
  display.fillScreen(BLACK);

  //Calibrate
  calibrate();  // No rotation!!

  //Clear Screen
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

//------------------------------------------------------------------------

void calibrate() {
  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;

  //Get calibration-Points
  touch.getCalibrationPoints(x1, y1, x2, y2);
  //Draw and get RAW-Refference
  calibratePoint(x1, y1, vi1, vj1);
  delay(1000);

  //The same again but with new points
  calibratePoint(x2, y2, vi2, vj2);

  TS_Calibration newCalibration = TS_Calibration(vi1, vj1, vi2, vj2);
  touch.setCalibration(newCalibration);

  //Write results into a buffer
  char buf[80];
  snprintf(buf, sizeof(buf), "%d,%d,%d,%d", (int)vi1, (int)vj1, (int)vi2, (int)vj2);

  //Print it to the Display
  display.setCursor(0, 25);
  display.setTextColor(WHITE);
  display.print("setCalibration params:");

  display.setCursor(0, 50);
  display.print(buf);

  display.setCursor(0, 75);
  display.print("Writing to EEPROM...");

  //Save to EEPROM
  touch.saveCalibrationToEEPROM(newCalibration);

  display.setCursor(0, 100);
  display.print("Finish!");

  display.setCursor(0, 125);
  display.print("Start drawing");

  delay(3000);
}

//------------------------------------------------------------------------

static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  display.drawFastHLine(x - 8, y, 17, WHITE);
  display.drawFastVLine(x, y - 8, 17, WHITE);
  display.drawCircle(x, y, 8, WHITE);

  //Wait until touched
  while (!touch.touched()) {
    delay(10);
  }

  //Get uncallibrated Raw touchpoint
  TS_Point p = touch.getPoint();

  vi = p.x;
  vj = p.y;

  // Erase by overwriting with black
  display.drawFastHLine(x - 8, y, 17, BLACK);
  display.drawFastVLine(x, y - 8, 17, BLACK);
  display.drawCircle(x, y, 8, BLACK);
}
