
#include <xc.h>
#include <proc/p32mx250f128b.h>
#include <plib.h>

#define PERIPHERAL_CLOCK 10000000       // the clock source for peripherals
#define I2C_CLOCK_FREQ 100000           // define the baud rate, currently 100kHz
#define MY_SLAVE_ADDRESS 0b01111100     // LCD slave address for I2C 
#define UART_DATA_RATE 100000           // define the uart baud rate
#define _SUPPRESS_PLIB_WARNING 0

void system_reg_unlock(void)
{
    SYSKEY = 0x12345678;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
}

void system_reg_lock(void)
{
    SYSKEY = 0x00000000; 
}

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

void initSPI(void){
    // setup the peripheral
}

void initUART(void){
    // setup the peripheral
    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_TX_BUFFER_EMPTY);
    UARTSetDataRate(UART1, PERIPHERAL_CLOCK, UART_DATA_RATE);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}

void initI2C(void){
    // setup the peripheral
    //I2CConfigure(I2C1, )  //Never mind, we don't need the config options that this func uses
    I2CSetFrequency(I2C2, PERIPHERAL_CLOCK, I2C_CLOCK_FREQ);
    I2CSetSlaveAddress(I2C2, MY_SLAVE_ADDRESS, 0, I2C_USE_10BIT_ADDRESS);
    I2CEnable(I2C2, TRUE);
}

void setupLCD(void){
    //setup the device
    setup_lcd();
    lcd_init();
}

void setupFTDI(void){
    //setup the device
    // pps unlock & unlock
    system_reg_unlock();
    U2RXR = 0b0001;         // RB5 = RX (U2)
    RPB4R = 0b0001;         // RB4 = TX (U1)    I know, I done goofed, oops
    // ignoring the rts & cts pins
    system_reg_lock();
}

void mainSetup(void){
    // setup a timer for delays
    T1CON = 0;
    PR1 = 3999;             //1 ms
    TMR1 = 0;
    IFS0bits.T1IF = 0;
    T1CONbits.ON = 1;
    
    // setup a timer for sending 
    
    //initSPI();      // (ADC)
    initI2C();      // (LCD)
    initUART();     // USB
    setupLCD();
    setupFTDI();    // USB
}

