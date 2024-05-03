/*
 * General SWD functions that can be used with a multitude of 
   components that support SWD
 * Parameters:
 * @SWDCLK: Pin for the clock 
 * @SWDIO: Pin for serial transfer
 * @SWD_CLK_HALFPERIOD: Half the clockperiod in microseconds
*/

#pragma once

class SWD{
  public:
  /*
   * SWD class function
   * @SWDCLK: Pin number for clock connection
   * @SWDIO: Pin number for serial(data) connection
   * @SWD_CLK_HALFPERIOD: Half period of clock signal in microseconds, usually 5
   * @debug: Turns on 
   */
    SWD(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug);
    void init(); // Setup pins
    uint32_t begin(); //Reads the id code
    
    bool DP_read(uint32_t &data, unsigned adress);  // Reads from DP register
    bool DP_write(uint32_t data, unsigned adress); // Writes to DP register
    bool AP_read(uint32_t &data, unsigned adress);  // Reads from AP register
    bool AP_write(uint32_t data, unsigned adress); // Writes to AP register
    
    void write(uint32_t output_data, uint8_t length); // Controls SWDIO and SWDCLK when writing
    uint32_t read(uint8_t lenght); // Controls SWDIO and SWDCLK when reading

  private: 
    int _SWDCLK; // Stores pin
    int _SWDIO; //Stores pin
    int _SWD_clk_halfperiod; // Stores half of the clock period (in microseconds)
    bool _debug; //If initiated 1 serial debug information will print

    bool send_request(unsigned adrs, bool APnDP, bool RnW); // Creates and sends data request
    bool trn(bool RnW); // Handles Trn periods and ACK, returns false on failure
    bool handle_ACK(); // Handles ACK, return false on response not OK
    bool check_parity(uint32_t check_data); // Checks the parity of a uint32_t number
};

