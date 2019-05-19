/*
 * main.c
 *
 *  Created on: 19. mai 2019
 *      Author: rando
 */

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
