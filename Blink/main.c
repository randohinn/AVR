/*
 * main.c
 *
 *  Created on: 19. mai 2019
 *      Author: rando
 */

#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>

int main() {
	DDRB = (1 << DDB1);
	while(1) {
		PORTB  ^= (1<<PINB1);
		_delay_ms(1000);
	}
	return 1;
}
