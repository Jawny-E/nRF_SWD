#pragma once
#include "Arduino.h"
#include "swd.h"
#include "nrf_interface.h"
#include "defines.h"
#include <FS.h>
#include <LittleFS.h>

bool connection_status;
bool nrf_lock_state;
bool task_write_flash = false;
bool task_read_flash = false;
bool task_read_custom_data;
uint32_t flash_offset;
NRF_INFO nrf_ficr;


NRF_IF::NRF_IF(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug) : _swd(SWDCLK, SWDIO, SWD_CLK_HALFPERIOD, 0){ 
  _debug = debug;
}

/***************************************************
            Section: Public functions
****************************************************/

/*
 * Reads relevant FICR info from the nRF device using MEM-AP
 * Base adress 0x10000000
 * Info is stored in global struct nrf_ficr
struct NRF_INFO{
  uint32_t codepage_size;
  uint32_t codesize;
  uint32_t config_id;
  uint32_t device_id0;
  uint32_t device_id1;
  uint32_t info_part;
  uint32_t info_variant;
  uint32_t info_package;
  uint16_t sd_info_area;
  uint32_t ucir_lock;
};
 */
void NRF_IF::read_info(){
  port_sel(0);
  nrf_ficr.codepage_size = read_register(0x10000010);
  nrf_ficr.codesize      = read_register(0x10000014);
  nrf_ficr.flash_size    = nrf_ficr.codepage_size * nrf_ficr.codesize;
  nrf_ficr.device_id0    = read_register(0x10000060);
  nrf_ficr.device_id1    = read_register(0x10000064);
  nrf_ficr.info_part     = read_register(0x10000100);
  nrf_ficr.info_variant  = read_register(0x10000104);
  nrf_ficr.info_package  = read_register(0x10000108);
  nrf_ficr.ucir_lock     = read_register(0x10001208);
  if(_debug){
    Serial.println("Device information:");
    Serial.println(nrf_ficr.core_id, HEX);
    Serial.println(nrf_ficr.flash_size);
    Serial.println(nrf_ficr.device_id0, HEX);
    Serial.println(nrf_ficr.device_id1, HEX);
    Serial.println(nrf_ficr.ucir_lock, HEX);
  }
}

/*
 * Starts up an swd-session, checks the DP-ID register
 * and requests startup of debug and system
 */
void NRF_IF::begin_if(){
  _swd.init();
  nrf_ficr.core_id = _swd.begin();
  //abort();
  if(nrf_ficr.core_id == 0x2ba01477){ //Standard Core-ID for nRF 52 devices
    connection_status = true;
    uint32_t temp;
    _swd.DP_write(0x0, DP_SELECT); //Choose AP0
    _swd.DP_write(0x50000000, DP_CTRL); //Request debug powerup
    _swd.DP_read(temp, DP_CTRL); //Check ack for request
    _swd.AP_write(0x23000052, MEM_AP_CSW);
  }
  else{
    if(_debug){
      Serial.print("Unable to connect to device");
    }
    connection_status = false;
  }
}

/*
 * Ends the interface session by ending the powerup requests
 */
void NRF_IF::end_if(){
  _swd.DP_write(0x0, DP_CTRL); //Clear powerup request
}

/*
 * Halts the CPU
 * More info at:
 * https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/serial-wire-debug-port-interface-for-nrf52832
 */
void NRF_IF::core_halt(){
  _swd.AP_write(0xa2000002, MEM_AP_CSW);
  _swd.AP_write(0xe000edf0, MEM_AP_TAR);
  uint32_t retry = 200;
  while(retry--){
    _swd.AP_write(0xA05F0003, MEM_AP_DRW);
  }
}

/*
 * Unhalts the CPU
 * More info at:
 * https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/serial-wire-debug-port-interface-for-nrf52832
 */
void NRF_IF::core_unhalt(){
  uint32_t temp;
  _swd.AP_write(0xE000EDFC, MEM_AP_TAR);
  _swd.AP_write(0x00000000, MEM_AP_DRW);
  _swd.DP_read(temp, DP_RDBUFF);
  _swd.AP_write(0xE000EDF0, MEM_AP_TAR);
  _swd.AP_write(0xA05F0001, MEM_AP_DRW);
  _swd.DP_read(temp, DP_RDBUFF);
}

