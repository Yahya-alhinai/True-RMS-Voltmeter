/* ************************************************************************** */
/** x500_uart1.h

  @Description
    Functions for setting up and using the uart1 peripheral.
 
 */
/* ************************************************************************** */

#ifndef _X500_UART1_H    /* Guard against multiple inclusion */
#define _X500_UART1_H

#include <xc.h>
#include <stdint.h>

void uart1_setup(void);
void uart1_send_char(uint8_t data);
void uart1_print_str(char * str);
uint8_t uart1_read_char(void);
void uart1_echo(void);

#endif /* _X500_UART1_H */

/* *****************************************************************************
 End of File
 */
