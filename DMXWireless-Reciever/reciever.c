#define F_CPU 20000000
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>


#include <avr/interrupt.h>

#include "nrf24l01.h"
volatile uint8_t* data;
#include <string.h>

#define DMX_BAUD 250000UL
#define DMX_BAUD_BREAK 80000UL

volatile unsigned char dmx_buffer[512];

void serial_send(char* ar) {
	int i;
	for (i = 0; i < strlen(ar); i++){
		while (( UCSR0A & (1<<UDRE0))  == 0){};
		delayms(1);
		UDR0 = ar[i];
	}
}

ISR(PCINT1_vect) {
	if(! (PINC && (1 << PINC5))) {
		PORTC |= (1 << PINC4);
	}

}

static unsigned int  dmx_channel_tx_count = 0;
static unsigned char dmx_tx_state = 0;
//############################################################################
//DMX Senderoutine
ISR (USART_TX_vect)
//############################################################################
{
	
	
	switch (dmx_tx_state)
	{
		case (0):
			UBRR0   = (F_CPU / (DMX_BAUD_BREAK * 16L) - 1);
			UDR0 = 0; //RESET Frame
			dmx_tx_state = 1;
			break;

		case (1):
			UBRR0   = (F_CPU / (DMX_BAUD * 16L) - 1);
			UDR0 = 0; //Start Byte
			dmx_tx_state = 2;
			break;

		case (2):
			delay(10);
			//Ausgabe des Zeichens
			UDR0 = dmx_buffer[dmx_channel_tx_count];
			dmx_channel_tx_count++;
			
			if(dmx_channel_tx_count == 512)
			{
				dmx_channel_tx_count = 0;
				dmx_tx_state = 0;
			}
			break;
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

	PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT13);

	//init_uart();

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
	
	val[0] = 3;
	// Payload width (how many bytes per package) 1-32
	nrf24l01_communicate(W, RX_PW_P0, val, 1);
	val[0] = 0x1F;
	nrf24l01_communicate(W, CONFIG, val, 1);
	delayms(100);	
	
	data = nrf24l01_recieve(3);
	nrf24l01_reset();
	
	data = nrf24l01_recieve(3);
	nrf24l01_reset();
//Init usart DMX-BUS
	UBRR0   = (F_CPU / (DMX_BAUD * 16L) - 1);
	DDRD |= (1<<PIND1); //Output TXD Pin ATmega88
	UCSR0B|=(1<<TXEN0)|(1<<TXCIE0); // TXEN0 Transmitter enable / TXCIE0 TX complete interrupt enable 
	UCSR0C|=(1<<USBS0); //USBS0 2 Stop bits	
	sei();//Globale Interrupts Enable
	UDR0 = 0;//Start DMX	
	while(1) {
		if(PINC && (1 << PINC4)) {
			data = nrf24l01_recieve(3);
			uint8_t lower =  data[0];
			uint8_t upper =  data[1];
			uint16_t addr = ((uint16_t) (upper << 8) ) | lower;
			dmx_buffer[addr] = data[2];
				
			PORTC &= ~(1 << PINC4);
			nrf24l01_reset();
			
		}
    }

    return 1;
}