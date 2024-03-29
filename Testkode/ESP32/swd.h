#pragma once
class SWD{
  public:
    SWD(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD);
    void init(); // Setup pins
    uint32_t DP_read(uint32_t &data, unsigned adress); // Reads from DP register
    uint32_t AP_read(); // Reads from AP register
    void DP_write(); // Writes to DP register
    void AP_write(); // Writes to AP register
  private: 
    int _SWDCLK; // Stores pin
    int _SWDIO; //Stores pin
    int _SWD_clk_halfperiod; // Stores half of the clock period (in microseconds)
    bool trns(bool &RnW); // Handles Trn periods and ACK, returns false on failure
    bool handle_ACK(); // Handles ACK, return false on response not OK
    void connection_seq(); //SWD connection sequence of 50 high bits
    void send_request(unsigned adrs, bool APnDP, bool RnW); // Creates and sends data request
    void write(uint32_t output_data, uint8_t length); // Controls SWDIO and SWDCLK when writing
    uint32_t read(uint8_t lenght); // Controls SWDIO and SWDCLK when reading
};

