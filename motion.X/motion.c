/*
code for wireless communication of node 1 (motion sensor) 
want to add 2 nodes, a motion sensor and a sensor board
*/

#include <xc.h> 
  
// CONFIG1H 
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator) 
#pragma  config  FCMEN  =  OFF            //  Fail-Safe  Clock  Monitor  Enable  bit  (Fail-Safe  Clock  Monitor disabled) 
#pragma  config  IESO = OFF           //  Internal/External Oscillator  Switchover  bit  (Oscillator  Switchover mode disabled) 
  
// CONFIG2L 
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled) 
#pragma  config  BOREN  =  SBORDIS    //  Brown-out  Reset  Enable  bits  (Brown-out  Reset  enabled in hardware only (SBOREN is disabled)) 
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting) 
  
// CONFIG2H 
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit)) 
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768) 
  
// CONFIG3H 
#pragma config  CCP2MX  =  PORTC    // CCP2 MUX  bit (CCP2  input/output  is multiplexed with RC1) 
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset) 
#pragma  config  LPT1OSC  =  OFF        //  Low-Power  Timer1  Oscillator  Enable  bit  (Timer1  configured for higher power operation) 
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled) 
  
// CONFIG4L 
#pragma  config  STVREN  =  ON            //  Stack  Full/Underflow  Reset  Enable  bit  (Stack  full/underflow will cause Reset) 
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled) 
#pragma  config  XINST  =  OFF            //  Extended  Instruction  Set  Enable  bit  (Instruction  set  extension and Indexed Addressing mode disabled (Legacy mode)) 
  
// CONFIG5L 
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-003FFFh) not code-protected) 
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (004000-007FFFh) not code-protected) 
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (008000-00BFFFh) not code-protected) 
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (00C000-00FFFFh) not code-protected) 
  
// CONFIG5H 
#pragma  config  CPB  =  OFF            //  Boot  Block  Code Protection  bit (Boot  block (000000-0007FFh) not code-protected) 
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected) 
  
// CONFIG6L 
#pragma  config WRT0 = OFF         //  Write  Protection  bit  (Block  0  (000800-003FFFh)  not  write-protected) 
#pragma  config WRT1 = OFF         //  Write  Protection  bit  (Block  1  (004000-007FFFh)  not  write-protected) 
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (008000-00BFFFh) not write-protected) 
#pragma  config  WRT3  =  OFF          // Write Protection  bit (Block  3  (00C000-00FFFFh)  not  write-protected) 
  
// CONFIG6H 
#pragma  config  WRTC  =  OFF              //  Configuration  Register  Write  Protection  bit Configuration registers (300000-3000FFh) not write-protected) 
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected) 
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected) 
  
// CONFIG7L 
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks) 
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks) 
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks) 
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks) 
  
// CONFIG7H 
#pragma  config  EBTRB  =  OFF            //  Boot  Block  Table  Read  Protection  bit  (Boot  Block  (000000-0007FFh) not protected from table reads executed in other blocks) 
  
  
#include <stdlib.h>                               
#include <stdio.h> 
#include <spi.h>                                  
#include <delays.h>          
#include <usart.h>                             
#include <string.h>                             
#include <adc.h> 
#include <timers.h> 
#include "MRF24J40.h"   //make sure this header file is put in the project directory             
#include <p18c452.h> 
  
