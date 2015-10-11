#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "usb_serial.h"
#include "sprite.h"
#include "assignment2_helper.h"

void draw_centred(unsigned char y, char* string) {
    // Draw a string centred in the LCD when you don't know the string length
    unsigned char l = 0, i = 0;
    while (string[i] != '\0') {
        l++;
        i++;
    }
    char x = 42-(l*5/2);
    draw_string((x > 0) ? x : 0, y, string);
}

void wait_for_button1(void){
    while(!(PINF & 0b01000000));
    while(PINF & 0b01000000);
    send_debug_string("pressed 1!");
}

void wait_for_button2(void){
    while(!(PINF & 0b00100000));
    while(PINF & 0b00100000);
    send_debug_string("pressed 2!");
}

int wait_for_any_button(void){
    while(!(PINF & 0b01000000) & !(PINF & 0b00100000));

    if(PINF & 0b01000000){
        while(PINF & 0b01000000);
        send_debug_string("button 1!");
        _delay_ms(50);
        return 1;
    }
    else if(PINF & 0b00100000){;
        while(PINF & 0b00100000);
        _delay_ms(50);
        send_debug_string("button 2!");
        return 2;
    }
}

void send_line(char* string) {
    // Send all of the characters in the string
    unsigned char char_count = 0;
    while (*string != '\0') {
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }

    // Go to a new line (force this to be the start of the line)
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}

void send_debug_string(char* string) {
     // Send the debug preamble...
     usb_serial_write("DBG: ", 5);

     // Send all of the characters in the string
     unsigned char char_count = 0;
     while (*string != '\0') {
         usb_serial_putchar(*string);
         string++;
         char_count++;
     }

     // Go to a new line (force this to be the start of the line)
     usb_serial_putchar('\r');
     usb_serial_putchar('\n');
 }

void init_hardware(void){
    set_clock_speed(CPU_8MHz);
    LCDInitialise(LCD_DEFAULT_CONTRAST);
    usb_init();
    sei();
}

void usb_wait(void){
    draw_string(0, 0, "pls connect");
    show_screen();
    while(!usb_configured() || !usb_serial_get_control());
    clear_screen();
    send_line("Debugger initialised. Debugging strings will appear below:");
}

void draw_menu(int cur_selection){
    draw_string(0, 0, "Please select a level");
    draw_string(0, 10, "Level 1");
    draw_string(0, 20, "Level 2");
    draw_string(0, 30, "Level 3");
    if(cur_selection > 0 & cur_selection < 4){
        draw_string(40, cur_selection * 10, "<--");
    }
}

void draw_status(int level, int score){
    char level_string[3];
    char score_string[5];

    sprintf(level_string, "L: %d", level);
    sprintf(score_string, "S: %d", score);

    draw_string(0, 0, level_string);
    draw_string(25, 0, score_string);
}