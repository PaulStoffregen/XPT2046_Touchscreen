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

#include "XPT2046_Touchscreen.h"

#define Z_THRESHOLD     400
#define Z_THRESHOLD_INT	75
#define MSEC_THRESHOLD  3
#define SPI_SETTING     SPISettings(2000000, MSBFIRST, SPI_MODE0)

static XPT2046_Touchscreen 	*isrPinptr;
void isrPin(void);

bool XPT2046_Touchscreen::begin(uint16_t width, uint16_t height, TS_Calibration cal)
{
  SPI.begin();
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  if (255 != tirqPin) {
    pinMode( tirqPin, INPUT );
    attachInterrupt(digitalPinToInterrupt(tirqPin), isrPin, FALLING);
    isrPinptr = this;
  }

  _width = width;
  _height = height;

  this->setCalibration(cal);
  return true;
}

void isrPin( void )
{
  XPT2046_Touchscreen *o = isrPinptr;
  o->isrWake = true;
}

TS_Point XPT2046_Touchscreen::getPoint()
{
  update();

  int16_t x = xraw, y = yraw;

  this->rotateRaw(x, y);

  return TS_Point(x, y, zraw);
}

bool XPT2046_Touchscreen::tirqTouched()
{
  return (isrWake);
}

bool XPT2046_Touchscreen::touched()
{
  update();
  return (zraw >= Z_THRESHOLD);
}

void XPT2046_Touchscreen::readData(uint16_t *x, uint16_t *y, uint8_t *z)
{
  update();

  int16_t tempX = xraw, tempY = yraw;
  rotateRaw(tempX, tempY);
  
  *x = tempX;
  *y = tempY;
  *z = zraw;
}

bool XPT2046_Touchscreen::bufferEmpty()
{
  return ((millis() - msraw) < MSEC_THRESHOLD);
}

static int16_t besttwoavg( int16_t x , int16_t y , int16_t z ) {
  int16_t da, db, dc;
  int16_t reta = 0;
  if ( x > y ) da = x - y; else da = y - x;
  if ( x > z ) db = x - z; else db = z - x;
  if ( z > y ) dc = z - y; else dc = y - z;

  if ( da <= db && da <= dc ) reta = (x + y) >> 1;
  else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
  else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

  return (reta);
}

// TODO: perhaps a future version should offer an option for more oversampling,
//       with the RANSAC algorithm https://en.wikipedia.org/wiki/RANSAC

void XPT2046_Touchscreen::update()
{
  int16_t data[6];

  if (!isrWake) return;
  uint32_t now = millis();
  if (now - msraw < MSEC_THRESHOLD) return;

  SPI.beginTransaction(SPI_SETTING);
  digitalWrite(csPin, LOW);
  SPI.transfer(0xB1 /* Z1 */);
  int16_t z1 = SPI.transfer16(0xC1 /* Z2 */) >> 3;
  int z = z1 + 4095;
  int16_t z2 = SPI.transfer16(0x91 /* X */) >> 3;
  z -= z2;
  if (z >= Z_THRESHOLD) {
    SPI.transfer16(0x91 /* X */);  // dummy X measure, 1st is always noisy
    data[0] = SPI.transfer16(0xD1 /* Y */) >> 3;
    data[1] = SPI.transfer16(0x91 /* X */) >> 3; // make 3 x-y measurements
    data[2] = SPI.transfer16(0xD1 /* Y */) >> 3;
    data[3] = SPI.transfer16(0x91 /* X */) >> 3;
  }
  else data[0] = data[1] = data[2] = data[3] = 0;	// Compiler warns these values may be used unset on early exit.
  data[4] = SPI.transfer16(0xD0 /* Y */) >> 3;	// Last Y touch power down
  data[5] = SPI.transfer16(0) >> 3;
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();
  //Serial.printf("z=%d  ::  z1=%d,  z2=%d  ", z, z1, z2);
  if (z < 0) z = 0;
  if (z < Z_THRESHOLD) { //	if ( !touched ) {
    // Serial.println();
    zraw = 0;
    if (z < Z_THRESHOLD_INT) { //	if ( !touched ) {
      if (255 != tirqPin) isrWake = false;
    }
    return;
  }
  zraw = z;

  // Average pair with least distance between each measured x then y
  //Serial.printf("    z1=%d,z2=%d  ", z1, z2);
  //Serial.printf("p=%d,  %d,%d  %d,%d  %d,%d", zraw,
  //data[0], data[1], data[2], data[3], data[4], data[5]);
  int16_t x = besttwoavg( data[0], data[2], data[4] );
  int16_t y = besttwoavg( data[1], data[3], data[5] );

  //Serial.printf("    %d,%d", x, y);
  //Serial.println();
  if (z >= Z_THRESHOLD) {
    msraw = now;	// good read completed, set wait
    xraw = x;
    yraw = y;
  }
}

