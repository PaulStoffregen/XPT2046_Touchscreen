/*
This is a sketch to calibrate the screem and list the values to use in the
setCal() function.

The calibration report is sent to the Serial port at 38400 baud.

Designed for use with the TFT_ILI9341 library
https://github.com/Bodmer/TFT_ILI9341

Uses font 2 only.

Created by Rowboteer for the ILI9341 3.4" 320 x 240 TFT touch display: 22/11/15
*/

// TFT Screen pixel resolution in landscape orientation, change these to suit your display
// Defined in landscape orientation !
#define HRES 320
#define VRES 240

// Call up the TFT driver library
#include <TFT_ILI9341.h> // Hardware specific TFT Library for example :  https://github.com/Bodmer/TFT_ILI9341
#include <SPI.h>

#include <XPT2046_Touchscreen.h>
#define CS_PIN  43


// Invoke custom TFT driver library
TFT_ILI9341 tft = TFT_ILI9341();       // Invoke custom library

// These are the pins I used to interface between the 2046 touch controller and Arduino Mega
// they can be changed to other digital pins

/* Create an instance of the touch screen library */
XPT2046_Touchscreen touch = XPT2046_Touchscreen(CS_PIN);

int X_Raw = 0, Y_Raw = 0;


void setup()
{
  // Messages are sent to the serial port
  Serial.begin(9600);

  // Initialise the TFT
  tft.init();

  // Set the TFT screen to landscape orientation
  tft.setRotation(1);

  tft.setTextDatum(TC_DATUM);  // Set text plotting reference datum to Top Centre (TC)
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text to white on black

  touch.begin();
  touch.setRotation(1);
}

/* Main program */
void loop()
{
  int x1, y1;
  int x2, y2;
  int x3, y3;
  bool xyswap  = 0, xflip = 0, yflip = 0;

  Serial.println("TFT_Touch Calibration, follow TFT screen prompts..");
  
  // Reset the calibration values
  touch.setCalibration(0, 4095, 0, 4095, 320, 240, 0);
  
  // Set TFT the screen to landscape orientation
  tft.setRotation(1);
  
  // Set Touch the screen to the same landscape orientation
  touch.setRotation(1);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Show the screen prompt
  drawPrompt();

  drawCross(30, 30, TFT_RED);
  while (!touch.touched());
  delay(100);
  
  getCoord(); // This function assigns values to X_Raw and Y_Raw

  drawCross(30, 30, TFT_BLACK);
  x1 = X_Raw;
  y1 = Y_Raw;

  drawCross(HRES/2, VRES/2, TFT_RED);
  delay(10);
  
  while (getCoord()); // This waits for the centre area to be touched
  
  drawCross(HRES/2, VRES/2, TFT_BLACK);
  drawCross(HRES-30, VRES-30, TFT_RED);
  
  while (!getCoord()); // This waits until the centre area is no longer pressed
  delay(10);           // Wait a little for touch bounces to stop after release
  
  getCoord();
  drawCross(HRES-30, VRES-30, TFT_BLACK);

  x2 = X_Raw;
  y2 = Y_Raw;

  drawCross(HRES/2, VRES/2, TFT_RED);
  delay(10);
  
  while (getCoord()); // This waits for the centre area to be touched
  
  drawCross(HRES/2, VRES/2, TFT_BLACK);
  drawCross(30, VRES-30, TFT_RED);
  
  while (!getCoord()); // This waits until the centre area is no longer pressed
  delay(10);           // Wait a little for touch bounces to stop after release
  
  getCoord();
  drawCross(30, VRES-30, TFT_BLACK);

  x3 = X_Raw;
  y3 = Y_Raw;

  int temp;
  if (abs(x1 - x3) > 1000) {
    xyswap = 1;
    temp = x1; x1 = y1; y1 = temp;
    temp = x2; x2 = y2; y2 = temp;
    temp = x3; x3 = y3; y3 = temp;
  }
  else 
  {
    xyswap = 0;
  }

  // adjust the coordinates with taking into account that the crosshair was placed 30 pixels from the border
  uint16_t hmin = touch.remap(x1,x2,HRES,30,1);
  uint16_t hmax = touch.remap(x1,x2,HRES,30,2);
  uint16_t vmin = touch.remap(y1,y2,VRES,30,1);
  uint16_t vmax = touch.remap(y1,y2,VRES,30,2);

  Serial.println();
  Serial.println("  //This is the calibration line to use in your sketch");
  Serial.println("  //you can copy and paste into your sketch setup()");
  Serial.print("  touch.setCalibration(");
  Serial.print(hmin); Serial.print(", ");
  Serial.print(hmax); Serial.print(", ");
  Serial.print(vmin); Serial.print(", ");
  Serial.print(vmax); Serial.print(", ");
  Serial.print(HRES); Serial.print(", ");
  Serial.print(VRES); Serial.print(", ");
  Serial.print(xyswap); 
  Serial.println(");");

  Serial.println();
  Serial.println("Test the touch screen, green crosses appear at the touch coordinates!");
  Serial.println("Send any character from the serial monitor window to restart calibration");

  // These are the calibration settings the sketch has calculated to try out!
  touch.setCalibration(hmin, hmax, vmin, vmax, HRES, VRES, xyswap);

//  touch.setCalibration(3915, 387, 3806, 227, 320, 240, 0);
  
  // Keep TFT and Touch rotations the same, try values 0 to 3
  // Start with the current orientation
  // Receiving any serial character moves on to next orientation test
  tft.setRotation(1);
  touch.setRotation(1);
  test();

  tft.setRotation(2);
  touch.setRotation(2);
  test();

  tft.setRotation(3);
  touch.setRotation(3);
  test();

  tft.setRotation(0);
  touch.setRotation(0);
  test();

  Serial.println();
}