/*
 * Uses the Non-Volatile Memory Controller to write to Flash memory
 * @value: Value to be written to the given address
 * @adrs: Address-location of data
 * More info at:
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fps_nrf52833%2Fnvmc.html&cp=5_1_0_3_2&anchor=register.ERASEALL
 */
uint8_t NRF_IF::nvmc_write(uint32_t value, uint32_t adrs){
  write_register(0x4001e504, 1); // Write 1 to CONFIG
  long timeout = millis();
  while (read_register(0x4001e400) != 1) //Read ready register
  {
    if(millis()-timeout>100)return 1;
  }
  write_register(adrs, value); // Write data to desired adress
  while (read_register(0x4001e400) != 1) // Read ready register
  {
    if(millis()-timeout>100)return 2;
  }
  write_register(0x4001e504, 0); // Write 0 to CONFIG
  return 0;
}

/*
 * Erases all using the Non-Volatile Memory Controller (not in use here)
 * More info at:
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fps_nrf52833%2Fnvmc.html&cp=5_1_0_3_2&anchor=register.ERASEALL
 */
uint8_t NRF_IF::nvmc_erase_all(){
  port_sel(0);
  write_register(0x4001E504, 2); // Write 0x2 to CONFIG
  long timeout = millis();
  while (read_register(0x4001E400) != 1) // Read READY till val = 0x1
  {
    if(millis()-timeout>1000)return 1;
  }
  write_register(0x4001E50C, 1); // Write 1 to ERASEALL register
  timeout = millis();
  while (read_register(0x4001e400) != 1) // Read READY until val = 0x1
  {
    if(millis()-timeout>1000)return 2;
  }
  write_register(0x40014504, 0); // Write 0x00 to CONFIG register
  timeout = millis();
  while (read_register(0x4001e400) != 1)
  {
    if(millis()-timeout>1000)return 3;
  }
  return 0;
}

/*
 * This function performs an ERASEALL command using Nordic Semiconductors CTRL-AP
 * More info at:
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fnwp_027%2FWP%2Fnwp_027%2FnWP_027_erasing.html
 */
void NRF_IF::ctrl_erase_all(){
  Serial.println("Start Erase all");
  port_sel(1); // Select CTRL-AP 
  write_port(0x00000001, CTRL_AP_ERASEALL, 1); // Write 0x01 to ERASEALL register
  long timeout = millis();
  while(((millis()-timeout)<15000)&&(read_port(CTRL_AP_ERASEALLSTAT, 1)!= 0x00)){ // Do nothing while STATUS != 0x0 or timeout occurs
    ;
  }
  write_port(0x0, CTRL_AP_ERASEALL, 1); // Write 0x0 to the ERASEALL register to complete sequence
  Serial.println("End Eraseall");
  port_sel(0);
}

/*
 * Uses the CTRL-AP to issue a soft reset of the device
 */
void NRF_IF::ctrl_reset(){
  write_port(0x1 ,CTRL_AP_RESET, 1); // Write 0x1 to RESET register
  delay(10);
  write_port(0x0, CTRL_AP_RESET, 1); // Write 0x0 to RESET register
}

uint8_t NRF_IF::flash_file(uint32_t offset){
  Serial.println("Flashing file");
  File f = LittleFS.open(standard_filename, "r");
  if(f == 0){
    return 0;
  }
  uint32_t file_size = f.size();
  uint8_t buffer[4096] = {0x00};

  for(int position = 0; position < file_size; position += 4096){
    Serial.println(position);
    uint32_t cur_len = (file_size - position >= 4096) ? 4096 : file_size - position;
    size_t bytes_read = f.read(buffer, cur_len);
    write_bank((uint32_t *)&buffer, position+offset, cur_len);
  }
  f.close();
  Serial.println("Completed flashing");
  return 1;
}

