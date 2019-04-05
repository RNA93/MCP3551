
// PIC16F1828 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#define _XTAL_FREQ 16000000

#include <xc.h>
#define SDI_TRI TRISB4
#define SDO_TRI TRISC7
#define SCK_TRI TRISB6
#define SS_TRI  TRISC6

#define SS          RC6
#define SDI         RB4
#define SCK         RB6

#define HOLD  NOP();NOP();

static unsigned long int ADC_DATA=0,ADC_DATA1=0;
static unsigned long int  ADC_value;
static unsigned char Byte[10]; 
static float voltage;     
unsigned char P=0;
void spi_init_MCP3551()
{   
    ANSELB=0;
    ANSELC=0;
    SS_TRI=0;//output
    SDO_TRI=0;//output
    SDI_TRI=1;//input
    SCK_TRI=0;//output
    SS=1;
 //mode 0,0 CKE=0 CKP=0 with sampling frequency of Fosc/4  
 SSP1CON1 = 0b00100000;              
 SSP1STAT = 0b00000000;              
 SSP1STATbits.BF=0x00;               //clearing  BF bit
             

}

void readADC()
 { 
                
   up:           spi_init_MCP3551();// SPI initialization
                 __delay_ms(1);
                 SS = 1;  // SS high
                 __delay_ms(1);
                 SS = 0; // SS LOW
                 __delay_us(10);

               
                SSP1BUF=0xFF;            //dummy data sending
                while(!SSP1IF);
                Byte[3]=SSP1BUF;      //receiving first byte
               // SS=1;
                __delay_us(1);
                   
                SSP1BUF=0xFF;            //dummy data sending
                while(!SSP1IF);
                Byte[2]=SSP1BUF;         //receiving first byte
                 __delay_us(1);
                 
                SSP1BUF=0xFF;            //dummy data sending
                while(!SSP1IF);
                Byte[1]=SSP1BUF;         //receiving first byte
                __delay_us(1);
                
                SSP1BUF=0xFF;            //dummy data sending
                while(!SSP1IF);
                Byte[0]=SSP1BUF;          //receiving first byte
                __delay_us(1);
                SS=1; //disable MCP3551
                __delay_ms(1);
                SSPEN=0;//turn OFF SPI mode
                
}
void inituart()
{
	 BAUDCON = 0b00000000;    // 8 BIT BAUDRATE GENERATOR MODE, BRG16 = 0
	 SP1BRGL = 25;             // BR = 9600bps, WITH 16MHZ
     
	 TRISB7 = 0;               // TX AS OUTPUT
	 TRISB5 = 1;               // RX AS INPUT
	 TXSTA = 0b00100000;      // 8BIT TRANSMISSION, TRANSMIT ENABLE,ASYNCHRONOUS,BR LOW
	 RCSTA = 0b10010000;      // SERIAL PORT ENABLED, 8-BIT RECEPTION, CONTINEOUS RECEPTION ENABLED
	 RCIE = 1;                // RECEIVE INTERRUPT ENABLED 

     //RTENBL = 0;               // RC0 = 0, RECEIVE MODE:: RC0 = 1, TRANSMIT MODE 
}

void uartrans(unsigned char tbyte)
{
 while(!(TXSTA&0x02));
 TXREG = tbyte;
}

void main(void)
{  
     OSCCON=0x78;
     inituart();
     
      
        while(1)
     {    
                        again: //very important loop,
      
                        ADC_DATA=0;
                        Byte[0]=0;     Byte[1]=0;   Byte[2]=0;      Byte[3]=0;
                        readADC(); 
                        ADC_DATA =ADC_DATA | Byte[3];
                        ADC_DATA =( (ADC_DATA<<8)| Byte[2] ) ;
                        ADC_DATA =( (ADC_DATA<<8)| Byte[1] );
                        ADC_DATA =( (ADC_DATA<<8)| Byte[1] );
                        ADC_DATA>>=7;                           //shifting data to right by 7
                        ADC_DATA &= 0x001FFFFF;                 //removing Overflow bits.  
                        if(ADC_DATA==0x001FFFFF)goto again;     //
                        
                        /* above line checking  ADC_DATA==0x001FFFFF is to remove error.
                         * don't forget to use it.
                         * According to Data sheet of MCP3551 we need to read 4 bytes ,
                         * these 4 byte includes 1 ready bit 1 underflow and overflow bits 
                         * and 7 extra bits
                         * in short we have to take only 21 bit data of ADC,and other bits have to remove.
                         * thats what we done in above few lines
                         */
                        
                      voltage=ADC_DATA*0.000001525;//resolution when Vref is 3.1V =0.000001525 //here we are converting ADC values in input Voltage form
                     
                      // bellow code is only to sending ADC value to UART in decimal format.
                       while( ADC_DATA != 0)
                            {
                                    ADC_DATA1   = ADC_DATA1 + (ADC_DATA %10);
                                    
                                    ADC_DATA    = ADC_DATA / 10 ;
                                    
                                    ADC_DATA1   *=  10;    
                             };
                             
                       while( ADC_DATA1 != 0)
                            {
                                 
                                    uartrans((ADC_DATA1 %10)+0x30);
                                    ADC_DATA1    = ADC_DATA1 / 10 ;
                                    
                             };         
                    uartrans(' ');
                   __delay_ms(1000);
    
     
     }
    
}
