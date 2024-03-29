#include "swd.h"
#include "defines.h"

SWD swd(21, 19, SWDCLK_PERIOD/2);


void setup() {
  swd.init();
  Serial.begin(9600);
}

void loop() {
  if(digitalRead(17) == HIGH){
    Serial.println(swd.read(2), BIN);
    Serial.println(swd.read(4), BIN);
    Serial.println(swd.read(8), BIN);
    Serial.println(swd.read(16), BIN);
    Serial.println(swd.read(32), BIN);
  }
  delay(1000);
}
