/*
Specialised SWD interface for use with the nRF 52833 dk. 
It has been tested for build codes Bxx or later, but might work on other devices
Parameters:
 * @SWDCLK: Pin for the clock 
 * @SWDIO: Pin for serial transfer
 * @SWD_CLK_HALFPERIOD: Half the clockperiod in microseconds
 * @debug: Enables Serial debug messages on the ESP32
*/

#pragma once
#include "swd.h"

class NRF_IF{
  public:
    NRF_IF(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug);
    void nrf_test();

    void ctrl_erase_all();
    void ctrl_reset();
    void core_unhalt();
    void core_halt();
    void begin_if();
    void end_if();
    bool check_tasks();
    uint32_t read_port(uint8_t adrs, bool APnDP);
    void write_port(uint32_t data, uint8_t adrs, bool APnDP);
    uint32_t read_register(uint32_t adrs);
    void write_register(uint32_t data, uint32_t adrs);

    void read_info();
    void port_sel(bool nrf_port);
    bool read_lockstate();
    void soft_rst();

    uint8_t nvmc_write(uint32_t value, uint32_t adrs);
    uint8_t nvmc_erase_all();
    uint8_t flash_file(uint32_t offset);
    uint8_t write_bank(uint32_t buffer[], uint32_t adrs, int size);

  private:
    SWD _swd;
    bool _debug;
};