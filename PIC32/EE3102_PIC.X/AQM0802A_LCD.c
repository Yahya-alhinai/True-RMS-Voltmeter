// https://github.com/Costava/AQM0802A-LCD-PIC32

// Uses SDA2 and SCL2
// Make sure the defines are correct for what you are doing

#include "xc.h"
#include "p32xxxx.h"
#include <math.h>
#include "AQM0802A_LCD.h"

// Used to roughly determine delays
#define SYSCLK_FREQ (48000000)// Hz

// The value of I2C2CONbits.DISSLW set in lcd_init
// 1 for SCL_FREQ at 100 kHz. 0 for 400 kHz.
#define BUS_100KHZ 0
// SCL and PBCLK frequencies used to set the baud rate register
#define SCL_FREQ (800000.0)// Hz
#define PBCLK_FREQ (48000000.0)// Hz

// The contrast value to set in lcd_init
#define INIT_CONTRAST54 0b01// Bits 5 and 4 of contrast
#define INIT_CONTRAST3210 0b1111// Bits 3, 2, 1, and 0 of contrast

unsigned int lcd_ms_cycles = SYSCLK_FREQ / 1000;
unsigned int lcd_us_cycles = SYSCLK_FREQ / 1000000;

void lcd_cmd(char cmd) {
    I2C2CONbits.SEN = 1;// Initiate Start condition
    while (I2C2CONbits.SEN != 0);// Wait for end of start sequence
    
    // Check for I2C2STATbits.BCL == 1 here (bus collision) if something is wrong
    
    IFS1bits.I2C2MIF = 0;
    // 7 bit address + R/W bit. R/W bit always 0 for commands.
    I2C2TRN = 0b01111100;
    while (IFS1bits.I2C2MIF != 1);
    
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = 0b00000000;// Control byte
    while (IFS1bits.I2C2MIF != 1);
    
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = cmd;// Command byte
    while (IFS1bits.I2C2MIF != 1);
    
    IFS1bits.I2C2MIF = 0;

    I2C2CONbits.PEN = 1;// Initiate Stop condition
    while (I2C2CONbits.PEN != 0);// Wait for end of stop sequence
    
    lcd_delay_us(27);
}

void lcd_init() {
    I2C2CONbits.ON = 0;// Disable before set baud

    // Do not read/write SFRs right after clear ON when PBCLK is 1:1
    lcd_delay_us(1);
    
    // Make I2C2 pins digital
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    I2C2CONbits.A10M = 0;// 7-bit slave address
    I2C2CONbits.DISSLW = BUS_100KHZ;
    
    I2C2BRG = (int) ((1.0 / (2.0 * SCL_FREQ) - 104.0 / pow(10.0, 9.0)) * PBCLK_FREQ - 2.0);
    IFS1bits.I2C2MIF = 0;// Clear interrupt flag
    I2C2CONbits.ON = 1;// Enable I2C2
    
    lcd_delay_ms(50);
    
    lcd_cmd(0b00111000);// Function set
    lcd_cmd(0b00111001);// Function set
    lcd_cmd(0b00010100);// Internal OSC frequency
    lcd_cmd(0b01110000 | INIT_CONTRAST3210);// Contrast set C3-C0
    lcd_cmd(0b01010100 | INIT_CONTRAST54);// Ion, Bon, C5-C4
    lcd_cmd(0b01101100);// Follower control
    
    lcd_delay_ms(200);
    
    lcd_cmd(0b00111000);// Function set
    lcd_cmd(0b00001100);// Turn on display
    lcd_cmd(0b00000001);// Clear display
    lcd_delay_ms(1);
    lcd_cmd(0b00000010);// Return home (reset shifting))
}

void lcd_set_cursor(int x, int y) {
    char command = (0x40 * y + x) | 0b10000000;// MSB always set
    
    lcd_cmd(command);
}

void lcd_print_char(char c) {
    I2C2CONbits.SEN = 1;// Initiate Start condition
    while (I2C2CONbits.SEN != 0);// Wait for end of start sequence
    
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = 0b01111100;// 7-bit slave address + R/nW bit
    while (IFS1bits.I2C2MIF != 1);
    
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = 0b01000000;// Control byte. RS (the second MSB) is set.
    while (IFS1bits.I2C2MIF != 1);

    IFS1bits.I2C2MIF = 0;
    I2C2TRN = c;
    while (IFS1bits.I2C2MIF != 1);

    IFS1bits.I2C2MIF = 0;
    
    I2C2CONbits.PEN = 1;// Initiate Stop condition
    while (I2C2CONbits.PEN != 0);// Wait for end of stop sequence
}

void lcd_print_str(const char* s) {    
    I2C2CONbits.SEN = 1;// Initiate Start condition
    while (I2C2CONbits.SEN != 0);// Wait for end of start sequence
    
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = 0b01111100;// 8-bits consisting of the slave address and the R/nW bit
    
    while (*s != '\0') {
        while (IFS1bits.I2C2MIF != 1);
        
        IFS1bits.I2C2MIF = 0;
        I2C2TRN = 0b11000000;// Control byte. RS (the second MSB) is set.
        while (IFS1bits.I2C2MIF != 1);
        
        IFS1bits.I2C2MIF = 0;
        I2C2TRN = *s;
        
        s += 1;
        
        // We need to delay enough here
        // Otherwise we will get unexpected printing behavior
        lcd_delay_us(5);
    }
    
    // We can send the stop to end it
    
    I2C2CONbits.PEN = 1;// Initiate Stop condition
    while (I2C2CONbits.PEN != 0);// Wait for end of stop sequence
}

void lcd_delay_ms(unsigned int x) {
    int i;
    for (i = 0; i < x * lcd_ms_cycles; i += 1);
}

void lcd_delay_us(unsigned int x) {
    int i;
    for (i = 0; i < x * lcd_us_cycles; i += 1);
}