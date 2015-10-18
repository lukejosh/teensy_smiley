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
    0b11111111, 0b11111111,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10011000, 0b00011001,
    0b10011000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10010000, 0b00001001,
    0b10010000, 0b00001001,
    0b10001000, 0b00010001,
    0b10000111, 0b11100001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b11111111, 0b11111111
};

unsigned char angry_bm[32] = {
    0b11111111, 0b11111111,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10001000, 0b00010001,
    0b10000100, 0b00100001,
    0b10000010, 0b01000001,
    0b10000000, 0b00000001,
    0b10000011, 0b11000001,
    0b10000100, 0b00100001,
    0b10001000, 0b00010001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b11111111, 0b11111111
};

unsigned char silly_bm[32] = {
    0b11111111, 0b11111110,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10011000, 0b00000001,
    0b10011000, 0b00000001,
    0b10000000, 0b00011001,
    0b10000000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000011, 0b11110001,
    0b10000000, 0b11000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b11111111, 0b11111111
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
volatile int level;
volatile int score;
volatile int lives;
volatile int continue_level;

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
}

void wait_for_button2(void){
    while(!(PINF & 0b00100000));
    while(PINF & 0b00100000);
}

int wait_for_any_button(void){
    while(!(PINF & 0b01000000) & !(PINF & 0b00100000));

    if(PINF & 0b01000000){
        while(PINF & 0b01000000);
        _delay_ms(50);
        return 1;
    }
    else if(PINF & 0b00100000){;
        while(PINF & 0b00100000);
        _delay_ms(50);
        return 2;
    }

    else{
        return 0;
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

// void send_debug_string(char* string) {
//      // Send the debug preamble...
//      usb_serial_write("DBG: ", 5);

//      // Send all of the characters in the string
//      unsigned char char_count = 0;
//      while (*string != '\0') {
//          usb_serial_putchar(*string);
//          string++;
//          char_count++;
//      }

//      // Go to a new line (force this to be the start of the line)
//      usb_serial_putchar('\r');
//      usb_serial_putchar('\n');
// }

void init_hardware(void){
    set_clock_speed(CPU_8MHz);
    LCDInitialise(LCD_DEFAULT_CONTRAST);
    usb_init();
    sei();
    TCCR4B |= ((1<< CS40) | (1 << CS41) | (1 << CS42) | (1 << CS43));
    // TCCR1B |= ((1 << CS12) | (1 << CS10));
    // TCCR1B &= ~(1 << CS11);

    
    //TCCR3B = (1 << WGM32);
    //OCR3A = 65000;
    //TIMSK3 = (1 << OCIE3A);
    //TCCR3B = (1 << CS32) | (1 << CS30);
}

void init_timer1(){
    TCCR1B = (1 << WGM12);
    OCR1A = 400;
    TIMSK1 = (1 << OCIE1A);
    TCCR1B |= (1 << CS12) | (1 << CS10);
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
    TCCR1B &= ~(1 << CS12);
    TCCR1B &= ~(1 << CS10);
    TCCR3B &= ~(1 << CS32);
    TCCR3B &= ~(1 << CS30);
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
    if((cur_selection > 0) & (cur_selection < 4)){
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
    int sprite1_left = sprite1.x;
    int sprite1_right = sprite1.x + sprite1.width;
    int sprite2_left = sprite2.x;
    int sprite2_right = sprite2.x + sprite2.width;
    int sprite3_left = sprite3.x;
    int sprite3_right = sprite3.x + sprite3.width;

    if ((sprite1_left + 5 >= sprite2_left && sprite1_left - 5 <= sprite2_right)||
        (sprite1_left + 5 >= sprite3_left && sprite1_left - 5 <= sprite3_right)||
        (sprite2_left + 5 >= sprite1_left && sprite2_left - 5 <= sprite1_right)||
        (sprite2_left + 5 >= sprite3_left && sprite2_left - 5 <= sprite3_right)||
        (sprite3_left + 5 >= sprite2_left && sprite3_left - 5 <= sprite2_right)||
        (sprite3_left + 5 >= sprite1_left && sprite3_left - 5 <= sprite1_right)
        ){
        return 0;
    }
    else{
        return 1;
    }
}

int isCollision_lev3(Sprite sprite1, Sprite sprite2){

    if (
       (
           //(sprite1.is_visible && sprite2.is_visible)&&
           (sprite1.x >= sprite2.x - 5 && sprite1.x <= sprite2.x + sprite2.width + 5)&&
           (sprite1.y + sprite1.height >= sprite2.y - 5 && sprite1.y <= sprite2.y + sprite2.height + 5)
       )

       ||

       (
           //(sprite2.is_visible && sprite1.is_visible)&&
           (sprite2.x >= sprite1.x - 5 && sprite2.x <= sprite1.x + sprite1.width + 5)&&
           (sprite2.y + sprite2.height >= sprite1.y - 5 && sprite2.y <= sprite1.y + sprite1.height + 5)
       )
       ){

       // ((sprite1.is_visible && sprite2.is_visible)&&
       // ((sprite2.x >= sprite1.x && sprite2.x <= sprite1.x + sprite1.width + 5)&&
       // ((sprite2.y + sprite2.height) > (sprite1.y) - 5) && ((sprite2.y) < (sprite1.y + sprite1.height))))){
        return 1;
    }

    else{
        return 0;
    }
}

int check_valid_faces_level3(Sprite sprite1, Sprite sprite2, Sprite sprite3, Sprite character){
    if(isCollision_lev3(sprite1, sprite2) == 1 || isCollision_lev3(sprite1, sprite3) == 1   || isCollision_lev3(sprite1, character) == 1||
       isCollision_lev3(sprite2, sprite3) == 1 || isCollision_lev3(sprite2, character) == 1 || isCollision_lev3(sprite3, character) == 1){
        return 0;
    }
    else{
        return 1;
    }
}

int testCollision(Sprite sprite1, Sprite sprite2){
    if(((sprite1.is_visible && sprite2.is_visible) &&
        
       ((sprite1.x >= sprite2.x) && (sprite1.x <= sprite2.x + sprite2.width + 2)) &&
       ((sprite1.y >= sprite2.y - sprite1.height) && (sprite1.y  <= sprite2.y + sprite2.height)))||

       (((sprite2.x >= sprite1.x) && (sprite2.x <= sprite1.x + sprite1.width + 2)) &&
       ((sprite2.y >= sprite1.y - sprite2.height) && (sprite2.y  <= sprite1.y + sprite1.height)))){
        return 1;
    }
    else{
        return 0;
    }
}

int testCollision1(Sprite sprite1, Sprite sprite2){
    int sprite1_newx = sprite1.x + sprite1.dx;
    int sprite1_newy = sprite1.y + sprite1.dy;
    int sprite2_newx = sprite2.x + sprite2.dx;
    int sprite2_newy = sprite2.y + sprite2.dy;

    if( (sprite1.is_visible && sprite2.is_visible) &&
        ((((sprite1_newx >= sprite2_newx) &&
    (sprite1_newx <= sprite2_newx + sprite2.width - 1))
    &&
    ((sprite1_newy >= sprite2_newy - sprite1.height + 1) &&
    (sprite1_newy <= sprite2_newy + sprite2.height - 1)))
    ||
    (((sprite2_newx >= sprite1_newx) &&
    (sprite2_newx <= sprite1_newx + sprite1.width - 1))
    &&
    ((sprite2_newy >= sprite1_newy - sprite2.height + 1) &&
    (sprite2_newy <= sprite1_newy + sprite1.height - 1))))){

        return 1;

    }

    else{
        return 0;
    }
}

int isCollision(Sprite sprite1, Sprite sprite2){

    if ((sprite1.is_visible && sprite2.is_visible)&&

       (((sprite1.x >= sprite2.x && sprite1.x <= sprite2.x + sprite2.width)&&
       ((sprite1.y + sprite1.height) > (sprite2.y)) && ((sprite1.y) < (sprite2.y + sprite2.height)))||
       ((sprite2.x >= sprite1.x && sprite2.x <= sprite1.x + sprite1.width)&&
       ((sprite2.y + sprite2.height) > (sprite1.y)) && ((sprite2.y) < (sprite1.y + sprite1.height))))){
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

    else if (isCollision(happy, angry) == 1 || isCollision(happy, silly) == 1 || isCollision(angry, silly) == 1){
        return 4;
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
    char *output;
    if(score == 20){
        output = "You win!!!";
    }
    else{
        output = "Game over! :(";
    }
    clear_screen();
    draw_centred(10, output);
    draw_centred(20, "Play again?");
    draw_string(0, 40, "Y");
    draw_string(75, 40, "N");
    show_screen();

    int continue_selection = wait_for_any_button();
    if (continue_selection == 1){
        clear_screen();
        menu();
    }

    else if (continue_selection == 2){
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

float rand_dir(void){
    float dir = rand() % 3;

    if (dir == 2){
        dir = -1;
    }
    return dir;
}

void init_all_sprites_level3(void){
    init_sprite(&happy, rand() % 67, (rand() % 22) + 10, 16, 16, happy_bm);
    init_sprite(&angry, rand() % 67, (rand() % 22) + 10, 16, 16, angry_bm);
    init_sprite(&silly, rand() % 67, (rand() % 22) + 10, 16, 16, silly_bm);
    init_sprite(&character, 42, 40, 8, 8, character_bm);

    happy.dx = rand_dir();
    happy.dy = rand_dir();
    angry.dx = rand_dir();
    angry.dy = rand_dir();
    silly.dx = rand_dir();
    silly.dy = rand_dir();

    while((happy.dx == 0 && happy.dy == 0)){
        happy.dx = rand_dir();
        happy.dy = rand_dir();
    }
    while((angry.dx == 0 && angry.dy == 0)){
        angry.dx = rand_dir();
        angry.dy = rand_dir();
    }
    while((silly.dx == 0 && silly.dy == 0)){
        silly.dx = rand_dir();
        silly.dy = rand_dir();
    }
}

void increment_all_level3(void){
    if(happy.dx + happy.dy == 1){
        happy.x = happy.x + happy.dx * 2;
        happy.y = happy.y + happy.dy * 2;
    }
    else{
        happy.x = happy.x + happy.dx;
        happy.y = happy.y + happy.dy;    
    }

    if(angry.dx + angry.dy == 1){
        angry.x = angry.x + angry.dx * 2;
        angry.y = angry.y + angry.dy * 2;
    }
    else{
        angry.x = angry.x + angry.dx;
        angry.y = angry.y + angry.dy;    
    }

    if(silly.dx + silly.dy == 1){
        silly.x = silly.x + silly.dx * 2;
        silly.y = silly.y + silly.dy * 2;
    }
    else{
        silly.x = silly.x + silly.dx;
        silly.y = silly.y + silly.dy;    
    }
}

int colliding_x(Sprite sprite1, Sprite sprite2){
    if (((sprite1.x > sprite2.x) && (sprite1.x < sprite2.x + sprite2.width))
        ||
        ((sprite2.x > sprite1.x) && (sprite2.x < sprite1.x + sprite1.width))){
        return 1;
    }
    else{
        return 0;
    }
}

int colliding_y(Sprite sprite1, Sprite sprite2){
    if (((sprite1.y > sprite2.y) && (sprite1.y < sprite2.y + sprite2.height))
        ||
        ((sprite2.y > sprite1.y) && (sprite2.y < sprite1.y + sprite1.height))){
        return 1;
    }
    else{
        return 0;
    }
}

//                                 HAPPY(M)        SILLY(S)
void determine_new_direction(Sprite sprite1, Sprite sprite2, float *directions){
    int move_sprite;
    int stat_sprite;

    Sprite sprites[2];
    sprites[0] = sprite1;
    sprites[1] = sprite2;

    if (sprites[0].dy == 0 && sprites[1].dy == 0){
        directions[0] = sprites[0].dx * -1;
        directions[2] = sprites[1].dx * -1;

        if(sprites[0].y == sprites[1].y){
            directions[1] = 0;
            directions[3] = 0;
        }
        else if (sprites[0].y < sprites[1].y){
            directions[1] = -1;
            directions[3] =  1;
        }
        else{
            directions[1] =  1;
            directions[3] = -1;
        }
    }

    else if (sprites[0].dy == 0 || sprites[1].dy == 0){
        if(sprites[0].dy == 0){
            move_sprite = 1;
            stat_sprite = 0;
        }
        else{
            stat_sprite = 1;
            move_sprite = 0; //happy
        }

        if(colliding_x(sprites[0], sprites[1])){
            directions[(move_sprite * 2) + 1] = sprites[move_sprite].dy * -1;
            directions[(move_sprite * 2)] = sprites[move_sprite].dx;

            directions[(stat_sprite * 2) + 1] = sprites[move_sprite].dy * -1;
            directions[(stat_sprite * 2)] = sprites[stat_sprite].dx;
        }

        else{
            directions[stat_sprite * 2] = sprites[stat_sprite].dx * -1;

            directions[move_sprite * 2] = sprites[move_sprite].dx * -1;
            directions[move_sprite * 2 + 1] = sprites[move_sprite].dy;

            if(sprites[move_sprite].y < sprites[stat_sprite].y - sprites[stat_sprite].height/2){
                directions[stat_sprite * 2 + 1] = 1;
            }
            else{
                directions[stat_sprite * 2 + 1] = -1;
            }
        }
    }

    else if(sprites[0].dx == 0 || sprites[1].dx == 0){
        if(sprites[0].dx == 0){
            move_sprite = 1;
            stat_sprite = 0;
        }
        else{
            stat_sprite = 1;
            move_sprite = 0; //happy
        }

        if(colliding_y(sprites[0], sprites[1])){
            directions[(move_sprite * 2) + 1] = sprites[move_sprite].dy;
            directions[(move_sprite * 2)] = sprites[move_sprite].dx * -1;

            directions[(stat_sprite * 2) + 1] = sprites[move_sprite].dy;
            directions[(stat_sprite * 2)] = sprites[move_sprite].dx;
        }

        else{
            directions[stat_sprite * 2] = sprites[move_sprite].dx;
            directions[stat_sprite * 2 + 1] = sprites[stat_sprite].dy;

            directions[move_sprite * 2] = sprites[move_sprite].dx;
            directions[move_sprite * 2 + 1] = sprites[move_sprite].dy * -1;

            // if(sprites[move_sprite].y < sprites[stat_sprite].y - sprites[stat_sprite].height/2){
            //     directions[stat_sprite * 2 + 1] = 1;
            // }
            // else{
            //     directions[stat_sprite * 2 + 1] = -1;
            // }
        }
    }

    else{

        int t1 = testCollision3(sprites[0], sprites[1]);
        if (t1 == 1){
            directions[0] = sprites[0].dx * -1;
            directions[2] = sprites[1].dx * -1;

            directions[1] = sprites[0].dy;
            directions[3] = sprites[1].dy;
        }
        else if (t1 == 2){
            directions[1] = sprites[0].dy * -1;
            directions[3] = sprites[1].dy * -1;

            directions[0] = sprites[0].dx;
            directions[2] = sprites[1].dx;
        }
    }
}

int testCollision3(Sprite sprite1, Sprite sprite2){
    if (testCollision1(sprite1, sprite2)){
        sprite1.dx *= -1;
        sprite2.dx *= -1;

        if (testCollision1(sprite1, sprite2)){
            return 2;
        }
        else{
            return 1;
        }
    }
    else{
        return 0;
    }
}

void level3_collisions(void){
    // int t1 = testCollision3(happy, silly);
    // if (t1 == 1){
    //     happy.dx *= -1;
    //     silly.dx *= -1;
    // }
    // else if (t1 == 2){
    //     happy.dy *= -1;
    //     silly.dy *= -1;
    // }

    // int t2 = testCollision3(happy, angry);
    // if (t2 == 1){
    //     happy.dx *= -1;
    //     angry.dx *= -1;
    // }
    // else if(t2 == 2){
    //     happy.dy *= -1;
    //     angry.dy *= -1;
    // }

    // int t3 = testCollision3(angry, silly);
    // if (t3 == 1){
    //     angry.dx *= -1;
    //     silly.dx *= -1;
    // }
    // else if (t3 == 2){
    //     angry.dy *= -1;
    //     silly.dy *= -1;
    // }

    if(testCollision3(happy, silly)){
        float d1[4];
        determine_new_direction(happy, silly, d1);
        happy.dx = d1[0];
        happy.dy = d1[1];
        silly.dx = d1[2];
        silly.dy = d1[3];
    }

    if(testCollision3(happy, angry)){
        float d2[4];
        determine_new_direction(happy, angry, d2);
        happy.dx = d2[0];
        happy.dy = d2[1];
        angry.dx = d2[2];
        angry.dy = d2[3];
    }

    if(testCollision3(angry, silly)){
        float d3[4];
        determine_new_direction(angry, silly, d3);
        angry.dx = d3[0];
        angry.dy = d3[1];
        silly.dx = d3[2];
        silly.dy = d3[3];
    }


    if((happy.x <= 0 && happy.dx == -1) || (happy.x >= 67 && happy.dx == 1)){
        happy.dx *= -1;
    }

    if((silly.x <= 0 && silly.dx == -1) || (silly.x >= 67 && silly.dx == 1)){
        silly.dx *= -1;
    }

    if((angry.x <= 0 && angry.dx == -1) || (angry.x >= 67 && angry.dx == 1)){
        angry.dx *= -1;
    }

    if((happy.y <= 10 && happy.dy == -1) || (happy.y >= 32 && happy.dy == 1)){
        happy.dy *= -1;
    }

    if((angry.y <= 10 && angry.dy == -1) || (angry.y >= 32 && angry.dy == 1)){
        angry.dy *= -1;
    }

    if((silly.y <= 10 && silly.dy == -1) || (silly.y >= 32 && silly.dy == 1)){
        silly.dy *= -1;
    }
}

int level3_player_collision(void){
    if(testCollision(character, happy)){
        return 1;
    }
    else if (testCollision(character, angry)){
        return 2;
    }
    else if (testCollision(character, silly)){
        return 3;
    }
    else{
        return 0;
    }
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

void redraw_level3(void){
    int loopmax = 45;
    int loopcount = 0;
    int valid;

    if(!happy.is_visible){
        init_sprite(&happy, rand() % 67, (rand() % 22) + 10, 16, 16, happy_bm);
        happy.is_visible = 0;
        valid = check_valid_faces_level3(happy, angry, silly, character);

        while(!valid && loopcount < loopmax){
            init_sprite(&happy, rand() % 67, (rand() % 22) + 10, 16, 16, happy_bm);
            valid = check_valid_faces_level3(happy, angry, silly, character);
            happy.is_visible = 0;
            loopcount++;
        }
        if(valid){
            happy.is_visible = 1;
            happy.dx = rand_dir();
            happy.dy = rand_dir();
            while((happy.dx == 0 && happy.dy == 0)){
                happy.dx = rand_dir();
                happy.dy = rand_dir();
            }

        }
        loopcount = 0;

    }

    if(!angry.is_visible){
        init_sprite(&angry, rand() % 67, (rand() % 22) + 10, 16, 16, angry_bm);
        angry.is_visible = 0;
        valid = check_valid_faces_level3(happy, angry, silly, character);

        while(!valid && loopcount < loopmax){
            init_sprite(&angry, rand() % 67, (rand() % 22) + 10, 16, 16, angry_bm);
            angry.is_visible = 0;
            valid = check_valid_faces_level3(happy, angry, silly, character);
            loopcount++;
        }
        if(valid){
            angry.is_visible = 1;
            while((angry.dx == 0 && angry.dy == 0)){
                angry.dx = rand_dir();
                angry.dy = rand_dir();
            }
        }
    loopcount = 0;
    }

    if(!silly.is_visible){
        init_sprite(&silly, rand() % 67, (rand() % 22) + 10, 16, 16, silly_bm);
        silly.is_visible = 0;
        valid = check_valid_faces_level3(happy, angry, silly, character);

        while(!valid && loopcount < loopmax){
            init_sprite(&silly, rand() % 67, (rand() % 22) + 10, 16, 16, silly_bm);
            silly.is_visible = 0;
            valid = check_valid_faces_level3(happy, angry, silly, character);
            loopcount++;
        }
        if(valid){
            silly.is_visible = 1;
            while((silly.dx == 0 && silly.dy == 0)){
                silly.dx = rand_dir();
                silly.dy = rand_dir();
            }
        }
        loopcount = 0;
    }
}

void interrupt_level12(void){
    happy.y = happy.y + 2;
    angry.y = angry.y + 2;
    silly.y = silly.y + 2;
    if (happy.y >= 48 || (!happy.is_visible && !angry.is_visible && !silly.is_visible)){
        happy.y = 10;
        angry.y = 10;
        silly.y = 10;

        happy.is_visible = 1;
        angry.is_visible = 1;
        silly.is_visible = 1;

        happy.x = rand() % 67;
        angry.x = rand() % 67;
        silly.x = rand() % 67;

        make_enemies_valid();
    }

    int coll = check_collisions(character, happy, angry, silly);
    switch(coll){
        case (1):
            if (happy.is_visible){
                score += 2;
            }
            happy.is_visible = 0;
            break;

        case (2):
            if (angry.is_visible){
                lives -= 1;
            }
            angry.is_visible = 0;
            break;

        case (3):
            if(silly.is_visible){
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
            break;
    }

    if (level == 2){
        uint16_t result = adc_read(0);
        int new_x = get_x_position_from_poten(result);
        character.x = new_x;
    }

    clear_screen();
    draw_all_sprites();
    draw_status(lives, score);
    show_screen();

    if (lives == 0 || score == 20){
        continue_level = 0;
    }
}

void interrupt_level3(void){
    if(lives == 0 || score == 20){
        continue_level = 0;
        return;
    }
    redraw_level3();
    level3_collisions();
    increment_all_level3();
    int coll = level3_player_collision();
    switch(coll){
        case (1):
            if (happy.is_visible){
                score += 2;
            }
            happy.is_visible = 0;
            break;

        case (2):
            if (angry.is_visible){
                lives -= 1;
            }
            angry.is_visible = 0;
            break;

        case (3):
            speed++;
            if (silly.is_visible){
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
            break;
    }
    clear_screen();
    draw_all_sprites();
    draw_status(lives, score);
    show_screen();
}