uint8_t NRF_IF::write_bank(uint32_t buffer[], uint32_t adrs, int size){
  port_sel(0);
  uint32_t temp;
  write_register(1, 0x4001E504);
  long timeout = millis();
  while(read_register(0x4001E400) != 1){
    if(millis()-timeout>300) {
      if(_debug) Serial.println("Timeout");
      return 3;
      }
  }
  Serial.println("Ready to write");
  _swd.AP_write(0xA2000012, MEM_AP_CSW);
  _swd.AP_write(adrs, MEM_AP_TAR);

  for(int position = 0; position < size; position += 4){
    long end_micros = micros() + 400;
    _swd.AP_write(buffer[position / 4], MEM_AP_DRW);
    while(micros() < end_micros){
      ;
    }
    Serial.println(position);
  }

}

bool NRF_IF::check_tasks(){
  if(connection_status){
    if(task_write_flash){
      /*Flash file*/
      task_write_flash = false;
      return true;
    }
    else if(task_read_flash){
      /*Read file*/
      task_read_flash = false;
      return true;
    }
    else{
      return true;
    }
  }
  else{
    return false;
  }
} 

/*
 * This function performs an AP or DP-read sequence
 * @adrs: address to be read from, most found in defines.h
 * @APnDP: AP(1) or DP(0) selection 
 */
uint32_t NRF_IF::read_port(uint8_t adrs, bool APnDP){
  uint32_t temp = 0;
  if(APnDP){
    _swd.AP_read(temp, adrs);
  }
  else{
    _swd.DP_read(temp, adrs);
  }
  _swd.DP_read(temp, DP_RDBUFF);
  _swd.DP_read(temp, DP_RDBUFF);
  return temp;
}

/*
 * This function performs an AP or DP write sequence, then dummy reads the RD_BUFFER
 * @data: uint32 of data to be written
 * @adrs: address to be written to (most found in defines.h)
 * @APnDP: Select AP(1) or DP(0)
 */
void NRF_IF::write_port(uint32_t data, uint8_t adrs, bool APnDP){
  uint32_t temp = 0;
  if(APnDP){
    _swd.AP_write(data, adrs);
  }
  else{
    _swd.DP_write(data, adrs);
  }
  _swd.DP_read(temp, DP_RDBUFF);
  _swd.DP_read(temp, DP_RDBUFF); // Dummy read
}

/*
 * This function reads a register using MEM-AP
 * @adrs: memory address being accessed
 */
uint32_t NRF_IF::read_register(uint32_t adrs){
  uint32_t temp = 0;
  _swd.AP_write(adrs, MEM_AP_TAR);
  _swd.AP_read(temp, MEM_AP_DRW);
  _swd.DP_read(temp, DP_RDBUFF);
  _swd.DP_read(temp, DP_RDBUFF);
  return temp;
}

/*
 * This function writes to a register using MEM-AP
 * @data: 32 bits to be written to the register
 * @adrs: 32 bit adress of the register
 */
void NRF_IF::write_register(uint32_t data, uint32_t adrs){
  uint32_t temp = 0;
  bool state1 = _swd.AP_write(adrs, MEM_AP_TAR);
  bool state2 = _swd.AP_write(data, MEM_AP_DRW);
  bool state3 = _swd.DP_read(temp, DP_RDBUFF);
}

/*
 * This function switches between the standard MEM-AP and Nordic Semiconductors CTRL-AP
 * @nrf_port: bool indicates which port is selected
 * 0 for MEM-AP, 1 for CTRL-AP
 */
void NRF_IF::port_sel(bool nrf_port){
  uint32_t temp = nrf_port ? 0x01000000 : 0x00;
  _swd.DP_write(temp, DP_SELECT);
}

/*
 * This function reads the lockstate from Nordic Semiconductors
 * CTRL-AP - APPROTECTSTAUS(0x0C) register
 * The register should have the value 0 or 1, for enabled or disabled
 * returns 0 when nRF is locked
 */
bool NRF_IF::read_lockstate(){
  uint32_t temp;
  port_sel(1);
  temp = read_port(CTRL_AP_APPROTECTSTAT, 1);
  port_sel(0);
  return temp & 1;
}


/***************************************************
            Section: Private functions
****************************************************/