TS_Point XPT2046_Touchscreen::getPixel() {
  update();
  
#if defined(SWAP_AXES) && SWAP_AXES
  uint16_t xPixel = (uint16_t)(_cal_dx * (yraw - _cal_vj1) / _cal_dvj + CAL_OFFSET);
  uint16_t yPixel = (uint16_t)(_cal_dy * (xraw - _cal_vi1) / _cal_dvi + CAL_OFFSET);
#else
  uint16_t xPixel = (uint16_t)(_cal_dx * (xraw - _cal_vi1) / _cal_dvi + CAL_OFFSET);
  uint16_t yPixel = (uint16_t)(_cal_dy * (yraw - _cal_vj1) / _cal_dvj + CAL_OFFSET);
#endif
  
  this->rotateCal(xPixel, yPixel);

  return TS_Point(xPixel, yPixel, zraw);
}

void XPT2046_Touchscreen::getCalibrationPoints(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2) {
  x1 = y1 = CAL_OFFSET;
  x2 = _width - CAL_OFFSET;
  y2 = _height - CAL_OFFSET;
}

void XPT2046_Touchscreen::setCalibration (TS_Calibration cal) {
  _cal_dx = _width - 2 * CAL_OFFSET;
  _cal_dy = _height - 2 * CAL_OFFSET;

  _cal_vi1 = cal.vi1;
  _cal_vj1 = cal.vj1;
  _cal_dvi = (int32_t)cal.vi2 - cal.vi1;
  _cal_dvj = (int32_t)cal.vj2 - cal.vj1;
}

void XPT2046_Touchscreen::saveCalibrationToEEPROM(TS_Calibration &cal, unsigned int eepromSlot){
  eepromSlot = eepromSlot * sizeof(cal);
  
  EEPROM.put(eepromSlot, cal);
}

TS_Calibration XPT2046_Touchscreen::getEEPROMCalibration(unsigned int eepromSlot){
  eepromSlot = eepromSlot * sizeof(TS_Calibration);
  TS_Calibration cal;

  EEPROM.get(eepromSlot, cal);

  return cal;
}

void XPT2046_Touchscreen::rotateRaw(int16_t &x, int16_t &y){
  int16_t tempX = x, tempY = y;
  
  switch (rotation) {
    case 0:
      x = 4095 - tempY;
      y = tempX;
      break;
    case 1:
      break;
    case 2:
      x = tempY;
      y = 4095 - tempX;
      break;
    default: // 3
      x = 4095 - tempX;
      y = 4095 - tempY;
  }
}

void XPT2046_Touchscreen::rotateCal(uint16_t &x, uint16_t &y){
  uint16_t tempX = x, tempY = y;
  
  switch (rotation) {
    case 0:
      x = _height - tempY;
      y = tempX;
      break;
    case 1:
      break;
    case 2:
      x = tempY;
      y = _width - tempX;
      break;
    default: // 3
      x = _width - tempX;
      y = _height - tempY;
  }
}
