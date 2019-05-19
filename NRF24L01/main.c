/*
 * main.c
 *
 *  Created on: 19. mai 2019
 *      Author: rando
 */


#include <avr/io.h>
#include <util/delay.h>
#include "nrf24l01.h"
#include <avr/wdt.h>

int main(void)
{
    wdt_reset();
    MCUSR=0;
    WDTCSR|=_BV(WDCE) | _BV(WDE);
    WDTCSR=0;
    initialize_nrf24l01();
    _delay_ms(200);
    //Enable AA
    nrf24l01_write_register(EN_AA, 0x01);
    //Enable data pipe 0
    nrf24l01_write_register(EN_RXADDR, 0x01);
    //Set address as 5 byte
    nrf24l01_write_register(SETUP_AW, 0x03);
    //Channel setup
    nrf24l01_write_register(RF_CH, 0x01);
    // Long range -0dB
    nrf24l01_write_register(RF_SETUP,0x07);
    //Siia tuleb teha aadressi asi


    while(1) {
        uint8_t data = nrf24l01_read_register(STATUS);
        _delay_ms(1000);
    }

    return 0;
}