#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include "nrf24l01.h"

volatile unsigned char dmx_buffer[512];

int main() {
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

	// Module address generation, just 5* 0x12 for now
	int i;
	for(i = 0; i < 5; i++) {
		val[i] = 0x12;
	}
	nrf24l01_communicate(W, RX_ADDR_P0, val, 5);	
	nrf24l01_communicate(W, TX_ADDR, val, 5);	
	
	// Payload width (how many bytes per package) 1-32
	val[0] = 32;
	nrf24l01_communicate(W, RX_PW_P0, val, 1);
	
	// boot up and set as transmitter;
	val[0] = 0x1E; //0b0001 1110 bit 0 = 0/1 transmitter/reciever bit 1 powerup , bit4 if 1 = mask_max_RT = irq not triggered if failed transmit
	nrf24l01_communicate(W, CONFIG, val, 1);
	_delay_ms(100);
	dmx_buffer[0]=255;
	dmx_buffer[1]=255;
	dmx_buffer[2]=255;

	int j;
	uint8_t* buffer[32];
	volatile int buffer_i = 0;
	while(1) {
		for (j = 0; j < 512; j++) {
			buffer[buffer_i] = j;
			buffer[buffer_i+1] = dmx_buffer[j];
			buffer[buffer_i+2] = 0X3B;
			buffer[buffer_i+3] = 0x2E;
			buffer_i = buffer_i+4;
			if(buffer_i >= 32) {
				nrf24l01_transmit(buffer, 32);
				nrf24l01_reset();
				buffer_i = 0;
			}
			
		}
	}
	return 1;
}