void Init_IO(void) 
{ 
            PORTA = 0x04;                                                                        //PORTA initially all zeros except RA2 (TC77 chip select) 
            TRISA = 0xF8;                                                                           //RA0 and RA1 outputs (LEDs), RA2 Output (TC77 CS), rest inputs 
            TRISB = 0xFF;                                                                           //PORTB all inputs (RB0 is interrupt, RB4 and RB5 are push buttons) 
            INTCON2bits.RBPU = 0;                                                          //enable pull up resistors on PORTB 
            ADCON0 = 0x1C;                                                                                 //turn off analog input 
  
            PORTCbits.RC0 = 1;                                                                 //Chip select (/CS) initially set high (MRF24J40) 
            TRISCbits.TRISC0 = 0;                                                  //Output: /CS 
            PORTCbits.RC1 = 1;                                                                 //WAKE initially set high (MRF24J40) 
            TRISCbits.TRISC0 = 0;                                                  //Output: WAKE 
            PORTCbits.RC2 = 1;                                                                 //RESETn initially set high (MRF24J40) 
            TRISCbits.TRISC2 = 0;                                                  //output: RESETn     
     
            INTCONbits.INT0IF = 0;                        //clear the interrupt flag (INT0 = RB0) 
            INTCONbits.INT0IE = 1;                                                       //enable INT0 
            RCONbits.IPEN = 1;                                                                 //enable interrupt priorities 
            INTCONbits.GIEH = 1;                                                          //global interrupt enable 
            OSCCONbits.IDLEN = 1;                                                           //enable idle mode (when Sleep() is executed) 
  OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT  &  USART_CONT_RX  &  USART_BRGH_HIGH,25  );          //setup USART @ 9600 Baud 
     
            OpenSPI(SPI_FOSC_4,MODE_00,SMPMID);               //setup SPI bus (SPI mode 00, 1MHz SCLK) (MRF24J40) 
} 
  
void USARTOut(char *data, char bytes) 
{ 
            int i; 
            for(i=0; i<bytes; i++) 
                        { 
                        while(BusyUSART()); 
                        WriteUSART(data[i]); 
                        } 
}            
  
typedef struct  
            { 
            unsigned char GID; 
            unsigned char NID;                             //unique identifier 
            float Data1, Data2, Data3;                             //data byte 
            unsigned int  crc;      
            }PacketType; 
  
PacketType TxPacket,RxPacket;                                 //transmitted and received packets 
char Text[128]; 
unsigned int ADCresult1, ADCresult2,ADCresult3, Strength, CRCRight; 
float Light, Temperature, Motion;                                            
  
void ADC(void) 
{ 
  // sensor board use ch1 and ch2, motion sensor use ch3
  
    OpenADC(ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_8_TAD,ADC_CH1 & ADC_INT_OFF & ADC_REF_VDD_VSS,0); 
    Delay10TCYx(5);    
    OpenADC(ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_8_TAD,ADC_CH2 & ADC_INT_OFF & ADC_REF_VDD_VSS,0); 
    Delay10TCYx(5); 
    OpenADC(ADC_FOSC_32 & ADC_RIGHT_JUST & ADC_8_TAD,ADC_CH3 & ADC_INT_OFF & ADC_REF_VDD_VSS,0); 
    Delay10TCYx(5); 
     
    SetChanADC(ADC_CH1); 
    ConvertADC(); 
    while(BusyADC()); 
    ADCresult1 = ReadADC(); 
  
    SetChanADC(ADC_CH2); 
    ConvertADC(); 
    while(BusyADC()); 
    ADCresult2 = ReadADC(); 
	
	SetChanADC(ADC_CH3); 
    ConvertADC(); 
    while(BusyADC()); 
    ADCresult3 = ReadADC();
} 
  
const unsigned char code_Table[256]={ 
0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D, 
0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D, 
0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD, 
0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD, 
0xC7,0xC1,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA, 
0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A, 
0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A, 
0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A, 
0x89,0x8E,0x87,0x80,0x95,0x93,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4, 
0xF9,0xFE,0xF7,0xF0,0xE5,0xE3,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4, 
0x69,0x6E,0x67,0x60,0x75,0x73,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44, 
0x19,0x1E,0x17,0x10,0x05,0x03,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34, 
0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x62, 
0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x12, 
0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x82, 
0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF2 
}; 
  
/*****************function to caluculate CRC************************/ 
unsigned char CalculateCRC(unsigned char* message, unsigned char length) 
{ 
     unsigned char i, crc = 0; 
      for (i = 0; i < length; i++) 
      crc = code_Table[crc ^ message[i]]; 
     return crc; 
} 
  
