/* 
 * File:   setups.h
 * Author: Emma
 *
 * Created on April 24, 2019, 2:32 PM
 */

#ifndef SETUPS_H
#define	SETUPS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
void system_reg_unlock(void);
void system_reg_lock(void);
void setup_lcd(void);
void lcd_cmd(char command);
void lcd_printChar(char myChar);
void lcd_printStr(const char *s);
void lcd_init(void);
void initSPI(void);
void initUART(void);
void initI2C(void);
void setupLCD(void);
void setupFTDI(void);
void mainSetup(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SETUPS_H */

