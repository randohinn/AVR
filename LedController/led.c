#define F_CPU 20000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define STRIP_LEN 59

uint8_t led_data[STRIP_LEN*3];

void one() {
	PORTC |= (1 << 5);
	_delay_us(0.80);
	PORTC &= ~(1<<5);
	_delay_us(0.45);
}

void zero() {
	PORTC |= (1 << 5);
	_delay_us(0.40);
	PORTC &= ~(1<<5);
	_delay_us(0.85);
}

void set_led(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
	uint8_t idx = led*3; 
	led_data[idx] = g;
	led_data[idx+1] = r;
	led_data[idx+2] = b;
}

void clear_strip() {
	int i;
	for(i = 0; i < sizeof(led_data); i++) {
		led_data[i] = 0;
	}
}

void set_strip(uint8_t r, uint8_t g, uint8_t b) {
	uint8_t i;
	for(i = 0; i < STRIP_LEN; i++) {
		set_led(i, r, g, b);
	}
}


void ledOut() {
	int i;
	for(i = 0; i < sizeof(led_data); i++) {
			int j;
			for(j = 7; j >= 0; j--) {
				if(led_data[i] & (1 << j)) {
					one();
				}
				else {
					zero();
				}
			}
		}
		
		_delay_us(60);
}

void go_red() {
	set_strip(255,0,0);
	ledOut();
}

void go_green() {
	set_strip(0,255,0);
	ledOut();
}

void go_blue() {
	set_strip(0,0,255);
	ledOut();
}

void fade_current_color(int start_r, int start_g, int start_b) {
	int r, g, b;
	float fade;
	for (fade = 1.0; fade >= 0.0; fade -= 0.0005)  {// 100 steps
		set_strip(start_r*fade, start_g*fade, start_b*fade);
		ledOut();
		_delay_us(10);
	}
	
	for (fade = 0.0; fade <= 1.0; fade += 0.0005)  {// 100 steps
		set_strip(start_r*fade, start_g*fade, start_b*fade);
		ledOut();
		_delay_us(10);
	}
	
	
}

int main() {
	DDRC |= (1 << 5);
	
	clear_strip();
	ledOut();
	
	int led;
	for(led = 0; led < STRIP_LEN; led++) {
		int val;
		for(val = 0; val < 255; val+=30) {
			set_led(led, val, val, val);
			ledOut();
		}
	}
	
	while(1) {
		fade_current_color(0,255,200);
		
	}
	return 1;
}