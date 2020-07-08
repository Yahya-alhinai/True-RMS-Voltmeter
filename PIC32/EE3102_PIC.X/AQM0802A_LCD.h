// https://github.com/Costava/AQM0802A-LCD-PIC32

#ifndef AQM0802A_LCD_H
#define	AQM0802A_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

// Send command cmd
void lcd_cmd(char cmd);

// Initialize the LCD
// Must call this before other functions
void lcd_init();

// Set cursor to the x and y position
// x and y start at 0
void lcd_set_cursor(int x, int y);

// Print char c to current cursor position
// Cursor position advances to next position
// Cursor will not wrap to next line
void lcd_print_char(char c);

// Print string s starting at current cursor position
// Cursor position at end of function is next position after last printed character
// Cursor will not wrap to next line
void lcd_print_str(const char* s);

// Blocking delay for roughly x milliseconds
void lcd_delay_ms(unsigned int x);

// Blocking delay for roughly x microseconds
void lcd_delay_us(unsigned int x);

#ifdef __cplusplus
}
#endif

#endif// AQM0802A_LCD_H