void test(void)
{
  tft.fillScreen(TFT_BLACK);

  drawCross(30, 30, TFT_WHITE);

  drawCross(tft.width() - 30, tft.height() - 30, TFT_WHITE);

  int centre = tft.width()/2; // Get and work out x coord of screen centre

  String text;
  text+= "Screen rotation = ";
  text+= tft.getRotation();
  char buffer[30];
  text.toCharArray(buffer,30);
  
  tft.drawString(buffer, centre, 80, 2);

  tft.drawString("Touch anywhere on screen", centre, 100, 2);
  tft.drawString("to test settings", centre, 120, 2);

  tft.drawString("Send a character from the", centre, 140, 2);
  tft.drawString("IDE Serial Monitor to", centre, 160, 2);
  tft.drawString("continue!", centre, 180, 2);

  while (Serial.available()) Serial.read(); // Empty the serial buffer before we start
  
  while (!Serial.available()) 
  {
    if (touch.touched()) // Note this function updates coordinates stored within library variables
    {
      TS_Point touchPoint = touch.getPoint(false); // use 'false' to skip updating the coordinates once more
      // retrieve calibrated XY in the same dimensions as the TFT display
      int X_Coord = touchPoint.calx;
      int Y_Coord = touchPoint.caly;

      drawCross(X_Coord, Y_Coord, TFT_GREEN);

      //delay(20);
      tft.setCursor(centre, 0,  2);
      tft.print("X = ");tft.print(X_Coord);tft.print("   ");
      tft.setCursor(centre, 20,  2);
      tft.print("Y = ");tft.print(Y_Coord);tft.print("   ");
      tft.setCursor(centre, 40,  2);
      tft.print("Xraw = ");tft.print(touchPoint.x);tft.print("   ");
      tft.setCursor(centre, 60,  2);
      tft.print("Yraw = ");tft.print(touchPoint.y);tft.print("   ");
    }
  }
}

void drawPrompt(void)
{
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int centre = tft.width()/2; // Get and work out x coord of screen centre

  tft.drawString("CALIBRATION", centre, 20, 2);

  tft.drawString("Touch the red cross accurately", centre, 61, 2);
  tft.drawString("( using a cocktail stick works well! )", centre, 81, 2);
  tft.drawString("( Settings will be sent to the serial monitor. )", centre, 101, 2);
}

void drawCross(int x, int y, unsigned int color)
{
  tft.drawLine(x - 5, y, x + 5, y, color);
  tft.drawLine(x, y - 5, x, y + 5, color);
}

bool getCoord()
{
  bool Xwait = 1, Ywait = 1;
  int X_Temp1 = 9999, Y_Temp1 = 9999;
  int X_Temp2 = -1, Y_Temp2 = -1;
  X_Raw = -1;
  Y_Raw = -1;

  while (Xwait || Ywait) 
  {
    if (touch.touched()) // Note this function updates coordinates stored within library variables
    {
      /* Read the current X and Y axis as co-ordinates at the last touch time*/
      TS_Point tp = touch.getPoint(false); //// use 'false' to skip updating the coordinates once more
      X_Temp1 = tp.x;
      Y_Temp1 = tp.y;
    }
    delay(5);
    if (touch.touched()) // Note this function updates coordinates stored within library variables
    {
      /* Read the current X and Y axis as co-ordinates at the last touch time*/
      TS_Point tp = touch.getPoint(false); // use 'false' to skip updating the coordinates once more
      X_Temp2 = tp.x;
      Y_Temp2 = tp.y;
    }

    #define RAW_ERROR 10
    
    if ( (abs(X_Temp1 - X_Temp2) < RAW_ERROR) && Xwait ) {
      X_Raw = (X_Temp1 + X_Temp2) / 2;
      Xwait = 0;
    }
    if ( (abs(Y_Temp1 - Y_Temp2) < RAW_ERROR) && Ywait ) {
      Y_Raw = (Y_Temp1 + Y_Temp2) / 2;
      Ywait = 0;
    }
  }

  // Check if press is near middle third of screen
  if ((X_Raw > 1365) && (X_Raw < 2731) && (Y_Raw > 1365) && (Y_Raw < 2371)) 
  {
    // near middle third of screen
    return 0;
  }
  else
  {
    // otherwise it is near edge for calibration points
    return 1;
  }
}
