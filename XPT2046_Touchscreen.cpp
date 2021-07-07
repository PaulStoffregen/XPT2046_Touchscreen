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
#define Z_THRESHOLD_INT	75
#define MSEC_THRESHOLD  3
#define SPI_SETTING     SPISettings(2000000, MSBFIRST, SPI_MODE0)

#define ADC_VBAT  0
#define ADC_AUXIN 1
#define ADC_TEMP  2

static XPT2046_Touchscreen 	*isrPinptr;
void isrPin(void);

bool XPT2046_Touchscreen::begin(SPIClass &wspi)
{
	_pspi = &wspi;
	_pspi->begin();
	pinMode(csPin, OUTPUT);
	digitalWrite(csPin, HIGH);
	if (255 != tirqPin) {
		pinMode( tirqPin, INPUT );
		attachInterrupt(digitalPinToInterrupt(tirqPin), isrPin, FALLING);
		isrPinptr = this;
	}
	return true;
}

#if defined(_FLEXIO_SPI_H_)
#define FLEXSPI_SETTING     FlexIOSPISettings(2000000, MSBFIRST, SPI_MODE0)
bool XPT2046_Touchscreen::begin(FlexIOSPI &wflexspi)
{
	_pspi = nullptr; // make sure we dont use this one... 
	_pflexspi = &wflexspi;
	_pflexspi->begin();
	pinMode(csPin, OUTPUT);
	digitalWrite(csPin, HIGH);
	if (255 != tirqPin) {
		pinMode( tirqPin, INPUT );
		attachInterrupt(digitalPinToInterrupt(tirqPin), isrPin, FALLING);
		isrPinptr = this;
	}
	return true;
}
#endif


ISR_PREFIX
void isrPin( void )
{
	XPT2046_Touchscreen *o = isrPinptr;
	o->isrWake = true;
}

TS_Point XPT2046_Touchscreen::getPoint()
{
	update();
	return TS_Point(xraw, yraw, zraw);
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
	*x = xraw;
	*y = yraw;
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
	int z;
	if (!isrWake) return;
	uint32_t now = millis();
	if (now - msraw < MSEC_THRESHOLD) return;
	if (_pspi) {
		_pspi->beginTransaction(SPI_SETTING);
		digitalWrite(csPin, LOW);
		_pspi->transfer(0xB1 /* Z1 */);
		int16_t z1 = _pspi->transfer16(0xC1 /* Z2 */) >> 3;
		z = z1 + 4095;
		int16_t z2 = _pspi->transfer16(0x91 /* X */) >> 3;
		z -= z2;
		if (z >= Z_THRESHOLD) {
			_pspi->transfer16(0x91 /* X */);  // dummy X measure, 1st is always noisy
			data[0] = _pspi->transfer16(0xD1 /* Y */) >> 3;
			data[1] = _pspi->transfer16(0x91 /* X */) >> 3; // make 3 x-y measurements
			data[2] = _pspi->transfer16(0xD1 /* Y */) >> 3;
			data[3] = _pspi->transfer16(0x91 /* X */) >> 3;
		}
		else data[0] = data[1] = data[2] = data[3] = 0;	// Compiler warns these values may be used unset on early exit.
		data[4] = _pspi->transfer16(0xD0 /* Y */) >> 3;	// Last Y touch power down
		data[5] = _pspi->transfer16(0) >> 3;
		digitalWrite(csPin, HIGH);
		_pspi->endTransaction();
	}	
#if defined(_FLEXIO_SPI_H_)
	else if (_pflexspi) {
		_pflexspi->beginTransaction(FLEXSPI_SETTING);
		digitalWrite(csPin, LOW);
		_pflexspi->transfer(0xB1 /* Z1 */);
		int16_t z1 = _pflexspi->transfer16(0xC1 /* Z2 */) >> 3;
		z = z1 + 4095;
		int16_t z2 = _pflexspi->transfer16(0x91 /* X */) >> 3;
		z -= z2;
		if (z >= Z_THRESHOLD) {
			_pflexspi->transfer16(0x91 /* X */);  // dummy X measure, 1st is always noisy
			data[0] = _pflexspi->transfer16(0xD1 /* Y */) >> 3;
			data[1] = _pflexspi->transfer16(0x91 /* X */) >> 3; // make 3 x-y measurements
			data[2] = _pflexspi->transfer16(0xD1 /* Y */) >> 3;
			data[3] = _pflexspi->transfer16(0x91 /* X */) >> 3;
		}
		else data[0] = data[1] = data[2] = data[3] = 0;	// Compiler warns these values may be used unset on early exit.
		data[4] = _pflexspi->transfer16(0xD0 /* Y */) >> 3;	// Last Y touch power down
		data[5] = _pflexspi->transfer16(0) >> 3;
		digitalWrite(csPin, HIGH);
		_pflexspi->endTransaction();

	}
#endif
	// If we do not have either _pspi or _pflexspi than bail. 
	else return;

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
		switch (rotation) {
		  case 0:
			xraw = 4095 - y;
			yraw = x;
			break;
		  case 1:
			xraw = x;
			yraw = y;
			break;
		  case 2:
			xraw = y;
			yraw = 4095 - x;
			break;
		  default: // 3
			xraw = 4095 - x;
			yraw = 4095 - y;
		}
	}
}

