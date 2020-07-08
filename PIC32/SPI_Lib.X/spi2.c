/* ************************************************************************** */
/**
  @Description
    Setup SPI2 peripheral.
 */
/* ************************************************************************** */

#include <xc.h>

void spi2_setup(void)
{
    TRISGbits.TRISG6 = 0;	    // set serial clock as an output
    TRISGbits.TRISG9 = 0;	    // set CS as an output
    TRISGbits.TRISG7 = 1;       // MISO input
    TRISGbits.TRISG8 = 0;       // MOSI output    
    
    
    _SDI2R = 0001;   // MISO RPn 
    _RPG8R = 0b0110; // MOSI RPn
    
    
    //ACCEL_CS = 1;               // disable chip select "ACCEL_CS"
        
    SPI2CON = 0;                // clear SPI2CON register
    SPI2BUF = 0;                // clear SPI2BUF register

    SPI2BRG = 511;              // SPI2BRG = ?   
               
    SPI2STATCLR = 1 << 6;        // clear overflow flag in SPI2STAT register
    
    SPI2CON = 0;                // framed support off    
    SPI2CONbits.MODE16 = 1;     // 16 bit communication
    
     
    SPI2CONbits.CKE = 0;        // CKE = ?
                      // Look at the pictures on pages 4-5 in LAB2 manual 
    SPI2CONbits.CKP = 1;        // CKP = ?                      // Very important to know how to figure these out

    SPI2CONbits.MSTEN = 1;      // master mode
    SPI2CONbits.SMP = 1;        // sample data at the end of clock time
    
    SPI2CONbits.ON = 1;         // enable SPI2 (ON)

}

int16_t spi2_read_register(uint8_t address)
{
    uint16_t read_frame;
    uint16_t value;
    
    read_frame = (((uint16_t)address|0x80) << 8) | 0x00;

    //delay(1);

    //ACCEL_CS = 0; // enable accelerometer chip select

    // send "read_frame to SPI2 buffer                   
    // wait for the SPI2 buffer full bit      
    // read the SPI2 buffer contents with the "value" variable                                      

    while(SPI2STATbits.SPIBUSY); // wait for spi to finish previous transactions
    
    SPI2BUF = read_frame;
    
    while(!SPI2STATbits.SPIRBF); // wait for data to be shifted in
    
    value = SPI2BUF & 0xff;
    
    //ACCEL_CS = 1; // disable accelerometer chip select
    
    return value;
}

void spi2_write_register(uint8_t address, uint8_t data)
{   
    uint16_t write_frame;
    uint16_t trash;
    
    write_frame = ((uint16_t)address << 8) | data;
            
    delay(1);

    // enable accelerometer chip select
    
    //ACCEL_CS = 0; // select device  
    
    while(SPI2STATbits.SPIBUSY); // wait for previous transactions to clear
    
    SPI2BUF = write_frame;  // send write frame
    
    while(!SPI2STATbits.SPIRBF);
    
    trash = SPI2BUF; // throw out shifted in data

    //ACCEL_CS = 1; // disable accelerometer chip select
}

