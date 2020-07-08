#define _SUPPRESS_PLIB_WARNING 0    // This shuts it up, take that plib

#include <xc.h>
#include "p32xxxx.h"
#include <plib.h>
#include <math.h>
#include "AQM0802A_LCD.h"

#pragma config FWDTEN   = OFF    // Turn off watchdog timer
#pragma config FNOSC    = FRCPLL // Select 8 MHz internal Fast RC (FRC) oscillator with PLL
#pragma config FPLLIDIV = DIV_2  // Divide PLL input (FRC) to get into required range [4 MHz, 5 MHz]
#pragma config FPLLMUL  = MUL_24 // PLL Multiplier
#pragma config FPLLODIV = DIV_2  // PLL Output Divider
#pragma config FPBDIV   = DIV_1  // Peripheral Clock divisor
#pragma config JTAGEN   = OFF    // IMPORTANT. Otherwise bus collision may occur.
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)

// GLOBALS
#define BUF_SIZE 16
long double buf[BUF_SIZE];
int idx = 0;
long double sum = 0.0;

#define SYS_FREQ  (48000000)
#define GetSystemClock()       (48000000ul)
#define GetPeripheralClock()   (GetSystemClock())
#define BaudRate   1024000   // Maximim Rate of PICKit2 UART Tool
#define PWM_FREQ    8000000
#define DUTY_CYCLE  50
#define I2C_CLOCK_FREQ 100000           // define the baud rate, currently 100kHz
#define MY_SLAVE_ADDRESS 0b01111100     // LCD slave address for I2C
#define UART_DATA_RATE 100000           // define the uart baud rate
#define SPI_BAUD_RATE 8000000          // SPI serial clock rate

void system_reg_unlock(void){       // PPS unlock
    SYSKEY = 0x12345678;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
}

void system_reg_lock(void){         // PPS lock
    SYSKEY = 0x00000000;
}

void delay_ms(int m){
    int i;
    int j;
    // 1 ms delay
    for(i = 0; i < m; i++){
        for (j = 0; j < 5999; j++) asm("nop");
    }
}

void initBuf(void){
    int i;
    for(i = 0; i < BUF_SIZE; i++){
        buf[i] = 0.0;
    }
}

void initSPI(void){
    // PPS
    system_reg_unlock();
    SDI1Rbits.SDI1R = 0;    // SDI1 = RPA1 = Pin #3
    system_reg_lock();
    // setup the peripheral
    SpiChnOpen(SPI_CHANNEL1, SPI_OPEN_MSTEN|SPI_OPEN_SMP_END|SPI_OPEN_MODE32, GetPeripheralClock()/SPI_BAUD_RATE);
}

void initI2C(void){
    // setup the peripheral
    I2CSetFrequency(I2C2, GetPeripheralClock(), I2C_CLOCK_FREQ);
    I2CSetSlaveAddress(I2C2, MY_SLAVE_ADDRESS, 0, I2C_USE_10BIT_ADDRESS);
    I2CEnable(I2C2, TRUE);
}

void initUART(void){
    SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    mPORTAClearBits(BIT_3);   
    mPORTASetPinsDigitalOut(BIT_3);      // Set PB10(Tx) as output
    mPORTBSetPinsDigitalIn (BIT_11);      // Set PB11(Rx) as input
    system_reg_unlock();
    RPA3Rbits.RPA3R = 0b0010;       // UART2 TX to RPA3
    U2RXRbits.U2RXR = 0b0011;       // UART2 RX to RPB11
    system_reg_lock();
    
    // Configure UART2
    UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY); 
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1); 
    UARTSetDataRate(UART2, GetPeripheralClock(), BaudRate); 
    UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));   
}

void setupFTDI(void){
    //setup the device
    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB4 = 0;
    _RB4 = 0;
    system_reg_unlock();        // pps unlock & unlock
    U2RXR = 0b0001;             // RB5 = RX (U2)
    RPB4R = 0b0001;             // RB4 = TX (U1)    I know, I done goofed, oops
    system_reg_lock();          // ignoring the rts & cts pins
}

long double getRMS(void){
    // get reading from ADC and convert to RMS
    // return RMS value
    long double average;
    long double rms;
    average = sum/BUF_SIZE;
    rms = sqrt(average);
    // more stuff that I got distracted from
    return rms;
}

void displayRMS(unsigned int rms){
    // send the RMS value to the LCD to be displayed
    char str[20];
    sprintf(str, "%6.4f", rms);
    lcd_print_str(str);
}

