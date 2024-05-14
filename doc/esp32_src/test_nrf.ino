#pragma once
#include "nrf_interface.h"

#define pin_SWDIO 19
#define pin_SWDCLK 21
#define clockperiod 5
#define debug true

NRF_IF nrf(pin_SWDCLK, pin_SWDIO, clockperiod, debug);

void setup(){
    Serial.begin(115200);
    firmware_download("URL to blinky .bin file", "/firmware.bin");
}

void loop(){
    nrf.begin_if();
    nrf.flash_write_from_file("/firmware.bin", 0);
    nrf.end_if();
    Serial.println("Blinky is running");
    delay(5000);
    nrf.begin_if();
    nrf.ctrl_erase_all();
    nrf.end_if();
    Serial.println("nRF is erased");
    delay(5000);
}


/*
 * Stealing the firmware download from the wireless interface
 */
void firmware_download(String httpUrl, String filename){
  File f;
  HTTPClient http;
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
  }
  delay(50);
  if(!LittleFS.begin(true)){
    Serial.println("Error LittleFS"); // Error in filesystem
    return;
  }
  if(LittleFS.exists("/test.bin"))
    LittleFS.remove("/test.bin");
  if(LittleFS.exists(filename))
    LittleFS.remove(filename);
  int httpCode;
  int error;
  f = LittleFS.open(filename, "w");
  if (f){ 
    http.begin(esp32Client, httpUrl);
    httpCode = http.GET();
    switch(httpCode)
    { case HTTP_CODE_OK:
        error = http.writeToStream(&f);
        Serial.printf("Size of download: %i\r\n", error);
        Serial.println("Success")<>
        break;
      default:
        Serial.println("ERROR");
        break;
    }
    f.close();
  }
  http.end();
}