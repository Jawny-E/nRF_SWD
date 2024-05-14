#pragma once
#include <Arduino.h>
#include "wireless_interface.h"
#include "nrf_interface.h"
#include <LittleFS.h>
#include <FS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

const char* topic1 = "command"; // Commandlist : FW, FR, DF, UF*, ME, ER
const char* topic2 = "length"; //Updates span
const char* topic3 = "offset"; //Updates offset
const char* topic4 = "url"; //Updates _your_url through tempStorage, you can apply similar methods for ssid, password, broker-ip etc.

enum taskStatus{
  IDLE, 
  FLASH_WR, 
  FLASH_RD, 
  FIRMWARE_DOWNLOAD, 
  FIRMWARE_UPLOAD,
  DM_READ,
  ERASEALL,
  UPDATE_URL
};

volatile taskStatus programState = IDLE; // enum to control switch-case
volatile uint8_t taskCompletion = 0;  // Number sent in ACK response: 0-No task done, 1-Task successful, 2+- Some fault has occured in task
uint32_t span = 0; // Stores the value used when reading a spesific amount of registers
uint32_t offset = 0; //Offset used to 
String tempStorage; //tempStorage used for String handling from callback

WiFiClient esp32Client; 
PubSubClient mqttClient(esp32Client);

/*
 * Initializes class and local variables
 */
WL_IF::WL_IF(const char* WIFI_SSID, const char* WIFI_PASSWORD, const char* MQTT_SERVER_IP, const char* MQTT_USER, const char* MQTT_PASSWORD,const char* MQTT_CLIENT_NAME, const char* YOUR_FILENAME, String YOUR_URL, int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug):_nrf(SWDCLK, SWDIO, SWD_CLK_HALFPERIOD, false){
    _wifi_ssid = WIFI_SSID;
    _wifi_password = WIFI_PASSWORD;
    _mqtt_server_ip = MQTT_SERVER_IP;
    _mqtt_user= MQTT_USER;
    _mqtt_password = MQTT_PASSWORD;
    _mqtt_client_name = MQTT_CLIENT_NAME;
    _your_filename = YOUR_FILENAME;
    _your_url = YOUR_URL;
    _callbackOn = true;
    pinMode(2, OUTPUT);
}

/*
 * Sets up the interface
 * Starts wifi and mqtt connection
 */
void WL_IF::interface_init(){
  digitalWrite(2, HIGH);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_wifi_ssid, _wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  mqttClient.setServer(_mqtt_server_ip, 1883);
  mqttClient.setCallback(callback);
  digitalWrite(2, LOW);
}

/*
 * This function keeps up the mqtt and wifi connection
 * and handles the states of the program
 * needs to be called continuously, preferably within void-loop
 */
void WL_IF::interface_run(){
  if(WiFi.status() != WL_CONNECTED){
    digitalWrite(2, HIGH);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      delay(500);
    }
  }
  if(!mqttClient.connected()){
    while(!mqttClient.connected()){
      if(mqttClient.connect(_mqtt_client_name)){
        mqttClient.subscribe(topic1);
        mqttClient.subscribe(topic2);
        mqttClient.subscribe(topic3);
        mqttClient.subscribe(topic4);
      }
      else{
        digitalWrite(2, HIGH);
        delay(2000);
      }
    }
  }
  digitalWrite(2, LOW);
  mqttClient.loop();

  switch(programState){
    case IDLE:
      set_callback();
      if(taskCompletion != 0){
        if(taskCompletion == 1){
          mqttClient.publish("ack", "Task Completed");
        }
        else{
          mqttClient.publish("ack", "Task fail error: ");
          char errorString[8];
          dtostrf(taskCompletion, 1, 2, errorString);
          mqttClient.publish("ack", errorString);
        }
        taskCompletion = 0;
      }
      break;
    case FLASH_WR:
      clear_callback();
      _nrf.begin_if();
      if(_nrf.flash_write_from_file(_your_filename, offset)){
        taskCompletion = 1;
      }
      else{
        taskCompletion = 2;
      }
      _nrf.end_if();
      programState = IDLE;
      break;
    case FLASH_RD:
      clear_callback();
      _nrf.begin_if();
      if(_nrf.flash_read_to_file(_your_filename, span, offset)){
        taskCompletion = 1;
      }
      else{
        taskCompletion = 0;
      }
      _nrf.end_if();
      programState = IDLE;
      break;
    case FIRMWARE_DOWNLOAD:
      clear_callback();
      firmware_download(_your_url, _your_filename); 
      programState = IDLE;
      break;
    case FIRMWARE_UPLOAD: //Unused due to server setup faults 
      //clear_callback();
      //firmware_upload(_your_url, _your_filename);
      Serial.println("Upload firmware coming later!");
      programState = IDLE;
      break;
    case DM_READ:
      {
        uint32_t temp[span] = {0};
        _nrf.measurement(temp, offset, span);
        for(int i = 0; i < span; i++){
          char tempString[8];
          dtostrf(temp[i], 1, 2, tempString);
          mqttClient.publish("data", tempString);
          mqttClient.loop();
        }
        programState = IDLE;
        break;
      }
    case ERASEALL:
      clear_callback();
      _nrf.begin_if();
      _nrf.ctrl_erase_all(); //Has no fail condition
      _nrf.end_if();
      programState = IDLE;
      break;
    case UPDATE_URL:
      clear_callback();
      _your_url = tempStorage;
      taskCompletion = 1;
      programState = IDLE;
      break;
  }
}

