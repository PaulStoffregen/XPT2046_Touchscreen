#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CS_PIN  8
// MOSI=11, MISO=12, SCK=13

//XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN  2
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void setup() {
  Serial.begin(38400);
  ts.begin();
  while (!Serial && (millis() <= 1000));
}

void loop() {
  Serial.println(ts.getVBat());
  Serial.println(ts.getAuxIn());
  Serial.println(ts.getTemp());
  Serial.println(ts.getTempF()); //This value is not taken at the same time, therefor value might not fit the Celsius value
  Serial.println();

  delay(1000);
}
