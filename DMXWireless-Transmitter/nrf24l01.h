/*
 * nrf24l01.h
 *
 *  Created on: 19. mai 2019
 *      Author: rando
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_


// The register map of this file is derived from maniacbug (2007 Stefan Engelke <mbox@stefanengelke.de>) - https://github.com/maniacbug/RF24/blob/master/nRF24L01.h also licensed under MIT

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD	    0x1C
#define FEATURE	    0x1D

/* Bit Mnemonics */
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0
#define AW          0
#define ARD         4
#define ARC         0
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      6
#define RX_DR       6
#define TX_DS       5
#define MAX_RT      4
#define RX_P_NO     1
#define TX_FULL     0
#define PLOS_CNT    4
#define ARC_CNT     0
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0
#define DPL_P5	    5
#define DPL_P4	    4
#define DPL_P3	    3
#define DPL_P2	    2
#define DPL_P1	    1
#define DPL_P0	    0
#define EN_DPL	    2
#define EN_ACK_PAY  1
#define EN_DYN_ACK  0


/* Instruction Mnemonics */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define ACTIVATE      0x50
#define R_RX_PL_WID   0x60
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define W_ACK_PAYLOAD 0xA8
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

/* Non-P omissions */
#define LNA_HCURR   0

/* P model memory Map */
#define RPD         0x09

/* P model bit Mnemonics */
#define RF_DR_LOW   5
#define RF_DR_HIGH  3
#define RF_PWR_LOW  1
#define RF_PWR_HIGH 2


/* shorter modes for the comms function */
#define W 1
#define R 0


/* Taneli magic */
void delay(uint16_t len){
  len=len-2;
  for(;len>0;len--){
    asm volatile("nop");
    asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
  }
}

void delayms(uint8_t len){
  len--; 
	for (;len>0;len--){
	delay(999);
  	asm volatile("nop");
  	asm volatile("nop");
  	asm volatile("nop");
  	asm volatile("nop");
  	asm volatile("nop");
  	asm volatile("nop");		
  	asm volatile("nop");		
	}
	delay(998);
}

uint8_t send_recieve_spi_byte(uint8_t data)
{
	// Load data into the buffer
	SPDR = data;
	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));

	// Return received data
	return(SPDR);
}


uint8_t* nrf24l01_communicate(uint8_t mode, uint8_t reg, uint8_t* package, uint8_t byte_count) {
	if(mode == W) {
		reg = W_REGISTER + reg; 
	} else {
		reg = R_REGISTER + reg;
	}
	
	static uint8_t ret[32];
	
	delay(10);
	PORTB &= ~(1 << PINB2);
	delay(10);	
	send_recieve_spi_byte(reg);
	delay(10);	
	int i;
	for(i = 0; i < byte_count; i++) {
		if(mode == R && reg != W_TX_PAYLOAD) { // payload is on top level, uses R mode!
			ret[i] = send_recieve_spi_byte(NOP); // Loeme välja 
			delay(10);		
		} else {
			send_recieve_spi_byte(package[i]);
			delay(10);		
		}
	}
	PORTB |= (1 << PINB2);
	return ret;
	
}

void nrf24l01_transmit(uint8_t* payload, uint8_t count) {
	nrf24l01_communicate(R, FLUSH_TX, payload, 0);
	nrf24l01_communicate(R, W_TX_PAYLOAD, payload, count);
	delayms(1);
	PORTB |= (1 << PINB1);
	delay(10);
	PORTB &= ~(1 << PINB1);
	delayms(1);
}

void nrf24l01_reset() {
	delay(10);	
	PORTB &= ~(1 << PINB2);
	delay(12);	
	uint8_t val[1];
	val[0] = 0x70;
	nrf24l01_communicate(W, STATUS, val, 1);
	delay(12);
	PORTB |= (1 << PINB2);

}

uint8_t* nrf24l01_recieve(uint8_t count) {
	PORTB |= (1 << PINB1);
	delayms(20);
	PORTB &= ~(1 << PINB1);
	uint8_t* ret;
	ret = nrf24l01_communicate(R, R_RX_PAYLOAD, ret, count);
	return ret;
}

void initialize_nrf24l01() {
    //ce csn
    DDRB = (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB5);

    //CSN Kõrgeks
    PORTB |= (1 << PINB2);
	PORTB &= ~(1 << PINB1);

    SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0);
	
}


#endif /* NRF24L01_H_ */