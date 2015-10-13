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

void init_timer3(void){
    TCCR3B = (1 << WGM32);
    OCR3A = 1500;
    TIMSK3 = (1 << OCIE3A);
    TCCR3B |= (1 << CS32) | (1 << CS30);
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
    draw_string(0, 0, "Please select a level");
    draw_string(0, 10, "Level 1");
    draw_string(0, 20, "Level 2");
    draw_string(0, 30, "Level 3");
    if(cur_selection > 0 & cur_selection < 4){
        draw_string(40, cur_selection * 10, "<--");
    }
}

void draw_status(int level, int score){
    char level_string[4];
    char score_string[6];

    sprintf(level_string, "L: %d", level);
    sprintf(score_string, "S: %d", score);

    draw_string(0, 0, level_string);
    draw_string(50, 0, score_string);
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
    // int sprite1_left = sprite1.x - sprite1.width/2;
    // int sprite1_right = sprite1.x + sprite1.width/2;
    // int sprite1_top = sprite1.y - sprite1.height/2;
    // int sprite1_bottom = sprite1.y + sprite1.height/2;

    // int sprite2_left = sprite2.x - sprite2.width/2;
    // int sprite2_right = sprite2.x + sprite2.width/2;
    // int sprite2_top = sprite2.y - sprite2.height/2;
    // int sprite2_bottom = sprite2.y + sprite2.height/2;

    // if((sprite1_left >= sprite2_left && sprite1_left <= sprite2_right)&&
    //    (sprite1_top >= sprite2_top && sprite1_top <= sprite2_bottom)  ||


    //    (sprite2_left >= sprite1_left && sprite2_left <= sprite1_right)&&
    //    (sprite2_top >= sprite1_top && sprite2_top <= sprite1_bottom)){
    //     return 1;
    // }

    // else{
    //     return 0;
    // }
    if(((sprite1.x >= sprite2.x && sprite1.x <= sprite2.x + sprite2.width)&&
       (sprite1.y >= sprite2.y && sprite1.y <= sprite2.y + sprite2.height))||


       ((sprite2.x >= sprite1.x && sprite2.x <= sprite1.x + sprite1.width)&&
       (sprite2.y >= sprite1.y && sprite2.y <= sprite1.y + sprite1.height))){
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
//     int collision = 0;

//     collision = isCollision(character, happy);
//     collision = collision || isCollision(character, angry);
//     collision = collision || isCollision(character, silly);

//     return collision;
// }

// void init_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3){
//     init_sprite(sprite1, rand(), 10, 16, 16, happy_bm);
//     init_sprite(sprite2, rand(), 10, 16, 16, angry_bm);
//     init_sprite(sprite3, rand(), 10, 16, 16, silly_bm);
// }

// void draw_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3){
//     draw_sprite(sprite1);
//     draw_sprite(sprite2);
//     draw_sprite(sprite3);
// }