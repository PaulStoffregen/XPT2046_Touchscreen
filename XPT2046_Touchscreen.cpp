/* Touchscreen library for XPT2046 Touch Controller Chip
 * Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "XPT2046_Touchscreen.h"

#define Z_THRESHOLD     400
#define MSEC_THRESHOLD  3
#define SPI_SETTING     SPISettings(2000000, MSBFIRST, SPI_MODE0)

XPT2046_Touchscreen::XPT2046_Touchscreen(uint8_t cs)
{
	csPin = cs;
	msraw = 0x80000000;
	xraw = 0;
	yraw = 0;
	zraw = 0;
}

bool XPT2046_Touchscreen::begin()
{
	SPI.begin();
	pinMode(csPin, OUTPUT);
	digitalWrite(csPin, HIGH);
	return true;
}

TS_Point XPT2046_Touchscreen::getPoint()
{
	update();
	return TS_Point(xraw, yraw, zraw);
}

bool XPT2046_Touchscreen::touched()
{
	update();
	return (zraw >= Z_THRESHOLD);
}

void XPT2046_Touchscreen::readData(uint16_t *x, uint16_t *y, uint8_t *z)
{
	update();
	*x = xraw;
	*y = yraw;
	*z = zraw;
}

bool XPT2046_Touchscreen::bufferEmpty()
{
	return ((millis() - msraw) < MSEC_THRESHOLD);
}

void XPT2046_Touchscreen::update()
{
	uint32_t now = millis();
	if (now - msraw < MSEC_THRESHOLD) return;
	msraw = now;
	SPI.beginTransaction(SPI_SETTING);
	digitalWrite(csPin, LOW);
	SPI.transfer(0xB1 /* Z1 */);
	int16_t z1 = SPI.transfer16(0xC1 /* Z2 */) >> 3;
	int16_t z2 = SPI.transfer16(0x91 /* X */) >> 3;
	int16_t x  = SPI.transfer16(0xD0 /* Y */) >> 3;
	int16_t y  = SPI.transfer16(0) >> 3;
	digitalWrite(csPin, HIGH);
	SPI.endTransaction();
	int z = z1 + 4095 - z2;
	if (z < 0) z = 0;
	zraw = z;
	if (z >= Z_THRESHOLD) {
		xraw = x;
		yraw = y;
	}
	//Serial.printf("p=%d,  x=%d, y=%d\n", zraw, x, y);
}


