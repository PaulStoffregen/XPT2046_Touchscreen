#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CS_PIN  8
// MOSI=11, MISO=12, SCK=13

XPT2046_Touchscreen ts(CS_PIN);

void setup() {
  ts.begin();
}

void loop() {
  TS_Point p = ts.getPoint();
  Serial.print("Pressure = ");
  Serial.print(p.z);
  if (ts.touched()) {
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
  }
  Serial.println();
  delay(100);
}
