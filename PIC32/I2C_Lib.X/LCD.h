#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

void setup_lcd(void);
void delay_us(long msecs);
void lcd_cmd(char command);
void lcd_init(void);
void lcd_printChar(char myChar);
void lcd_printStr(const char *s);
void lcd_setCursor(char x, char y);

#define CONTRAST 0b100000
#define MAXLEN 8

#ifdef	__cplusplus
}
#endif

#endif