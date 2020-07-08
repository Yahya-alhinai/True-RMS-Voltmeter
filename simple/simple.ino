#include <SPI.h>
#include <Wire.h>
#include <FaBoLCDmini_AQM0802A.h>

#define MISOPIN 12
#define SCLKPIN 13
#define CLKOUT 9
#define bufsize 100

FaBoLCDmini_AQM0802A lcd;
byte byte1, byte2, byte3;
double data;
int numSampled = 0;
double rms = 0;
double sum = 0;

int data_out_count = 0;
double data_out = 0; 

unsigned long sendMillis;
unsigned long startMillis;

void setup()
{
  lcd.begin();
  Serial.begin(115200);

  //Set up Timer 1 for 8 MHz Clk
  pinMode(CLKOUT, OUTPUT); //Set CLK Out
  TCCR1A = bit (COM1A0);  //Toggle OC1A on Compare Match
  TCCR1B = bit (WGM12) | bit (CS10);   //CTC, no prescaling
  OCR1A =  0;       //Output every cycle

  pinMode(SCLKPIN, OUTPUT); //Set SCLK pin as output
  pinMode(MISOPIN, INPUT); //Set DRY/DOUT pin

  reset_adc(); //Put ADC on reset at the outset

  SPI.begin(); //SPI with default settings

  digitalWrite(SCLKPIN, LOW); //Release ADC from reset
  sendMillis = 2000;
  startMillis = millis();
}

void loop()
{
  if((millis()-startMillis) == sendMillis){
    Serial.println((byte)0x00,8);
    startMillis = millis();
  }
  while(digitalRead(MISOPIN) == HIGH); //Start of data in 
  while(digitalRead(MISOPIN) == LOW);
  while(digitalRead(MISOPIN) == HIGH);

  read_adc();
  
  if(numSampled >= bufsize)
  {
    rms = sum/(numSampled);
    rms = sqrt(rms);
    sum = 0;
    numSampled = 0;
    String toPrint;  
    toPrint = String(rms, 16);
//    Serial.print("Vrms is ");
//    Serial.println(rms, 16);
//    Serial.println();
    lcd.clear();
    lcd.home();
    lcd.print("mVrms:");
    lcd.setCursor(0, 1);
    lcd.print(toPrint);
    lcd.noAutoscroll();

  }
}

void reset_adc()
{
  digitalWrite(SCLKPIN, HIGH);
  delayMicroseconds(288); //wait for conversion
}

void read_adc()
{
  byte1 = SPI.transfer(0x00);
  byte2 = SPI.transfer(0x00);
  byte3 = SPI.transfer(0x00);

//  Serial.println(byte3);
//  Serial.println(byte2);
//  Serial.println(byte1);
  // read in adc data (sending out don't care bytes)
  // and store read data into three bytes */

  unsigned long int input;
  input = byte1;
  input <<= 8;
  input |= byte2;
  input <<= 8;
  input |= byte3;  
  input += 8388608;

  if (input >= 16777216)
    input = input - 16777216;
  
  data = (double) input;

  double data2 = ((data/(16777216))*5)-2.9;
  Serial.println(data2, 16);
  
//  data_out += data;
//  data_out_count++;
//  if (data_out_count == 10000){
//    data_out /= 10000; 
//    data_out_count = 0;
//    Serial.println(data, 16);
//    data_out = 0;
//  }

  sum += pow(data2*2,2);
  numSampled++;
}
