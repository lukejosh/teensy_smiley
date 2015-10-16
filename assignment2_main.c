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
int some_random;
volatile int lives = 3;
volatile int score = 0;
volatile int speed = 0;
volatile int continue_level;

void level1(void);
void level2(void);
void level3(void);
void to_level(int level);
void menu(void);
int main(void);

void level1(void){
    continue_level = 1;
    lives = 3;
    score = 0;
    init_timer3(2500);
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
    draw_status(lives, score);
    show_screen();

    send_debug_string("begin loop");
    while(continue_level);

    turnoff_all_interrupts();
    send_debug_string("out of loop");
    clear_screen();
    draw_centred(10, "Game over! :(");
    draw_centred(20, "Play again?");
    draw_string(0, 40, "Y");
    draw_string(75, 40, "N");
    show_screen();

    int continue_selection = wait_for_any_button();
    send_debug_string("passed wait!");

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

void level2(void){
    int continue_level = 1;
    level = 2;
    init_poten();
    send_debug_string("level 2");
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
    init_timer3(2500);
    while(continue_level);
}

void level3(void){
    send_debug_string("level 3");
    clear_screen();
    level = 3;
    draw_status(lives, score);
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
    init_hardware();
    usb_wait();
    ADMUX = 0b01000000;
    ADCSRA = 0b10000111;
    long result; //store conversion result
    char disp_buffer[32]; // store value to display on CD
    char int_buffer[10]; // store ascii conversion

    startup();
    menu();
    return 0;
}

ISR(TIMER3_COMPA_vect){//screen refresh timer
    happy.y = happy.y + 2;
    angry.y = angry.y + 2;
    silly.y = silly.y + 2;
    if (happy.y >= 48){
        happy.y = 10;
        angry.y = 10;
        silly.y = 10;

        happy.is_visible = 1;
        angry.is_visible = 1;
        silly.is_visible = 1;

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

    if (coll == 1){
        if (happy.is_visible){
            score += 2;
        }
        happy.is_visible = 0;
    }

    else if (coll == 2 && angry.is_visible){
        if (angry.is_visible){
            lives -= 1;
        }
            angry.is_visible = 0;
            char lbuff[5];
            sprintf(lbuff, "%d", lives);
            send_debug_string(lbuff);
    }

    else if (coll == 3){
        speed++;
        if (speed == 4){
            speed = 1;
        }

        if(speed == 1){
            init_timer3(2500);
        }
        else if (speed == 2){
            init_timer3(1750);
        }
        else if (speed ==  3){
            init_timer3(600);
        }

        silly.is_visible = 0;
    }

    if (level == 2){
        uint16_t result = adc_read(0);
        int new_x = get_x_position_from_poten(result);
        character.x = new_x;
        char buff[50];
        sprintf(buff, "POTEN: %d", new_x);
        send_debug_string(buff);
    }

    clear_screen();
    draw_sprite(&happy);
    draw_sprite(&angry);
    draw_sprite(&silly);
    draw_sprite(&character);
    draw_status(lives, score);
    show_screen();

    if (lives == 0 || score == 20){
        continue_level = 0;
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