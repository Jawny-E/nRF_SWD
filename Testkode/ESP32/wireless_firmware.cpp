#include "WiFiType.h"
#include "Arduino.h"
#include "wireless_firmware.h"
#include "defines.h"

#include <LittleFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

WL_FIR::WL_FIR(const char* ssid, const char* password, const char* url){
  _ssid = ssid;
  _password = password;
  _url = url;
}

void WL_FIR::wireless_connect(){
  Serial.begin(9600);
  
  Serial.println("Try Connecting to ");
  Serial.println(_ssid);
  WiFi.begin(_ssid, _password); // Connect to your wi-fi modem
 
  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32-IP on serialmonitor

}

bool WL_FIR::download_firmware(){
  HTTPClient http;
  WiFiClient client;
  File f;
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);
  Serial.print(F("Connecting"));

  // Wait for the Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print('.');
  }
  Serial.println(F("Connected!"));
  delay(500);
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
    return 0;
    }
  // check if test file exists and delete it if needed
  if(LittleFS.exists("/test.bin"))
    LittleFS.remove("/test.bin");
  if(LittleFS.exists(standard_filename))
    LittleFS.remove(standard_filename);

  // try to download a file
  int httpCode;
  int error;
  Serial.print("Downloading: ");
  Serial.println(_url);
  f = LittleFS.open(standard_filename, "w");
  if (f)
  {
    http.begin(client, _url);
    httpCode = http.GET();
    switch(httpCode)
    {
      case HTTP_CODE_OK:
        error = http.writeToStream(&f);
        Serial.printf("Error code: %i\r\n", error);
        break;

      default:
        Serial.println(F("[HTTP] GET failed, error: "));
        Serial.printf("%i - %s\r\n", httpCode, http.errorToString(httpCode).c_str());
        Serial.println(_url);
        break;
    }
    f.close();
  }
  http.end();
  

  // output file content
  Serial.println("Reading file content");
  Serial.println("---");
  f = LittleFS.open(standard_filename, "r");
  String content = f.readString();
  //Serial.println(content);
  Serial.println("Size of file is:");
  size_t filesize = f.size();
  f.close();
  Serial.println(filesize);
  Serial.println(F("---"));
  WiFi.mode(WIFI_OFF);
  return 1;
}