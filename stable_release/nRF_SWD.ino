#pragma once
//#include "swd.h"
#include "nrf_interface.h"
#include "wireless_firmware.h"
#include "defines.h"

#define URL "http://192.168.177.143:8000/zephyr.bin"
char*  SSID  ="ESP123";
char* PASSWORD = "ESP321321";

//SWD swd(21, 19, 5);
NRF_IF swd_if(21, 19, 5, true);
WL_FIR wl_fir(SSID, PASSWORD, URL);


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
  swd_if.ctrl_reset();
  swd_if.core_unhalt();
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
