#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include "nrf24l01.h"
volatile uint8_t* data;
#include <string.h>

volatile unsigned char dmx_buffer[512];

ISR(PCINT1_VECT) {

		data = nrf24l01_recieve(32);
		nrf24l01_reset();
		/*int i;
		for(i = 0; i < 32; i++) {
			dmx_buffer[0] = 255;
		}*/
		dmx_buffer[0] = 255;    
	
}

void serial_send(char* ar) {
	int i;
	for (i = 0; i < strlen(ar); i++){
		while (( UCSR0A & (1<<UDRE0))  == 0){};
		_delay_ms(1);
		UDR0 = ar[i];
	}
}

void init_uart() {
	CLKPR = 128;
	CLKPR = 0;
	DDRD |= (1 << PIND1);
	DDRD &= ~(1 << PIND0);
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);   // Turn on the transmission and reception circuitry
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); // Use 8-bit character sizes

	UBRR0H = 0;
	UBRR0L=54;
}

int main()
{
	uint8_t val[5];
	
	
	DDRC &= ~(1 << DDC5);
	
	PCICR |= (1 << PCIE1);    // set PCIE0 to enable PCMSK0 scan
    PCMSK0 |= (1 << PCINT13);  // set PCINT0 to trigger an interrupt on state change 
	
	sei();

	init_uart();

    initialize_nrf24l01();
    delayms(200);
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
	delayms(100);
	
		data = nrf24l01_recieve(32);
		nrf24l01_reset();
	
	
    while(1) {

	
	/*	nrf24l01_transmit(data);
		nrf24l01_reset();
		_delay_ms(100);
		uint16_t i;
		for(i = 0; i < 512; i++) {
			char buffer[8 * sizeof (uint8_t) + 1 ];
			utoa(dmx_buffer[0], buffer, 10);
			serial_send(buffer);
			//_delay_ms(10);
			serial_send("\r\n");
		}*/
    }

    return 1;
}