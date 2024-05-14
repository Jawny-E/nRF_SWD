/*
 * A proof-of-concept wireless interface layer that uses
 * MQTT to control the ESP32 and nRF. It uses an instance of NRF_IF
 * to be able to read and write to the memory of the nRF. The ESP32 will
 * turn on it's inbuilt LED if there's connection issues
 * Parametres:
  @WIFI_SSID: SSID for your WiFi network
  @WIFI_PASSWORD: Password for your WiFi network
  @MQTT_SERVER_IP: IP of the MQTT broker
  @MQTT_USER: Unused, but can be used to set up MQTT autharization and targeted messages
  @MQTT_PASSWORD: Unused, but can be used to set up MQTT autharization and targeted messages
  @MQTT_CLIENT_NAME: Unique name for this ESP32-device 
  @YOUR_FILENAME: Local filename for storage, recommended to not change this!
  @YOUR_URL: URL used for downloading firmware
  @SWDCLK: Pin for the SWDCLK connection
  @SWDIO: Pin for the SWDIO connection
  @SWD_CLK_HALFPERIOD: Half a clock period in microseconds, 5 is recommended
  @debug: Turns on debug messages for *all* layers, recommended to keep this off or manually adjust per layer
 */

#pragma once
#include "nrf_interface.h"

class WL_IF{
  public:
    WL_IF(const char* WIFI_SSID, 
      const char* WIFI_PASSWORD, 
      const char* MQTT_SERVER_IP, 
      const char* MQTT_USER, 
      const char* MQTT_PASSWORD, 
      const char* MQTT_CLIENT_NAME, 
      const char* YOUR_FILENAME,
      String YOUR_URL,
      int SWDCLK, 
      int SWDIO, 
      int SWD_CLK_HALFPERIOD, 
      bool debug);

    void interface_init(); //Run this in void-setup, sets up WiFi and MQTT
    void interface_run();  //Run this in void-loop, handles program states and upkeep

  private:
    void firmware_download(String url, String filename); //Downloads firmware from a given url
    void firmware_upload(String url, String filename);  //Untested/Unused due to server setup
    void set_callback(); //Turns callback interrupt on
    void clear_callback(); // Turns callback interrupt off
    const char* _wifi_ssid;
    const char* _wifi_password;
    const char* _mqtt_server_ip;
    const char* _mqtt_user;
    const char* _mqtt_password;
    const char* _mqtt_client_name;
    const char* _your_filename;
    String _your_url;
    bool _callbackOn;
    NRF_IF _nrf;
};

void callback(char* topic, byte* message, unsigned int length); //Handles incoming MQTT messages