#include "xc.h"
#include "LCD.h"

void setup_lcd(void) {
    // Pin and MCU Settings
    //CLKDIVbits.RCDIV = 0;   // Divide clock by 1 (16 MHz)
    //PB3DIVbits.
    //AD1PCFG = 0x9FFF;       // Sets all pins to digital
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 0;
    _RB2 = 0;
    _RB3 = 0;
    
    // Setup for Baud Rate Generator
    IFS1bits.I2C2MIF = 0;   // Clear interrupt flag of I2C
    I2C2CONbits.I2CEN = 0;  // Disables I2C2 module for modification
    I2C2BRG = 0x9D;         // Sets baud rate to 16 kHz
    I2C2CONbits.I2CEN = 1;  // Re-enables I2C2 module
}

void delay_us(long msecs){
    T1CON = 0;
    IFS0bits.T1IF = 0;
    PR1 = 15;
    TMR1 = 0;
    T1CONbits.TON = 1;
    while(msecs > 0){
        while (!IFS0bits.T1IF);
        IFS0bits.T1IF = 0;
        msecs--;
    }
    T1CON = 0;
}

void lcd_cmd(char command) {
    I2C2CONbits.SEN = 1;    // Start Transmission
    while(I2C2CONbits.SEN);
    
    I2C2TRN = 0b01111100;   // Slave Address
    while(I2C2STATbits.TRSTAT);
    
    I2C2TRN = 0b00000000; // CO & RS bits
    while(I2C2STATbits.TRSTAT);
    
    I2C2TRN = command;      // Package
    while(I2C2STATbits.TRSTAT);
    
    I2C2CONbits.PEN = 1;    // End Transmission
    while(I2C2CONbits.PEN);
    
    delay_us(2);
}

void lcd_init(void) {
    char contrast = 0b100000;
    char contrast_1 = 0b01110000 | contrast;
    char contrast_2 = 0b01010100 | (contrast >> 4);
    
    delay_us(50000);       // Wait for Power Stable
    
    lcd_cmd(0b00111000);      // Function Set
    lcd_cmd(0b00111001);      // Function Set
    lcd_cmd(0b00010100);      // Internal OSC Freq
    
    lcd_cmd(contrast_1);      // Contrast Set
    lcd_cmd(contrast_2);      // Contrast Set
    
    lcd_cmd(0b01101100);      // Follower Control
    
    delay_us(200000);      // Wait for Power Stable
    
    lcd_cmd(0b00111000);      // Function Set
    lcd_cmd(0b00001111);      // Display Control
    lcd_cmd(0b00000001);      // Clear Display
}

void lcd_printChar(char myChar) {
    I2C2CONbits.SEN = 1;    // Start Transmission
    while(I2C2CONbits.SEN);
    
    I2C2TRN = 0b01111100;   // Slave Address
    while(I2C2STATbits.TRSTAT);
    
    I2C2TRN = 0b01000000; // CO & RS bits = 0 & 1
    while(I2C2STATbits.TRSTAT);
    
    I2C2TRN = myChar;      // Package
    while(I2C2STATbits.TRSTAT);
    
    I2C2CONbits.PEN = 1;    // End Transmission
    while(I2C2CONbits.PEN);
    
    delay_us(2);
}

void lcd_printStr(const char *s) {
    int count = 0;
    int i = 0;
    int j = 0;
    
    while(*(s+1) != 0b00000000){
        count++;
        s++;
    }
    while(i < count){
        s--;
        i++;
    }
    I2C2CONbits.SEN = 1;        // Start Transmission
    while(I2C2CONbits.SEN);
    
    I2C2TRN = 0b01111100;       // Slave Address
    while(I2C2STATbits.TRSTAT);
    
    while(j < count) {
        I2C2TRN = 0b11000000;   // CO = 1, RS = 1
        while(I2C2STATbits.TRSTAT);

        I2C2TRN = *s;           // Letter in string
        while(I2C2STATbits.TRSTAT);

        s++;
        j++;
    }
    
    I2C2TRN = 0b01000000;   // CO = 0, RS = 1
    while(I2C2STATbits.TRSTAT);

    I2C2TRN = *s;           // Last letter of string
    while(I2C2STATbits.TRSTAT);
    
    I2C2CONbits.PEN = 1;        // End Transmission
    while(I2C2CONbits.PEN);
    
    delay_us(2);
}

void lcd_setCursor(char x, char y) {
    char position = ((0x40 * y) + x) | 0x80;
    lcd_cmd(position);
}


