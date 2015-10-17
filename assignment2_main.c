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

volatile int lives;
volatile int score;
volatile int speed;
volatile int level;
volatile int continue_level;

Sprite happy;
Sprite angry;
Sprite silly;
Sprite character;

void level1(void);
void level2(void);
void level3(void);
void to_level(int level);
void menu(void);
int main(void);

void level1(void){
    init_level(1);
    continue_level = 1;
    init_timer3(2500);
    init_right_interrupt();
    init_left_interrupt();
    clear_screen();

    init_all_sprites();
    make_enemies_valid();
    draw_all_sprites();

    level = 1;
    draw_status(lives, score);
    show_screen();

    while(continue_level);
    end_level();
}

void level2(void){
    init_level(2);
    continue_level = 1;
    init_poten();
    init_all_sprites();
    int valid = check_valid_faces(happy, angry, silly);
    make_enemies_valid();
    clear_screen();
    draw_all_sprites();
    show_screen();
    while(continue_level);
    end_level();
}

void level3(void){
    continue_level = 1;
    init_level(3);
    init_timer3(1500);
    init_all_sprites_level3();
    init_left_interrupt();
    init_right_interrupt();
    int valid = check_valid_faces_level3(happy, angry, silly, character);

    while(!valid){
        init_all_sprites_level3();
        valid = check_valid_faces_level3(happy, angry, silly, character);
    }
    clear_screen();
    draw_all_sprites();
    show_screen();
    while(continue_level);
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
    int cur_selected = 1;
    lives = 3;
    score = 20;

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
    lives = 3;
    score = 0;
    speed = 0;
    init_hardware();
    usb_wait();
    startup();
    srand(TCNT1);
    menu();
    return 0;
}

ISR(TIMER3_COMPA_vect){ //screen refresh timer
    if (level != 3){
        interrupt_level12();
    }
    else{
        interrupt_level3();
    }
}

ISR(INT0_vect){ //right dpad
    //while(PINB & 0b00000001);
    if(character.x < 76){
        send_debug_string("RIGHT");
        character.x+=2;
    }
}

ISR(PCINT0_vect){//left dpad
    if (PINB & 0b00000010){
        send_debug_string("LEFT");
        //while(PINB & 0b00000010);
        if(character.x > 0){
            character.x-=2;
        }
    } 
}