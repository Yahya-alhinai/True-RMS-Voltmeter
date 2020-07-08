/* ************************************************************************** */
/** uart1_setup.c

  @Description
    Sets up UART1 with a baud rate of 9600.
 
 */
/* ************************************************************************** */

#include <xc.h>
#include "uart1.h"

//void system_reg_unlock(void)
//{
//    SYSKEY = 0x12345678;
//    SYSKEY = 0xAA996655;
//    SYSKEY = 0x556699AA;
//}

//void system_reg_lock(void)
//{
//    SYSKEY = 0x00000000; 
//}

void uart1_setup(void)
{   
//    system_reg_unlock();            // unlock PPS
//    CFGCONbits.IOLOCK = 0;
//	U1RXRbits.U1RXR = 0x0002; // RF4->UART1:U1RX;
//    RPF5R = 0b0011; // RF5 -> U1TX
//    CFGCONbits.IOLOCK = 1;          // lock   PPS
//    system_reg_lock(); 
    
    __XC_UART = 1; // enable printf for uart1
    U1MODE = 0;
    U1STA  = 0;
    U1MODEbits.BRGH = 0;        // BRGH OFF; UEN TX_RX; ON enabled; 
    U1MODEbits.UEN = 0;
    U1BRG = 259;                // BaudRate = 9600; Frequency = 40000000 Hz; BRG 259; 
    U1STAbits.UTXEN = 1;        // Enable transmit
    U1STAbits.URXEN = 1;        // Enable receive
    U1MODEbits.ON = 1;          // Enable UART
}

void uart1_send_char(uint8_t data)
{
    while(U1STAbits.UTXBF);         // Wait while buffer is full
    U1TXREG = data;                 // Send character via UART1
}

void uart1_print_str(char * str){
    while (*str != '\0'){
        uart1_send_char(*str);
        str = ++str;
    }
}    

uint8_t uart1_read_char(void)
{
    while(!U1STAbits.URXDA);        // Wait for information to be received
    if(U1STAbits.OERR){             // Check if OERR bit was set, clear if set
        U1STAbits.OERR = 0;
    }
    return U1RXREG;                 // Return received character
}

void uart1_echo(void)
{
    char data;
    if(U1STAbits.URXDA){            // Read UART and echo data back
        data = uart1_read_char();
        uart1_send_char(data);
    }
}

/* *****************************************************************************
 End of File
 */
