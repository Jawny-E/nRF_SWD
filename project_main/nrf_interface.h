/*
 * Specialised SWD interface for use with the nRF 52833 dk. It uses an instance of SWD_IF
 * to get basic SWD commands. The class handles reading/writing/halting the nRF device
 * It has been tested for build codes Bxx or later, but might work on other devices
 * Parameters:
 * @SWDCLK: Pin for the clock 
 * @SWDIO: Pin for serial transfer
 * @SWD_CLK_HALFPERIOD: Half the clockperiod in microseconds (recommended: 5)
 * @debug: Enables Serial debug messages on the ESP32
*/

#pragma once
#include "swd.h"

class NRF_IF{
  public:
    NRF_IF(int SWDCLK, int SWDIO, int SWD_CLK_HALFPERIOD, bool debug);
    bool begin_if(); //Begins the interface
    void end_if(); //Ends the interface

    void read_info(); //Reads FICR info from the nRF
    void ctrl_erase_all(); //Issues an eraseall operation using Nordic Semiconductors CTRL-AP
    void ctrl_reset(); //Unused

    bool flash_write_from_file(String filepath, uint32_t offset); // Flashes a file of .bin format consecutively beginning at the offset address
    bool flash_read_to_file(String filepath, uint32_t byte_amount, uint32_t offset); // Reads a given sectio

    void measurement(uint32_t* buffer, uint32_t start_address, uint32_t size); //Non-disturbing measurement request
  private:
    uint32_t read_port(uint8_t adrs, bool APnDP);
    void write_port(uint32_t data, uint8_t adrs, bool APnDP);
    uint32_t read_register(uint32_t adrs);
    void write_register(uint32_t data, uint32_t adrs);

    bool nvmc_write(uint32_t value, uint32_t adrs);
    bool nvmc_erase_all(); //Unused

    void core_unhalt();
    void core_halt();

    bool read_lockstate();
    void port_sel(bool nrf_port);
    bool write_bank(uint32_t buffer[], uint32_t adrs, int size);
    bool read_bank(uint32_t buffer[], uint32_t adrs, int size);

    SWD_IF _swd;
    bool _debug;
};