void send_debug_string(char* string) {
     // Send the debug preamble...
    char buff[BUFF_LENGTH + 7];
    sprintf(buff, "[DBG @ %06.2f] ", get_system_time());
    usb_serial_write(buff, 16);

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

void recv_line(char* buff, unsigned char max_length) {
    // Loop over storing characters until the buffer is full or a newline character is received
    unsigned char char_count = 0;
    int16_t curr_char;
    do {
        // BLOCK here until a character is received
        do {
            curr_char = usb_serial_getchar();
        } while (curr_char == -1);

        // Add to the buffer if it wasn't a newline (accepts other gibberish that may not necessarily be a character!)
        if (curr_char != '\n' && curr_char != '\r') {
            buff[char_count] = curr_char;
            char_count++;
        }
    } while (curr_char != '\n' && curr_char != '\r' && char_count < max_length - 1);

    // Add the null terminator to the end of the string
    buff[char_count] = '\0';
}

void button_check(void){
	while 
}