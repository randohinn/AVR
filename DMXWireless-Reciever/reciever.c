#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

#include "nrf24l01.h"
volatile uint8_t* data;
#include <string.h>

volatile unsigned char dmx_buffer[512];

ISR(PCINT1_vect) {

	if((PINC & (1 << PINC5)) == 1){
    }
    else
    {
        /* HIGH to LOW pin change */

		data = nrf24l01_recieve(30);
		nrf24l01_reset();
		volatile uint8_t at = 0;
		int ok = 0;
		while(1) {
			uint16_t addr = (data[at+1] << 8) | data[at];
			char buffer[8 * sizeof (uint16_t) + 1 ];
			utoa(addr, buffer, 10);
			serial_send(buffer);
			serial_send("\r\n");
			at++;
			at++;
			at++;
			if(at >= 30) {
				break;
			}
		}
		
	}
}

void serial_send(char* ar) {
	int i;
	for (i = 0; i < strlen(ar); i++){
		while (( UCSR0A & (1<<UDRE0))  == 0){};
		delayms(1);
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
	DDRC |= (1 << DDC4);
	
	PCICR |= (1 << PCIE1);    // set PCIE0 to enable PCMSK0 scan
    PCMSK1 |= (1 << PCINT13);  // set PCINT0 to trigger an interrupt on state change 
	

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
	
	val[0] = 30;
	// Payload width (how many bytes per package) 1-32
	nrf24l01_communicate(W, RX_PW_P0, val, 1);
	
	val[0] = 0x1F;
	nrf24l01_communicate(W, CONFIG, val, 1);
	delayms(100);	
	data = nrf24l01_recieve(30);
	nrf24l01_reset();
	sei();

	while(1) {


		
    }

    return 1;
}