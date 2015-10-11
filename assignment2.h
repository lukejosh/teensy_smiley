void send_debug_string(char* string);
void draw_centred(unsigned char y, char* string);
void send_line(char* string);
void recv_line(char* buff, unsigned char max_length);

#define BYTES_PER_FACE 32;
#define FREQUENCY 8000000.0
#define PRESCALER_0 1024.0
#define PRESCALER_1 1024.0