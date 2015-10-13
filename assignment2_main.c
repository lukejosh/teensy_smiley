#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "usb_serial.h"
#include "sprite.h"
#include "assignment2_helper.h"

#define FREQUENCY 8000000.0
/*
RIGHT BUTTON: F5
LEFT  BUTTON: F6m
SWITCH THING: 
SCREEN      :
POTEN 1     :
POTEN 2     :
LED1        :
LED2        :
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
Sprite character;
int level;
int lives = 3;
int score = 0;
int speed = 0; //in ticks!

void level1(void);
void level2(void);
void level3(void);
void to_level(int level);

void level1(void){
    init_timer3();
    init_right_interrupt();
    init_left_interrupt();
    clear_screen();
    srand(TCNT1);
    init_sprite(&happy, rand() % 70, 10, 16, 16, happy_bm);
    init_sprite(&angry, rand() % 70, 10, 16, 16, angry_bm);
    init_sprite(&silly, rand() % 70, 10, 16, 16, silly_bm);
    init_sprite(&character, 42, 40, 8, 8, character_bm);

    int valid = check_valid_faces(happy, angry, silly);
    while(!valid){
        happy.x = rand() % 70;
        angry.x = rand() % 70;
        silly.x = rand() % 70;
        valid = check_valid_faces(happy, angry, silly);
    }
    draw_sprite(&happy);
    draw_sprite(&silly);
    draw_sprite(&angry);
    draw_sprite(&character);
    level = 1;
    draw_status(level, score);
    show_screen();

    while(lives != 0 || score != 20){
        //move sprites
        //check for collisions | wraparounds
        //send_debug_string("reduce 1");
    }
}

void level2(void){
    send_debug_string("level 2");
    clear_screen();
    level = 2;
    draw_status(level, score);
    show_screen();
}

void level3(void){
    send_debug_string("level 3");
    clear_screen();
    level = 3;
    draw_status(level, score);
    show_screen();
}

void to_level(int level){
    switch(level){
        case 1:
            level1();
            break;

        case 2:
            level2();
            break;

        case 3:
            level3();
            break;
    }
}

void startup(void){
    clear_screen();
    draw_centred(0, "CAB202");
    draw_centred(10, "Assignment 2");
    draw_centred(20, "Luke Josh");
    draw_centred(30, "N9155554");
    draw_centred(40, "Press to cont.");
    show_screen();
    wait_for_button1();
    clear_screen();
    send_debug_string("startup finished");
}

void menu(void){
    int cur_selected;

    send_debug_string("menu starting");
    int selected = 0;
    draw_menu(cur_selected);
    show_screen();

    while(!selected){
        int button = wait_for_any_button();
        if(button == 2){
            clear_screen();
            cur_selected++;
            if(cur_selected == 4){
                cur_selected = 1;
            }
            draw_menu(cur_selected);
            show_screen();
        }
        else if(button == 1){
            send_debug_string("load level");
            selected = 1;
        }
    }
    send_debug_string("menu finished!");
    to_level(cur_selected);
}

int main(void){
    init_hardware();
    usb_wait();
    startup();
    menu();
    return 0;
}

ISR(TIMER3_COMPA_vect){
    clear_screen();
    happy.y = happy.y + 1;
    angry.y = angry.y + 1;
    silly.y = silly.y + 1;
    if (happy.y == 48){
        happy.y = 10;
        angry.y = 10;
        silly.y = 10;

        happy.x = rand() % 70;
        angry.x = rand() % 70;
        silly.x = rand() % 70;

        int valid = check_valid_faces(happy, angry, silly);

        while(!valid){
            happy.x = rand() % 70;
            angry.x = rand() % 70;
            silly.x = rand() % 70;
            valid = check_valid_faces(happy, angry, silly);
        }
    }

    char buff[5];
    int coll = check_collisions(character, happy, angry, silly);
    sprintf(buff, "C: %d", coll);
    send_debug_string(buff);

    draw_sprite(&happy);
    draw_sprite(&angry);
    draw_sprite(&silly);
    draw_sprite(&character);
    draw_status(level, score);
    show_screen();//screen refresh timer
}

ISR(INT0_vect){ //right dpad
    //while(PINB & 0b00000001);
    if(character.x < 76){
        send_debug_string("RIGHT");
        character.x++;
    }
}

ISR(PCINT0_vect){
    if (PINB & 0b00000010){
        send_debug_string("LEFT");
        //while(PINB & 0b00000010);
        if(character.x > 0){
            character.x--;
        }
    }
}