/*
 * Downloads firmware from a given url and saves it locally to a given filename
 * there is no method for changing this filename in case of memory problems, and users are adviced
 * to be careful when implementing one
 * @httpUrl: URL firmware is downloaded from
 * @filename: local filename for storage with LittleFS
 */
void WL_IF::firmware_download(String httpUrl, String filename){
  File f;
  HTTPClient http;
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
  }
  delay(50);
  if(!LittleFS.begin(true)){
    taskCompletion = 2; // Error in filesystem
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
        taskCompletion = 1; //Successful download
        break;
      default:
        taskCompletion = 3; //http error
        break;
    }
    f.close();
  }
  http.end();
}

/*
//Untested, due to python server not accepting POST
//This funciton is for now outside of the scope of the project
void WL_IF::firmware_upload(String url, String filename){
  File f;
  HTTPClient http;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print('.');
  }
  delay(500);
  if(!LittleFS.begin()){
    Serial.println("LittleFS Mount Failed");
    taskCompletion = 2;
    return;
  }
  if (!LittleFS.exists(_your_filename)) {
    Serial.println("File not found");
    taskCompletion = 3;
    return;
  }

  f = LittleFS.open(_your_filename, "r");
  if (!f) {
    Serial.println("Failed to open file for reading");
    taskCompletion = 4;
    return;
  }

  http.begin(esp32Client, url); // Start the HTTP connection
  http.addHeader("Content-Type", "application/octet-stream"); // Set the content type to binary data
  int httpCode;

  size_t fileSize = f.size();
  http.addHeader("Content-Length", String(fileSize));

  httpCode = http.sendRequest("POST", &f, fileSize);
  if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Upload successful");
      } else {
        Serial.print("Upload failed, error: ");
        Serial.println(http.errorToString(httpCode).c_str());
      }
  } else {
      Serial.print("HTTP error: ");
      Serial.println(http.errorToString(httpCode).c_str());
  }

  f.close();
  http.end();
}
*/

/*
 * Turns on the callback function 
 */
void WL_IF::set_callback(){
  if(!_callbackOn){
    mqttClient.setCallback(callback);
    _callbackOn = true;
  }
}
/*
 * Turns off callback function
 * used to avoid interrupts during sensitive tasks 
 * such as writing to flash
 */
void WL_IF::clear_callback(){
  if(_callbackOn){
    mqttClient.setCallback(NULL);
    _callbackOn = false;
  }
}

/*
 * Callback function used with MQTT
 * changes variables depending on topic and mesage 
 */
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if(String(topic) == topic1){
    if(messageTemp == "FW"){
      programState = FLASH_WR;
    }
    else if(messageTemp == "FR"){
      programState = FLASH_RD;
    }
    else if(messageTemp == "DF"){
      programState = FIRMWARE_DOWNLOAD;
    }
    else if(messageTemp == "UF"){
      programState = FIRMWARE_UPLOAD;
    }
    else if(messageTemp == "ME"){
      programState = DM_READ;
    }
    else if(messageTemp == "ER"){
      programState = ERASEALL;
    }
  }
  else if(String(topic) == topic2){
    span = messageTemp.toInt(); 
    char tempString[8];
    dtostrf(span, 7, 0, tempString);
    mqttClient.publish("ack", "Span updated");
    taskCompletion = 1;
  }
  else if(String(topic) == topic3){
    offset = messageTemp.toInt();
    char tempString[8];
    dtostrf(offset, 1, 2, tempString);
    mqttClient.publish("ack", "Offset updated");
    taskCompletion = 1;
  }
  else if(String(topic) == topic4){
    tempStorage = messageTemp;
    programState = UPDATE_URL;
  }
}