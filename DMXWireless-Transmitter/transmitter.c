#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include "nrf24l01.h"
#include <avr/interrupt.h>
#include <string.h>

volatile unsigned char dmx_buffer[512];

volatile unsigned char data[2][10];
volatile int data_index = 0;
volatile unsigned char data_count = 0;

volatile int data_valid = 0;

ISR(USART_RX_vect)
{
	data[data_index][data_count] = UDR0;
	if(data[data_index][data_count] == '\r') {
		data_count = 0;
		data_valid = 1;
		data_index = !data_index;

	} else {
		data_count++;
	}
}

void usart_putc (char send)
{
    // Do nothing for a bit if there is already
    // data waiting in the hardware to be sent
    while ((UCSR0A & (1 << UDRE0)) == 0) {};
    UDR0 = send;
}

void usart_puts (const char *send)
{
    // Cycle through each character individually
    while (*send) {
        usart_putc(*send++);
    }
}

void init_uart() {
    CLKPR = 128;
    CLKPR = 0;
    DDRD |= (1 << PIND1);
    DDRD &= ~(1 << PIND0);
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);   // Turn on the transmission and reception circuitry
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); // Use 8-bit character sizes
    UBRR0H = 0;
    UBRR0L=51;
}

int main() {
    uint8_t val[5];

    initialize_nrf24l01();
    init_uart();
    sei();
    
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
    val[0] = 3;
    nrf24l01_communicate(W, RX_PW_P0, val, 1);
    
    // boot up and set as transmitter;
    val[0] = 0x1E; //0b0001 1110 bit 0 = 0/1 transmitter/reciever bit 1 powerup , bit4 if 1 = mask_max_RT = irq not triggered if failed transmit
    nrf24l01_communicate(W, CONFIG, val, 1);
    _delay_ms(100);
    uint16_t j;
    uint8_t* buffer[3];
    volatile int buffer_i = 0;
    while(1) {
        if(data_valid) {
			int process_index = !data_index;
	
			int i;
			uint16_t addr = 0;
			uint8_t val = 0;
			char* s = data[process_index];
			for(i=0; i < 3; i++) {
				addr = addr*10+(*(s+i)-'0');
			} 
			for(i=4; i < 7; i++) {
				val = val*10+(*(s+i)-'0');
			} 
			dmx_buffer[addr] = val;
			
			buffer[0] = addr;
            buffer[1] = (addr >> 8);
            buffer[2] = val;
			nrf24l01_transmit(buffer, 3);
            nrf24l01_reset();
            buffer_i = 0;
			data_valid = 0;
        }
		
       /* for (j = 0; j < 512; j++) {
            buffer[buffer_i] = j;
            buffer[buffer_i+1] = (j >> 8);
            buffer[buffer_i+2] = dmx_buffer[j];
            buffer_i = buffer_i+3;
            if(buffer_i >= 30) {
                nrf24l01_transmit(buffer, 30);
                nrf24l01_reset();
                buffer_i = 0;
            }
        }*/
    }
    return 1;
}