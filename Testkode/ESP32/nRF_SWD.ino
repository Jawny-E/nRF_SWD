#pragma once
#include "swd.h"
#include "nrf_interface.h"
#include "wireless_firmware.h"
#include "defines.h"

#define URL "http://192.168.177.143:8000/zephyr.bin"
char*  SSID  ="ESP123";
char* PASSWORD = "ESP321321";

//SWD swd(21, 19, 5);
NRF_IF swd_if(21, 19, 5, true);
WL_FIR wl_fir(SSID, PASSWORD, URL);

/*Shamelessly stolen - not in use rn, but handy*/
uint32_t reverse(uint32_t b) {
   uint32_t mask = 0b11111111111111110000000000000000;
   b = (b & mask) >> 16 | (b & ~mask) << 16;
   mask = 0b11111111000000001111111100000000;
   b = (b & mask) >> 8 | (b & ~mask) << 8;
   mask = 0b11110000111100001111000011110000;
   b = (b & mask) >> 4 | (b & ~mask) << 4;
   mask = 0b11001100110011001100110011001100;
   b = (b & mask) >> 2 | (b & ~mask) << 2;
   mask = 0b10101010101010101010101010101010;
   b = (b & mask) >> 1 | (b & ~mask) << 1;
   return b;
}

void setup(){
  Serial.begin(115200);
  wl_fir.download_firmware();
  //swd.init();
}


void loop() {
  swd_if.begin_if();
  swd_if.core_halt();
  swd_if.ctrl_erase_all();
  swd_if.read_info();
  swd_if.flash_file(0x00);
  swd_if.core_unhalt();
  swd_if.end_if();
  Serial.println("Check your nRF!");
  delay(20000);

  /*
  Serial.println("-----");
  Serial.println(swd_if.read_register(0x10001208), HEX);
  swd_if.begin_if();
  swd_if.core_halt();
  swd_if.ctrl_erase_all();
  Serial.println(swd_if.read_register(0x10001208), HEX);
  swd_if.ctrl_reset();
  swd_if.end_if();
  Serial.println(swd_if.read_register(0x10001208), HEX);
  delay(10000);
  */
  //swd_if.core_unhalt();
  //delay(5000);
}
