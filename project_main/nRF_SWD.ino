/*
 * This project is built with three layers of libraries
    wireless_interface.h      /1\
    nrf_interface.h          / 2 \
    swd.h                   /  3  \
 * since they build on each other you only need to include
 * the topmost layer youre intending to use
 */
#pragma once
#include "wireless_interface.h"
//#include "nrf_interface.h"
//#include "swd.h"
#include "defines.h"
#include <LittleFS.h>

/* Feel free to Edit or Replace these variables dependent on your needs: */
#define pin_SWDIO 19
#define pin_SWDCLK 21
#define clockperiod 5
#define debugging false
const char* YOUR_FILENAME     = "/firmware.bin";   //Local storage for ESP32, dont edit this unless you know what youre doing
const char* YOUR_MQTT_BROKER  = "XXX.XXX.XXX.XXX"; //IP for your mqtt broker
const char* YOUR_SSID         = "********";        //SSID for your network
const char* YOUR_PASSWORD     = "********";        // Password for your network
String YOUR_FIRMWARE_URL = "http://XXX.XXX.XXX.XXX:8000/zephyr.bin"; // The basic URL for your firmware, change as needed

/* Example setup of the differnt interfaces */
WL_IF wl(YOUR_SSID,YOUR_PASSWORD, YOUR_MQTT_BROKER, " ", " ", "UniqueName", YOUR_FILENAME, YOUR_FIRMWARE_URL, pin_SWDCLK , pin_SWDIO, clockperiod, debugging);
//NRF_IF nrf(21, 19, 5, 0); //Basic nRF_interface
//SWD_IF swd(21, 19, 5, 0); //Basic SWD_interface

void setup(){
  Serial.begin(115200);
  wl.interface_init(); // Start the Wireless interface
}

void loop() {
  wl.interface_run(); // Run the state machine of the wireless interface
}