void main (void) 
{ 
Init_IO(); 
MRF24J40Init();                                               //initialise IEEE 802.15.4 transceiver 
//set RF channel CHANNEL_1x, WHERE x is the PC number, eg x=2 for PC-002, x=3 for PC-003,and x=4 for PC-004,... 
SetChannel(CHANNEL_17);//  WRITE  THE  VALUE  OF  x,  different  Group  has  different  channels/Use Group number as Channel number 
//set RF channel CHANNEL_1x, WHERE x is the PC number, eg x=2 for PC-002, x=3 for PC-003,and x=4 for PC-004,... 
OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_16);                   
//setup timer 0 with prescaler x16 
WriteTimer0(3036); 
while(1) 
        { 
         ADC(); 
         Light=(float)ADCresult1*3.2/1023; 
         Temperature=-25.445*(float)ADCresult2*3.2/1023+73.169;//After calibration, coefficients change 
		 Motion=(float)ADCresult3;
         TxPacket.GID = 23;//Group ID 
         TxPacket.NID = 1;//Node ID 
         TxPacket.Data1 = Light;                                           
         TxPacket.Data2 = Temperature;                                     
         TxPacket.Data3= Motion;									// motion sensor output

         TxPacket.crc = CalculateCRC(&TxPacket,sizeof(TxPacket)-sizeof(TxPacket.crc));     
//    //--------------------send data wirelessly----------------------- 
     if(INTCONbits.TMR0IF)                                                                                 
                        { 
                        INTCONbits.TMR0IF = 0;                                                                      
                        PORTA = 0x05;                                                                                                                                     
                        PHYTransmit((char  *)&TxPacket,sizeof(PacketType));                      //Transmit  RF  data packet  
						if	(Motion==0)
                        sprintf(Text,"Group ID=%u Node ID=%u Voltage_light=%.2f V Temperature=%.2f C Watching~~%.2f RSSI =%u CRCRight=%u \r\n", 
                                      TxPacket.GID, TxPacket.NID, TxPacket.Data1,TxPacket.Data2, TxPacket.Data3,Strength,CRCRight); 
						else
                        {
                        TxPacket.Data3=1;
						sprintf(Text,"Group ID=%u Node ID=%u Voltage_light=%.2f V Temperature=%.2f C Motion detected!%.2f RSSI =%u CRCRight=%u \r\n", 
                                      TxPacket.GID, TxPacket.NID, TxPacket.Data1,TxPacket.Data2, TxPacket.Data3,Strength,CRCRight); 
                        }
                        USARTOut(Text,strlen(Text));  
                        PORTA = 0x04; 
                        } 
            //----------------receive data------------------------- 
            if( PHYReceive((char *)&RxPacket,&Strength) == sizeof(PacketType)) 
                 
            { 
                PORTA = 0x06; 
                 if (0 == CalculateCRC(&RxPacket,sizeof(PacketType))) 
                                    CRCRight = 1;                                                                                       
                 else 
                       CRCRight = 0; 
                 switch(RxPacket.NID) //Apply different delay for different nodeID 
                       { 
                        case 2:  
                               sprintf(Text,"Group ID=%u Node ID=%u Voltage_light=%.2f V Voltage_temp=%.2f V RSSI =%u CRCRight=%u \r\n", 
                                                RxPacket.GID,  RxPacket.NID,RxPacket.Data1,  RxPacket.Data2, Strength, CRCRight); 
                               USARTOut(Text,strlen(Text)); 
                               break; 
                        case 3:  
                               sprintf(Text,"Group ID=%u Node ID=%u Moisture=%.2f Temperature=%.2f C Light=%.2f V RSSI =%u CRCRight=%u \r\n", 
                                                RxPacket.GID, RxPacket.NID,RxPacket.Data1,RxPacket.Data2,RxPacket.Data3,Strength, CRCRight); 
                               USARTOut(Text,strlen(Text)); 
                               break; 
                       } 
                PORTA = 0x04;    
            }                 
        }        
}
