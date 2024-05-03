#pragma once
#include "nrf_interface.h"
#include "wireless_firmware.h"
#include "defines.h"
#include <LittleFS.h>
NRF_IF swd_if(21, 19, 5, false);

void setup(){
  Serial.begin(115200);
}

void loop() {
  Serial.println("--------------------");
  Serial.println("Flash downloaded firmware");
  download_firmware(YOUR_URL, STANDARD_FILENAME);
  
  swd_if.begin_if();
  swd_if.flash_write_from_file(STANDARD_FILENAME, 0x00);
  swd_if.end_if();
  
  Serial.println("Completed flashing and reading");

  delay(5000);
  Serial.println("Reflash with firmware from nRF");
  
  swd_if.begin_if();
  swd_if.flash_read_to_file(STANDARD_FILENAME, 24000, 0x00);
  swd_if.ctrl_erase_all();
  swd_if.flash_write_from_file(STANDARD_FILENAME, 0x00);
  swd_if.end_if();

  Serial.println("Check your nRF!");
  delay(10000);
}
