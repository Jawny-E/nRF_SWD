#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <hal/nrf_gpio.h>

// Starts an array of uint32_t at registeraddress 0x20000100
// Each data entry is 0x4 wide
static uint32_t * dataPlacement = (uint32_t *)0x20000100;
static int stateControl = 0;
static int count = 1;

#define INPUT_PIN NRF_GPIO_PIN_MAP(1,2)
#define OUTPUT_PIN NRF_GPIO_PIN_MAP(1,1)
#define SLEEP_TIME 1000

int main(void)
{
    nrf_gpio_cfg_output(OUTPUT_PIN);
    nrf_gpio_cfg_input(INPUT_PIN, NRF_GPIO_PIN_PULLDOWN);
    approtect_hw_disable();
    while (1) {
        // INPUT/OUTPUT
        k_msleep(SLEEP_TIME);
        switch(stateControl){
            case 0:    // 0/0 - Waiting 
                printf("Case is: %i\n", stateControl);
                if(nrf_gpio_pin_read(INPUT_PIN) == 1){
                    stateControl = 1;
                }
                else{
                    stateControl = 0;
                }
                break;
            case 1:    // 1/0 - Measurement has been requested
                printf("Case is: %i\n", stateControl);
                /*Change this case!*/
                if(count > 5){
                    count = 1;
                }
                else{
                    count += 1;
                }
                for(int i = 0; i < 5; i+= 1){
                    dataPlacement[i] = sqr(i*count);
                    printf("Memory address is: %p\n", &dataPlacement[i]);
                    printf("Value squared is: %d\n", dataPlacement[i]);
                    k_msleep(SLEEP_TIME);
                }
                nrf_gpio_pin_set(OUTPUT_PIN);
                stateControl = 2;
                break;
            case 2:    // 1/1 - Measurement is finished
                printf("Case is: %i\n", stateControl);
                while(nrf_gpio_pin_read(INPUT_PIN) != 0){
                    ;
                }
                stateControl = 3;
            case 3:    // 0/1 - Data has been read
                printf("Case is: %i\n", stateControl);
                nrf_gpio_pin_clear(OUTPUT_PIN);
                stateControl = 0;
                break;
        }
    }
    return 0;
}

int sqr(int n1)
{
    return n1*n1;
}

void approtect_hw_disable(void)
{
    if ((NRF_UICR->APPROTECT & UICR_APPROTECT_PALL_Msk) 
       == (UICR_APPROTECT_PALL_Msk))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->APPROTECT = (UICR_APPROTECT_PALL_HwDisabled << UICR_APPROTECT_PALL_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NVIC_SystemReset();
    }  
}