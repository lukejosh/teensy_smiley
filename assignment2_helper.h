void draw_centred(unsigned char y, char* string);
void wait_for_button1(void);
void wait_for_button2(void);
int wait_for_any_button(void);
void send_line(char* string);
void send_debug_string(char* string);
void init_hardware(void);
void usb_wait(void);
void draw_menu(int cur_selection);
void draw_status(int level, int score);