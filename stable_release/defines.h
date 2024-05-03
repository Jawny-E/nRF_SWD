/* Feel free to Edit or replace these variables: */
#define pin_SWDIO 19
#define pin_SWDCLK 21
#define STANDARD_FILENAME "/firmware.bin"
#define YOUR_URL "http://192.168.177.143:8000/zephyr.bin"
#define YOUR_SSID "ESP123"
#define YOUR_PASSWORD "ESP321321"

/* Be careful before editing these variables*/
// Frequency and period
#define SWDCLK_FREQ 200000
#define SWDCLK_PERIOD 10

// SW-DP registers 
#define DP_IDCODE 0x00
#define DP_CTRL 0x04
#define DP_RESEND 0x08
#define DP_SELECT 0x08
#define DP_RDBUFF 0x0C

// nRF CTRL AP-registers
#define CTRL_AP_RESET 0x00
#define CTRL_AP_ERASEALL 0x04
#define CTRL_AP_ERASEALLSTAT 0x08
#define CTRL_AP_APPROTECTSTAT 0x0C
#define CTRL_AP_IDR 0xFC

// MEM-AP registers
#define MEM_AP_CSW 0x00
#define MEM_AP_TAR 0x04
#define MEM_AP_DRW 0x0C
#define MEM_AP_BD0 0x10
#define MEM_AP_BD1 0x14
#define MEM_AP_BD2 0x18
#define MEM_AP_BD3 0x1C
#define MEM_AP_CFG 0xF4
#define MEM_AP_BAS 0xF8
#define MEM_AP_IDR 0xFC

struct NRF_INFO{
  int flash_size;
  uint32_t core_id;
  uint32_t codepage_size;
  uint32_t codesize;
  uint32_t device_id0;
  uint32_t device_id1;
  uint32_t info_part;
  uint32_t info_variant;
  uint32_t info_package;
  uint32_t ucir_lock;
};
