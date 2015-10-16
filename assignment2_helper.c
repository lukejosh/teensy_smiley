#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "usb_serial.h"
#include "sprite.h"
#include "assignment2_helper.h"

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

unsigned char character_bm[8] = {
    0b00111100,
    0b01000010,
    0b10101001,
    0b10101001,
    0b10000101,
    0b10111001,
    0b01000010,
    0b00111100
};

Sprite happy;
Sprite angry;
Sprite silly;

void menu(void);

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
    TCCR1B &= ~((1<<WGM13));
    TCCR1B &= ~((1<<WGM12));
    TCCR1A &= ~((1<<WGM11));
    TCCR1A &= ~((1<<WGM10));
    TCCR1B |= ((1 << CS12) | (1 << CS10));
    TCCR1B &= ~(1 << CS11);

    
    //TCCR3B = (1 << WGM32);
    //OCR3A = 65000;
    //TIMSK3 = (1 << OCIE3A);
    //TCCR3B = (1 << CS32) | (1 << CS30);
}

void init_timer3(int speed){
    TCCR3B = (1 << WGM32);
    OCR3A = speed;
    TIMSK3 = (1 << OCIE3A);
    TCCR3B |= (1 << CS32) | (1 << CS30);
}

void turnoff_timer3(void){
    TCCR3B &= ~(1 << CS32);
    TCCR3B &= ~(1 << CS30);
}

void turnoff_all_interrupts(void){
    EICRA &= ~(1 << ISC01);
    EIMSK &= ~(1 << INT0);
    PCICR &= ~(1 << PCIE0);
    PCMSK0 &= ~(1 << PCINT1);
    TCCR3B &= ~(1 << WGM32); 
    TIMSK3 &= ~(1 << OCIE3A);
}

void set_fall_speed(int speed){
    if(speed == 1){
        OCR3A = 2500;
    }
    else if (speed == 2){
        OCR3A = 2000;
    }
    else if(speed == 3)
        OCR3A = 1500;
}

void init_right_interrupt(void){
    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);
}

void init_left_interrupt(void){
    PCICR = (1 << PCIE0);
    PCMSK0 = (1 << PCINT1);
}

void usb_wait(void){
    draw_string(0, 0, "pls connect");
    show_screen();
    while(!usb_configured() || !usb_serial_get_control());
    clear_screen();
    send_line("Debugger initialised. Debugging strings will appear below:");
}

void draw_menu(int cur_selection){
    draw_string(0, 0, "Select a level");
    draw_string(0, 10, "Level 1");
    draw_string(0, 20, "Level 2");
    draw_string(0, 30, "Level 3");
    if(cur_selection > 0 & cur_selection < 4){
        draw_string(40, cur_selection * 10, "<--");
    }
}

void draw_status(int liv, int sco){
    char lives_string[10];
    char score_string[10];

    sprintf(lives_string, "L: %d.", liv);
    sprintf(score_string, "S: %d.", sco);

    draw_string(0, 0, lives_string);
    draw_string(42, 0, score_string);
    draw_line(0, 9, 83, 9);
}

int check_valid_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3){
    int sprite1_left = sprite1.x - sprite1.width/2;
    int sprite1_right = sprite1.x + sprite1.width/2;
    int sprite2_left = sprite2.x - sprite2.width/2;
    int sprite2_right = sprite2.x + sprite2.width/2;
    int sprite3_left = sprite3.x - sprite3.width/2;
    int sprite3_right = sprite3.x + sprite3.width/2;

    if ((sprite1_left + 5 >= sprite2_left && sprite1_left - 5 <= sprite2_right)||
        (sprite1_left + 5 >= sprite3_left && sprite1_left - 5 <= sprite3_right)||
        (sprite2_left + 5 >= sprite1_left && sprite2_left - 5 <= sprite1_right)||
        (sprite2_left + 5 >= sprite3_left && sprite2_left - 5 <= sprite3_right)||
        (sprite3_left + 5 >= sprite2_left && sprite3_left - 5 <= sprite2_right)||
        (sprite3_left + 5 >= sprite1_left && sprite3_left - 5 <= sprite1_right)
        ){
        send_debug_string("not valid");
        return 0;
    }
    else{
        send_debug_string("valid!");
        return 1;
    }
}

void loop_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3){
    if (sprite1.y == 40){
        sprite1.x = 10;
    }
    if (sprite2.y == 40){
        sprite2.y = 10;
    }
    if (sprite3.y == 40){
        sprite3.y = 10;
    }
}

int isCollision(Sprite sprite1, Sprite sprite2){
    if ((sprite1.is_visible && sprite2.is_visible)&&
       ((sprite1.x >= sprite2.x && sprite1.x <= sprite2.x + sprite2.width)&&
       ((sprite1.y + sprite1.height) > (sprite2.y)) && ((sprite1.y) < (sprite2.y + sprite2.height)))||
       ((sprite2.x >= sprite1.x && sprite2.x <= sprite1.x + sprite1.width)&&
       ((sprite2.y + sprite2.height) > (sprite1.y)) && ((sprite2.y) < (sprite1.y + sprite1.height)))){
        return 1;
    }

    else{
        return 0;
    }
}

int check_collisions(Sprite character, Sprite happy, Sprite angry, Sprite silly){
    if(isCollision(character, happy) == 1){
        return 1;
    }
    else if (isCollision(character, angry) == 1){
        return 2;
    }

    else if (isCollision(character, silly) == 1){
        return 3;
    }

    else{
        return 0;
    }
}

void init_poten(void){
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

void turn_poten_off(void){
    ADCSRA &= ~(1 << ADSC);
}

uint16_t adc_read(uint8_t ch){
    ch &= 0b00000111;
    ADMUX = (ADMUX & 0xF8)|ch;
    ADCSRA |= (1<<ADSC); 
    while(ADCSRA & (1<<ADSC));

 
    return (ADC);
}

int get_x_position_from_poten(uint16_t poten){
    double dec = poten/1023.0 * 76.0;;
    return (round(dec));
}

void init_level(int l){
    level = l;
    score = 0;
    lives = 3;

    init_timer3(2500);
}

void end_level(void){
    turnoff_all_interrupts();
    clear_screen();
    draw_centred(10, "Game over! :(");
    draw_centred(20, "Play again?");
    draw_string(0, 40, "Y");
    draw_string(75, 40, "N");
    show_screen();

    int continue_selection = wait_for_any_button();
    if (continue_selection == 1){
        clear_screen();
        menu();
    }

    else{
        clear_screen();
        draw_centred(20, "Goodbye!");
        show_screen();
    }

    show_screen();
}

void init_all_sprites(void){
    init_sprite(&happy, rand() % 67, 10, 16, 16, happy_bm);
    init_sprite(&angry, rand() % 67, 10, 16, 16, angry_bm);
    init_sprite(&silly, rand() % 67, 10, 16, 16, silly_bm);
    init_sprite(&character, 42, 40, 8, 8, character_bm);
}

void make_enemies_valid(void){
    int valid = check_valid_faces(happy, angry, silly);
    while(!valid){
        happy.x = rand() % 67;
        angry.x = rand() % 67;
        silly.x = rand() % 67;
        valid = check_valid_faces(happy, angry, silly);
    }
}

void draw_all_sprites(void){
    draw_sprite(&happy);
    draw_sprite(&silly);
    draw_sprite(&angry);
    draw_sprite(&character);
}