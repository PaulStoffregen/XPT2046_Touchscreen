# XPT2046 Touchscreen Arduino Library

XPT2046_Touchscreen is a library for the XPT2046 resistive touchscreen controllers used on many low cost TFT displays.

![ILI9431Test Example Program](doc/ILI9431Test.jpg)

## Setup Functions

First, create an instance of the library for your touchscreen.  The digital pin
used for chip select is required.  The normal MISO, MOSI and SCK pins will be
used automatically.

    #define CS_PIN  8
    XPT2046_Touchscreen ts(CS_PIN);

The use of the Touch interrupt pin can be optionally specified. If the Teensy
pin specified is actively connected to the T_IRQ display pin then the normal
touch calls will respond, but can be called more often as each call returns
without hardware access when no interrupt was recorded.

    #define TIRQ_PIN  2
    XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);

In setup(), use the begin() function to initialize the touchscreen, and
optionally use setRotation(n), where n is 0 to 3, matching the rotation
setting in ILI9341_t3, Adafruit_ILI9341 or other Adafruit compatible TFT
libraries.

      ts.begin();
      ts.setRotation(1);

Optionally you can also use a calibration (you can read more about calibration down below) to match the Pixels on your Screen. In the setup() ether use the calibration saved in the EEPROM:

      ts.begin(displayWidth, displayHeight, getEEPROMCalibration());
      
Or just type in your own calibration data (read down below how to get them):

      ts.begin(displayWidth, displayHeight, TS_Calibration(vi1, vj1, vi2, vj2);

To get the width and heigth of the screen with a Adafruit GFX library compatible screen (or with the ILI9341_t3 library) you can use these commands:

      ILI9341_t3 display = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST);
      ts.begin(display.width(), display.height(), getEEPROMCalibration());
      
## Reading Touch Info

The touched() function tells if the display is currently being touched,
returning true or false.

      if (ts.touched()) {
        // do something....
      }

You can read the touch coordinates with readData()

      uint16_t x, y, z;
      ts.readData(&x, &y, &z);

or with getPoint(), which returns a TS_Point object:

      TS_Point p = ts.getPoint();
      Serial.print("x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);

or with a calibration, to match the pixels on your screen (you have to set a calibration like shown above to use calibration, you also have to calibrate the display like shown down below):

      TS_Point p = ts.getPixel();
      Serial.print("calibrated x = ");
      Serial.print(p.x);
      Serial.print(", calibrated y = ");
      Serial.print(p.y);

The Z coordinate represents the amount of pressure applied to the screen.

## Calibration

To calibrate the Touchscreen, use the example scetch called "Calibration". Its ment to be used with the ILI9341_t3 library by Paul Stoffregen (but you can easily modify it to match your display using the Adafruit GFX library). To calibrate the display, load the scetch on your Arduino or Teensy (Change the pin definitions if they don't match with your setup) and tap the first, than the second + on the Screen. After all, you will see the calibration data in the following order for 3 seconds: vi1, vj1, vi2, vj2. The calibration is automatically saved in the EEPROM that you can use it easier the next time with a different scetch. After it saved the calibration to EEPROM and then after 3 seconds, you can draw something on the screen to test the calibration. If your drawing doesn't really match up with your actual touch-point, just redo the calibration process.

What is EEPROM? The eeprom is a non-volitale storage in the Arduino/Teensy to save small amounts of data even if the Arduino is turned off. So it is great to save one or more calibration data in it to reuse it next time you power up the Arduino, or to use it with a different scetch.

## Adafruit Library Compatibility

XPT2046_Touchscreen is meant to be a compatible with sketches written for Adafruit_STMPE610, offering the same functions, parameters and numerical ranges as Adafruit's library.

## Using The Interrupt Pin : Built in support when connected nothing else is needed. When specified as above
no SPI calls are made unless a Touch was detected.  On normal connections - this means the Teensy LED
won't blink on every touch query.

## Using The Interrupt Pin : Custom use would preclude the normal built in usage. The warning below is justified.

The XPT2046 chip has an interrupt output, which is typically labeled T_IRQ on many low cost TFT displays.  No special software support is needed in this library.  The interrupt pin always outputs a digital signal related to the touch controller signals, which is LOW when the display is touched.  It also is driven low while software reads the touch position.

The interrupt can be used as a wakeup signal, if you put your microcontroller into a deep sleep mode.  Normally, you would stop reading the touch data, then enable the interrupt pin with attachInterrupt(), and then configure your processor to wake when the interrupt occurs, before enter a deep sleep mode.  Upon waking, you would normally disable the interrupt before reading the display, to prevent false interrupts caused by the process of reading touch positions.

You can also use the interrupt to respond to touch events.  Setup might look similar to this:

      SPI.usingInterrupt(digitalPinToInterrupt(pin))
      attachInterrupt(digitalPinToInterrupt(pin), myFunction, FALLING);

However, inside your interrupt function, if the display is no longer being touched, any attempt to read the touch position will cause the interrupt pin to create another falling edge.  This can lead to an infinite loop of falsely triggered interrupts.  Special care is needed to avoid triggering more interrupts on the low signal due to reading the touch position.

For most applications, regularly reading the touch position from the main program is much simpler.

