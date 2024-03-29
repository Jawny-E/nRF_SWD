#include "Arduino.h"
#include "swd.h"

/* Package
    - Startbit 
    - APnDP - This bit is 0 for an DPACC access, or 1 for a APACC access
    - RnW -  This bit is 0 for an write
access, or 1 for a read acces
    - A[2:3] - two bits giving adress field, LSB first
    - Parity bit for the preceding packet
    - Stop bit
    - Park
    - Trn of one cycle
    - ACK[0:2] LSB first
    - Trn of one cycle
    - WDATA [0:31] LSB first
    - RDATA [0:31] LSB first
    - Parity for data
*/

/***************************************************
            Section: Public functions
****************************************************/

SWD::SWD(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD){
  _SWDCLK = SWDCLK;
  _SWDIO = SWDIO;
  _SWD_clk_halfperiod = SWD_CLK_HALFPERIOD;
}

void SWD::init(){
  pinMode(_SWDCLK, OUTPUT);
  pinMode(_SWDIO, OUTPUT);
  connection_seq();
}

uint32_t SWD::DP_read(uint32_t &data, unsigned adress){
  /*
  send_request();
  trns();
  read();
  */
}
uint32_t SWD::AP_read(){
}
void SWD::DP_write(){
    /*
  send_request();
  trns();
  write();
  */
}
void SWD::AP_write(){
}
/***************************************************
          Section: Private functions
****************************************************/

/*
  - Startbit 
  - APnDP
  - A[2:3] - two bits giving adress field, LSB first
  - Parity bit for the preceding packet
  - Stop bit
  - Park
*/

/*
*This function directly controls the SWDCLK, and SWDIO lines when writing
* @output_data: data to be transferred
* @length: number of bits to be transferred
*/
void SWD::write(uint32_t output_data, uint8_t length){
  while(length--){
    digitalWrite(_SWDIO, output_data & 1);
    digitalWrite(_SWDCLK, HIGH);
    delayMicroseconds(_SWD_clk_halfperiod);
    output_data >>= 1;
    digitalWrite(_SWDCLK, LOW);
    delayMicroseconds(_SWD_clk_halfperiod);
  }
  digitalWrite(_SWDIO, LOW);
  delayMicroseconds(_SWD_clk_halfperiod);
}
/*
 *This function directly controls the SWDCLK, and SWDIO lines when reading 
 *@length: number of bits to be read
 */
uint32_t SWD::read(uint8_t lenght){
  uint32_t temp;
  pinMode(_SWDIO, INPUT);
  while(lenght--){
    digitalWrite(_SWDCLK, HIGH);
    delayMicroseconds(_SWD_clk_halfperiod);
    digitalWrite(_SWDCLK, LOW);
    bool readval = digitalRead(_SWDIO);
    temp |= readval << lenght;
    delayMicroseconds(_SWD_clk_halfperiod);
  }
  return temp;
}

/*
 * This function handles the creating of SWD - request and writes it to SWDIO
 * @adrs
 * @APnDP - Set high to access AP, low for DP
 * @RnW   - Set high to read, low for write
 */
void SWD::send_request(unsigned adrs, bool APnDP, bool RnW){
  pinMode(_SWDIO, OUTPUT);
  bool parity = ((adrs >> 2) & 1) ^ ((adrs >> 3) & 1) ^ APnDP  ^ RnW;
  uint8_t request_packet = (1 << 0) | (APnDP << 1) | (RnW << 2) | ((adrs & 0xC) << 1) | (parity << 5) | (1 << 7);
  write(request_packet, 8);
}

/*
 * This function handles the Trn and ACK phase between sending a request and reading/writing data
 * The function returns false if handle_ACK fails (response is not OK)
 * @RnW - Read or Write bit, determines _SWDIO input or output
 */
bool SWD::trns(bool &RnW){
  //Trn
  digitalWrite(_SWDCLK, HIGH);
  pinMode(_SWDIO, INPUT_PULLUP);
  delayMicroseconds(_SWD_clk_halfperiod /2);
  digitalWrite(_SWDCLK, LOW);
  delayMicroseconds(_SWD_clk_halfperiod /2);
  //Check ACK
  if(handle_ACK()){
    //Trn
    digitalWrite(_SWDCLK, HIGH);
    if(RnW){ pinMode(_SWDIO, INPUT_PULLUP); }
    else{ pinMode(_SWDIO, OUTPUT); }  
    delayMicroseconds(_SWD_clk_halfperiod /2);
    digitalWrite(_SWDCLK, LOW);
    delayMicroseconds(_SWD_clk_halfperiod /2);
    return true;
  }
  else{return false;}
}

/*
 * Handles ACK responses, in SWD only 0b001 is OK response
 * returns false if ACK is not OK
 */
bool SWD::handle_ACK(){
  //0b001 == OK
  uint32_t ACK = read(3);
  if (ACK == 1){
    return true;
  }
  else{
    return false;
  }
}

/*
 * This function handles the connection sequence for SWD
 * which consists of 50 clock periods with SWDIO == HIGH
 */
void SWD::connection_seq(){
  for (int i = 0; i <= 51; i++){
    digitalWrite(_SWDCLK, HIGH);
    digitalWrite(_SWDIO, HIGH);
    delayMicroseconds(_SWD_clk_halfperiod);
    digitalWrite(_SWDCLK, LOW);
    delayMicroseconds(_SWD_clk_halfperiod);
  }
}
