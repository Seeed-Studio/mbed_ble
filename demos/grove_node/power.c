
#include "nrf51.h"
#include "nrf51_bitfields.h"

#define POWER_PIN   8

void power_on()
{
    NRF_GPIO->PIN_CNF[POWER_PIN] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                            | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                            | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                                            | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                            | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    NRF_GPIO->OUTSET = (1UL << POWER_PIN);
}


void power_down()
{
    NRF_GPIO->OUTCLR = (1UL << POWER_PIN);
    
    // Enter system OFF. After wakeup the chip will be reset, and the MCU will run from the top 
    NRF_POWER->SYSTEMOFF = 1;
}
