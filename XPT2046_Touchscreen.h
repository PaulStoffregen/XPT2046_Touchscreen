/* Touchscreen library for XPT2046 Touch Controller Chip
   Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice, development funding notice, and this permission
   notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef _XPT2046_Touchscreen_h_
#define _XPT2046_Touchscreen_h_

#include "Arduino.h"
#include <SPI.h>
#include <EEPROM.h>

// On my display driver ICs i,j mapped to (width-y),x
// Flipping can be handled by order of calibration points, but not swapping
#if !defined(SWAP_AXES)
#  define SWAP_AXES 1
#endif

#if ARDUINO < 10600
#error "Arduino 1.6.0 or later (SPI library) is required"
#endif

class TS_Calibration {
  public:
    TS_Calibration(void)
      : vi1(0), vj1(0), vi2(0), vj2(0) {}

    TS_Calibration(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
      : vi1(x1), vj1(y1), vi2(x2), vj2(y2) {}

    uint16_t vi1, vj1, vi2, vj2;
};

class TS_Point {
  public:
    TS_Point(void) : x(0), y(0), z(0) {}
    TS_Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
    bool operator==(TS_Point p) {
      return ((p.x == x) && (p.y == y) && (p.z == z));
    }
    bool operator!=(TS_Point p) {
      return ((p.x != x) || (p.y != y) || (p.z != z));
    }
    int16_t x, y, z;
};

class XPT2046_Touchscreen {
  public:
    uint16_t CAL_OFFSET = 20;

    constexpr XPT2046_Touchscreen(uint8_t cspin, uint8_t tirq = 255)
      : csPin(cspin), tirqPin(tirq) { }
    bool begin(uint16_t width = 0, uint16_t height = 0, TS_Calibration cal = TS_Calibration());
    TS_Point getPoint();
    TS_Point getPixel();
    bool tirqTouched();
    bool touched();
    void readData(uint16_t *x, uint16_t *y, uint8_t *z);
    bool bufferEmpty();
    uint8_t bufferSize() {
      return 1;
    }
    void setRotation(uint8_t n) {
      rotation = n % 4;
    }
    void setWidth(uint16_t width){
      _width = width;
    }
    void setHeight(uint16_t height){
      _height = height;
    }

    void getCalibrationPoints(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2);
    void setCalibration(TS_Calibration cal);
    TS_Calibration getCalibrationObject(uint16_t vi1, uint16_t vj1, uint16_t vi2, uint16_t vj2) {
      return TS_Calibration(vi1, vj1, vi2, vj2);
    }

    void saveCalibrationToEEPROM(TS_Calibration &cal, unsigned int eepromSlot = 0);
    TS_Calibration getEEPROMCalibration(unsigned int eepromSlot = 0);

    // protected:
    volatile bool isrWake = true;

  private:
    void update();
    void rotate(uint16_t &x, uint16_t &y);
    
    uint8_t csPin, tirqPin, rotation = 1;
    int16_t xraw = 0, yraw = 0, zraw = 0, _width = 0, _height = 0;
    uint32_t msraw = 0x80000000;

    int32_t _cal_dx = 0, _cal_dy = 0, _cal_dvi = 0, _cal_dvj = 0;
    uint16_t _cal_vi1 = 0, _cal_vj1 = 0;
};

#endif
