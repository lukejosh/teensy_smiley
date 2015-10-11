#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "usb_serial.h"
#include "assignment2.h"
#include "sprite.h"

#define FREQUENCY 8000000.0
#define PRESCALER_0 1024.0
#define PRESCALER_1 1024.0
/*
RIGHT BUTTON: F5
LEFT  BUTTON: F6
SWITCH THING: 
SCREEN		:
POTEN 1		:
POTEN 2		:
LED1		:
LED2		:
*/

unsigned char happy_bm[32] = {
	0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01011000, 0b00011010,
    0b10011000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10010000, 0b00001001,
    0b10010000, 0b00001001,
    0b10001000, 0b00010001,
    0b01000111, 0b11100010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
};

unsigned char angry_bm[32] = {
	0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b10001000, 0b00010001,
    0b10000100, 0b00100001,
    0b10000010, 0b01000001,
    0b10000000, 0b00000001,
    0b10000011, 0b11000001,
    0b10000100, 0b00100001,
    0b01001000, 0b00010010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
};

unsigned char silly_bm[32] = {
	0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b10011000, 0b00000001,
    0b10011000, 0b00000001,
    0b10000000, 0b00011001,
    0b10000000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000011, 0b11110001,
    0b01000000, 0b11000010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
};

void startup(void){
	
}

void menu(void){

}

int main(void){
	set_clock_speed(CPU_8MHz);
	LCDInitialise(LCD_DEFAULT_CONTRAST);
	Sprite happy;
	init_sprite(&happy, 34, 16, 16, 16, silly_bm);
	draw_sprite(&happy);
	_delay_ms(500);
	show_screen();
	return 0;
}