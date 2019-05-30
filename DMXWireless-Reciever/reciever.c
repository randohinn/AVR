#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include "nrf24l01.h"

ISR(INT0_VECT) {

}

int main()
{
	uint8_t val[5];

    initialize_nrf24l01();
    _delay_ms(200);
    //Enable AA	
	val[0] = 0x01;
	nrf24l01_communicate(W, EN_AA, val, 1);
	
	// Setup max retries
	val[0] = 0x2F;
	nrf24l01_communicate(W,SETUP_RETR, val, 1);
	
    //Enable data pipe 0
	val[0] = 0x01;
    nrf24l01_communicate(W, EN_RXADDR, val, 1);
	
	//Set address as 5 byte
	val[0] = 0x03;
	nrf24l01_communicate(W, SETUP_AW, val, 1);
	
    //Channel setup
    val[0] = 0x01;
	nrf24l01_communicate(W, RF_CH, val, 1);
    // Long range -0dB
	val[0] = 0x07;
	nrf24l01_communicate(W, RF_SETUP, val, 1);

	// Module address
	int i;
	for(i = 0; i < 5; i++) {
		val[i] = 0x12;
	}
	nrf24l01_communicate(W, RX_ADDR_P0, val, 5);	
	nrf24l01_communicate(W, TX_ADDR, val, 5);	
	
	val[0] = 32;
	// Payload width (how many bytes per package) 1-32
	nrf24l01_communicate(W, RX_PW_P0, val, 1);
	
	val[0] = 0x1F;
	nrf24l01_communicate(W, CONFIG, val, 1);
	_delay_ms(100);
	
	uint8_t* data;
	
	
	DDRD &= ~(1<<DDD2);
	EICRA |= (1<<ISC01);
	sei();
	//data[0] = 0x01;
    while(1) {
	/*	nrf24l01_transmit(data);
		nrf24l01_reset();
		_delay_ms(100);*/
		data = nrf24l01_recieve(32);
		nrf24l01_reset();
		

    }

    return 1;
}