int16_t XPT2046_Touchscreen::updateADC(int16_t adc)
{
    int16_t adc_address[4] = {0xA7, 0xE7, 0x87, 0xF7};
    
    int16_t data[2];
    
    if (_pspi) {
		_pspi->beginTransaction(SPI_SETTING);
		digitalWrite(csPin, LOW);
        
        _pspi->transfer16(adc_address[adc]);  // dummy measure, 1st is always noisy
        data[0] = _pspi->transfer16(adc_address[adc]) >> 3;
        if (adc == ADC_TEMP) {
            _pspi->transfer16(adc_address[adc+1]);  // dummy measure, 1st is always noisy
            data[1] = _pspi->transfer16(adc_address[adc+1]) >> 3;
            
            data[0] = data[1] - data[0];
        }
        
        _pspi->transfer16(adc_address[adc]-7);	// dummy measure power down
        _pspi->transfer16(0);

        digitalWrite(csPin, HIGH);
        _pspi->endTransaction();
    }
#if defined(_FLEXIO_SPI_H_)
	else if (_pflexspi) {
        _pflexspi->beginTransaction(FLEXSPI_SETTING);
		digitalWrite(csPin, LOW);
        
        _pflexspi->transfer16(adc_address[adc]);  // dummy measure, 1st is always noisy
        data[0] = _pflexspi->transfer16(adc_address[adc]) >> 3;
        if (adc == ADC_TEMP) {
            _pflexspi->transfer16(adc_address[adc+1]);  // dummy measure, 1st is always noisy
            data[1] = _pflexspi->transfer16(adc_address[adc+1]) >> 3;
            
            data[0] = data[1] - data[0]; //Calclate differential between both readings
        }
        
        _pflexspi->transfer16(adc_address[adc]-7);	// dummy measure power down
        _pflexspi->transfer16(0);
        
        digitalWrite(csPin, HIGH);
		_pflexspi->endTransaction();
    }
#endif
    
    return data[0];
}

float XPT2046_Touchscreen::getVBat()
{
    int16_t data = updateADC(ADC_VBAT);
    
    float vbat  = ((data*0.0031) + (-0.3799));
    if (vbat <= 0.135) vbat = 0.0; //0.125V is the minimum
    
    return vbat;
}

float XPT2046_Touchscreen::getAuxIn()
{
    int16_t data = updateADC(ADC_AUXIN);
    
    float auxin  = ((data*0.0031) + (-0.3799));
    if (auxin <= 0.135) auxin = 0.0; //0.125V is the minimum
    
    return auxin;
}

float XPT2046_Touchscreen::getTemp()
{
    int16_t data = updateADC(ADC_TEMP);
    
    //Calculate temperature using differential temp0 vs temp1
    float N = 91; // Current ratio in diode = 91. 
    float lnN = log(N); //ln(N) = 4.51086
    float k = 1.38054e-23;  // Boltzmann’s constant (1.38054 • 10−23 electron volts/Kelvin)
    float q = 1.602189e-19; // Electron charge (1.602189 • 10–19 C). 
    float vref = 2.5; //Internal VREF used for temperature
    float dV = float(data) * (vref/4096.0F) * 1000.0F;
    float constants = q / (k * lnN) / 1000.0F; //2.573
    float C = (constants*dV);   //Resolution of 1.6°C per LSB (per bit)
    
    return C;
}

float XPT2046_Touchscreen::getTempF()
{
    float celsius = getTemp();
    return (((9.0/5.0) * celsius) + 32);    //Resolution of 2.88°F per LSB (per bit)
}
