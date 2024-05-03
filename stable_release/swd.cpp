#pragma once
#include "Arduino.h"
#include "swd.h"
#include "defines.h"

bool stream_state = 0; // This  bool is 0 when writing and 1 while reading

/***************************************************
            Section: Public functions
****************************************************/

SWD::SWD(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug){
  _SWDCLK = SWDCLK;
  _SWDIO = SWDIO;
  _SWD_clk_halfperiod = SWD_CLK_HALFPERIOD;
  _debug = debug;
}

/*
 * Sets up pins for SWD connection
 */
void SWD::init(){
  pinMode(_SWDCLK, OUTPUT);
  pinMode(_SWDIO, OUTPUT);
}

/*
 * Handles startup of SWD 
 * and reads the components ID-register
 */
uint32_t SWD::begin(){
  write(0xffffffff, 32);
  write(0xffffffff, 32);
  write(0xe79e, 16);
  write(0xffffffff, 32);
  write(0xffffffff, 32);
  write(0x00000000, 8);

  uint32_t idcode = 0;
  DP_read(idcode, DP_IDCODE);
  return idcode;
}

/*
 * Handles a single DP-read sequence 
 * @data: variable that will hold read data
 * @address: address for request packet
 * returns false if 16 tries to read fail
 */
bool SWD::DP_read(uint32_t& data, unsigned adress){
  uint8_t retry = 15;
  while(retry--){  
    if(send_request(adress, 0, 1)){
      data = read(32);
      bool temp_par = read(1);
      if(check_parity(data) == temp_par){
        return true;
      }
      else{
        Serial.println("Parity Fault");
        return false;
      }
    }
  }
  if(_debug) Serial.println("DP read not ACKed");
  return false;
}

/*
 * Handles a single AP-read sequence 
 * @data: variable that will hold read data
 * @address: address for request packet
 * returns false if 16 tries to read fail
 */
bool SWD::AP_read(uint32_t &data, unsigned address){
    uint8_t retry = 15;
    while(retry--){  
      if(send_request(address, 1, 1)){
        data = read(32);
        bool temp_par = read(1);
        if(check_parity(data) == temp_par){
          return true;
        }
      }
    }
  if(_debug){
    Serial.println("AP read not ACKed");
  }
  return false;
}

/*
 * This function handles a single DP-write sequence
 * @data: 32 bits of data to be written
 * @address: address to be entered in request packet
 * returns false if all 16 tries fail
 */
bool SWD::DP_write(uint32_t data, unsigned address){
    uint8_t retry = 15;
    while(retry--){  
      if(send_request(address, 0, 0)){
        write(data, 32);
        write(check_parity(data), 1);
        return true;
      }
    }
  if(_debug){
    Serial.println("DP write not ACKed");
  }
  return false;
}

/*
 * This function handles a single AP-write sequence
 * It retries the transfer upto 16 times before failure
 * @data: data to be transferred
 * @adress: adress used in request packet
 * returns false if all 16 tries failed
 */
bool SWD::AP_write(uint32_t data, unsigned address){
    uint8_t retry = 15;
    while(retry--){  
      if(send_request(address, 1, 0)){
        write(data, 32);
        write(check_parity(data), 1);
        return true;
      }
    }
  if(_debug){
    Serial.println("AP write not ACKed");
  }
  return false;
}

/***************************************************
          Section: Private functions
****************************************************/

/*
 * This function directly controls the SWDCLK, and SWDIO lines when writing
 * @output_data: data to be transferred
 * @length: number of bits to be transferred
 */
void SWD::write(uint32_t output_data, uint8_t length){
  if(stream_state == 1) {
    stream_state = trn(0);
  }
  while(length--){
    digitalWrite(_SWDIO, output_data & 1);
    digitalWrite(_SWDCLK, LOW);
    delayMicroseconds(_SWD_clk_halfperiod);
    output_data >>= 1;
    digitalWrite(_SWDCLK, HIGH);
    delayMicroseconds(_SWD_clk_halfperiod);
  }
}

/*
 *This function directly controls the SWDCLK, and SWDIO lines when reading
 *This function reads LSB first as per SWD standard, you may want to do some switcheroos  
 *@length: number of bits to be read
 *returns read data
 */
uint32_t SWD::read(uint8_t lenght){
  if(stream_state == 0) {
    stream_state = trn(1);
  }
  uint32_t temp = 0;
  uint8_t position = 0;
  while(lenght--){
    digitalWrite(_SWDCLK, LOW);
    delayMicroseconds(_SWD_clk_halfperiod);
    bool readval = digitalRead(_SWDIO);
    temp |= readval << position;
    digitalWrite(_SWDCLK, HIGH);
    delayMicroseconds(_SWD_clk_halfperiod);
    position++;
  }
  return temp;
}

/*
 * This function handles the creating of SWD - request and writes it to SWDIO
 * @adrs: two adress bits in package usually defined
 * @APnDP: Set high to access AP, low for DP
 * @RnW: Set high to read, low for write
 * returns false if ACK response is wrong
 */
bool SWD::send_request(unsigned adrs, bool APnDP, bool RnW){
  bool parity = ((adrs >> 2) & 1) ^ ((adrs >> 3) & 1) ^ APnDP  ^ RnW;
  uint8_t request_packet = (1 << 0) | (APnDP << 1) | (RnW << 2) | ((adrs & 0xC) << 1) | (parity << 5) | (1 << 7);
  write(request_packet, 8);
  if(handle_ACK()) return true;
  else return false;
}

/*
 * This function handles a single Trn phase
 * @RnW: Read or Write bit, determines _SWDIO input or output
 * returns the input
 */
bool SWD::trn(bool RnW){
  //Trn
  digitalWrite(_SWDIO, HIGH);
  pinMode(_SWDIO, INPUT_PULLUP);
  digitalWrite(_SWDCLK, LOW);
  delayMicroseconds(_SWD_clk_halfperiod);
  digitalWrite(_SWDCLK, HIGH);
  delayMicroseconds(_SWD_clk_halfperiod);
  if(!RnW){
    pinMode(_SWDIO, OUTPUT);
  }
  return RnW;
}

/*
 * Handles ACK responses, in SWD only 0b001 is OK response
 * returns false if ACK is not OK
 */
bool SWD::handle_ACK(){
  //0b001 == OK
  uint32_t ACK = read(3);
  if (ACK == 0b001){
    return true;
  }
  else{
    return false;
  }
}

/*
 * Function returns 1 if the data has an odd amount of 1s
 * @data_to_check: data of type uint32_t to be examined
 * returns 1 if there is an odd number of 1s
 */
bool SWD::check_parity(uint32_t data_to_check){
  data_to_check ^= (data_to_check >> 16);
  data_to_check ^= (data_to_check >> 8);
  data_to_check ^= (data_to_check >> 4);
  data_to_check ^= (data_to_check >> 2);
  data_to_check ^= (data_to_check >> 1);
  return (data_to_check & 1);
}