void sendToHist(int data){
    // send the RMS value to the histogram
    while(UARTTransmitterIsReady(UART1));
    UARTSendDataByte (UART1, (BYTE)data);
    while(UARTTransmitterIsReady(UART1));
    UARTSendDataByte (UART1, (BYTE)(data>>8));
    while(UARTTransmitterIsReady(UART1));
    UARTSendDataByte (UART1, (BYTE)(data>>16));
//    Serial_print(&(buf[idx]));
}

void serialPrint(char *buffer){ 
   while(*buffer != (char)0) 
   {
      while(!UARTTransmitterIsReady(UART2)); 
      UARTSendDataByte(UART2, *buffer++); 
   }
    while(!UARTTransmissionHasCompleted(UART2)); 
   UARTSendDataByte(UART2, '\r'); 
   UARTSendDataByte(UART2, '\n'); 
} 

void put(long double value){
    sum -= buf[idx];
    buf[idx++] = value*value;
    idx %= BUF_SIZE;
    sum += buf[idx];
}

void setupADCClk(){
    SYSTEMConfigPerformance(SYS_FREQ);
    mPORTBSetPinsDigitalOut(BIT_13);            // Set OC1 to pin RB7 with peripheral pin select
    RPB13Rbits.RPB13R = 0x0005;
    OC4CON = 0x0006;                            // Configure standard PWM mode for output compare module 1
    PR3 = (SYS_FREQ / PWM_FREQ) - 1;            // A write to PRy configures the PWM frequency
                                                // ... PR = [FPB / (PWM Frequency * TMR Prescale Value)] ? 1
                                                // ... : note the TMR Prescaler is 1 and is thus ignored
    OC4RS = (PR3+1)*((float)DUTY_CYCLE/100);    // A write to OCxRS configures the duty cycle
                                                // ... : OCxRS / PRy = duty cycle
    T3CONSET = 0x8000;                          // Enable Timer2, prescaler 1:1
    OC4CONSET = 0x8008;                         // Enable Output Compare Module 1
}

void setupAll(void){
    //set system clacl (CLKDIVbits.RCDIV)
    //set pins all digital (AD1PCFG)
    setupADCClk();
    initSPI();      // (ADC)
    //initI2C();      // (LCD)
    initUART();     // USB
    //setupLCD();
    setupFTDI();    // USB
    initBuf();
    lcd_init();
    
    // Setup a timer for sending a 0 every 2 seconds
    T1CON = 0;
    T1CONbits.TCKPS = 0b11;     // Prescalar = 256
    TMR1 = 0;
    PR1 = 62499;                // With prescalar, time = 1/3 sec
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
}

void triggerSPI(void){
    int garbage;
    SpiChnPutC(SPI_CHANNEL1, 0x1b);
    garbage = SpiChnReadC(SPI_CHANNEL1);
    SpiChnPutC(SPI_CHANNEL1, 0x1b);
    IEC1bits.SPI1RXIE = 1;       // Interrupt when RX buffer is full
    IFS1bits.SPI1RXIF = 0;
}

void __attribute__ ((interrupt(IPL4SOFT)))__T1Interrupt(void){
    IFS0bits.T1IF = 0;
    static count = 0;
    if(count==5){               // Every 2 seconds (6 times in int) send 0 to Hist
        sendToHist(0x0);
    }
    count++;
    count %= 6;
}
    
void __ISR(_SPI_1_VECTOR, IPL3AUTO)__SPI1Interrupt(void){
    IFS1bits.SPI1RXIF = 0;
    static count = 0;
    int data;
    // read from ADC
    data = SpiChnReadC(SPI_CHANNEL1);
    put(data);                              // Store response in the buffer
    SpiChnPutC(SPI_CHANNEL1, 0x1b);         // Send garbage to trigger a response
    
    if(count == 0){
        char arr[20];
        sprintf(arr, "%d", data);
        sendToHist(data);                       // Send every 4 raw value to histogram
        serialPrint(arr);
    }
    count++;
    count %= 4;
}

unsigned int temp[10];
char arr[20];

int main(void){ 
    setupAll();
    int i = 0;
//    while (1){
//        lcd_set_cursor(0, 0);
//        //sprintf(arr, "%d", PORTBbits.RB7);
//        sprintf(arr, "%d", i++);
//        lcd_print_str(arr);
//        //serialPrint(arr);
//        //delay_ms(100);
//    }
    triggerSPI();
    int rms;
    while(1){
        rms = getRMS();             // Calculate the RMS
        displayRMS(rms);            // Display the current RMS on LCD
        //sendToHist(rms); //done in interrupt           
        //displayRMS(*temp);//getRMS());
        asm("nop");
    }
    return 0;
}