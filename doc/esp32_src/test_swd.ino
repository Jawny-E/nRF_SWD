#pragma once
#include "swd.h"

#define pin_SWDIO 19
#define pin_SWDCLK 21
#define clockperiod 5
#define debug true

SWD_IF swd(pin_SWDCLK, pin_SWDIO, clockperiod, debug);

void setup(){
    Serial.begin(115200);
}

void loop(){
    uint32_t temp = 0;
    temp = swd.begin();
    Serial.println(temp); //This should read 0x2ba01477 for an nRF52
    delay(2000);
}

