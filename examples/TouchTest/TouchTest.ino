#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CS_PIN  8
// MOSI=11, MISO=12, SCK=13

XPT2046_Touchscreen ts(CS_PIN);

void setup() {
  Serial.begin(38400);
  ts.begin();
  while (!Serial && (millis() <= 1000));
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
