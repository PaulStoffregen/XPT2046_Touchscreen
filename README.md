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

The Z coordinate represents the amount of pressure applied to the screen.

## Calibrated coordinates

The library can provide calibrated coordinates instead of the raw touch screen coordinates.
With calibrated coordinates, the touch screen coordinates will match the tft display coordinates.
If you would touch a pixel drawn at x=30 and y=120,
the coordinates returned by the touch screen library will also be x=30 and y=120.

The calibration settings can be obtained by first running the program 'TFT_Touch_Calibrate'
in the TouchCalibrate folder of this library.
TFT_Touch_Calibrate is an adapted version of the program 'TFT_Touch_Calibrate_v2' which is
provided in the Bodmer TFT_ILI9341 library (https://github.com/Bodmer/TFT_ILI9341).
Before running this program, make sure that the program called 'ILI9341Test' is working correctly.
If 'ILI9341Test' is not working correctly, the calibration program will also not work correctly.
The TFT_Touch_Calibrate program uses the Bodmer TFT_ILI9341 library (https://github.com/Bodmer/TFT_ILI9341).
Other tft libraries can also be used by changing the include, constructor and potentially
adding some defines for that specific tft library.
The Bodmer TFT_ILI9341 library uses the Arduino hardware SPI interface and the settings
differ for each board.
The settings are located in a file called 'User_Setup.h' that is in the Bodmer TFT_ILI9341
library folder.
For using the touch screen you need to specify to which pin the CS signal of the touch panel
has been connected.
Just like all the other sample programs, the 'TFT_Touch_Calibrate' contains a define
called 'CS_PIN' which is the pin number to which the CS of the touchpanel (not the CS of the TFT panel) is connected.

The program connects to the Serial Monitor with 9600 baud.
Follow the instructions on the tft display.
After touching all the red crosshairs on the screen, the program will report the calibration settings.

      //This is the calibration line to use in your sketch
      //you can copy and paste into your sketch setup()
      touch.setCalibration(3941, 404, 3845, 165, 320, 240, 0);

When using the XPT2046_Touchscreen library in your own project,
just copy this line of code to your own project.
Add line before you call touch.begin().
Calibration settings can be updated runtime but if you have fixed settings,
The calibration program will use the calculated settings and allow you to test whether those settings are working correctly.
If you touch the screen, it will draw a green crosshair on that given location.
For accuracy, use a pen, toothpick or any other pointed that does not damage the touchpanel.
If you send any character with the Serial monitor to the running program,
it will change the orientation of the screen and allow you to test the orientation.
After showing all 4 orientations, the program returns to the calibration screen again.

After you initialize the XPT2046_Touchscreen object in your program with your calibration settings,
you can get the calibrated x and y coordinates with the following commands :

      TS_Point touchPoint = touch.getPoint();
      int X_Coord = touchPoint.calx;
      int Y_Coord = touchPoint.caly;

If you called 'touched()' in advance, you can skip the coordinates being retrieved once more by setting the parameter to false :

      touch.getPoint(false);

The calibrates coordinates also support the rotation feature.
The rotation parameter is the same as the Adafruit's TFT library and the Bodmer / TFT_ILI9341 library.
When changing the tft display orientation, you can set the touch screen orientation accordingly.

For example :

      // Set the TFT screen to landscape orientation
      tft.setRotation(1);
      // Set the touch screen to landscape orientation
      touch.setRotation(1);

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
