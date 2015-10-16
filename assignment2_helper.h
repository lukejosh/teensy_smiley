extern volatile int lives;
extern volatile int score;
extern volatile int speed;
extern volatile int level;

void draw_centred(unsigned char y, char* string);
void wait_for_button1(void);
void wait_for_button2(void);
int wait_for_any_button(void);
void send_line(char* string);
void send_debug_string(char* string);
void init_hardware(void);
void usb_wait(void);
void draw_menu(int cur_selection);
void draw_status(int liv, int sco);
int check_valid_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3);
void init_timer3(int);
void loop_faces(Sprite sprite1, Sprite sprite2, Sprite sprite3);
void init_right_interrupt(void);
void init_left_interrupt(void);
int isCollision(Sprite sprite1, Sprite sprite2);
int check_collisions(Sprite character, Sprite happy, Sprite angry, Sprite silly);
void set_fall_speed(int speed);
void turnoff_timer3(void);
void turnoff_all_interrupts(void);
void init_poten(void);
void turn_poten_off(void);
uint16_t adc_read(uint8_t ch);
int get_x_position_from_poten(uint16_t poten);
void init_level(int l);
void end_level(void);