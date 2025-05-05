/* Title: Assignment 9 ADC and Interfacing to an LCD
 * 
 * Program Description:
 *  This program demonstrates how to read analog signals using the built-in ADC 
 * on the PIC18F47K42 and display the corresponding voltage or light intensity 
 * (lux) on a 20x2 character LCD. It also incorporates an external interrupt 
 * using a button to trigger an LED blinking sequence and halt the program. 
 *  
 * I/O:
 * Inputs: 
 *  - Potentiometer (analog input) to RA0
 *  - Photo-resistor (analog input) to RA0 (need to switch between what input we want)
 *  - Pushbutton (digital interrupt) to RC2 (Interrupt-On-Change)
 * Outputs:
 *  - LCD 20x2 Character Display
 *      ? RS to RD0
 *      ? EN to RD1
 *      ? D0-D7 to RB0-RB7
 *  - LED to RC3 (blinks on interrupt)
 * 
 * Setup: C-Simulator
 * Date: May 4, 2025
 * File Dependencies / Libraries: 
 *      - <xc.h> for compiler-specific and device-specific features
 *      - <stdio.h> for sprintf
 *      - <string.h> for strcat
 *      - <stdlib.h> for general purposes
 * IDE: MPLAB X IDE v6.20
 * Compiler: XC8, 3.00
 * 
 * Author: Farid Farahmand
 * Author/Editor: Christian Gonzalez
 * 
 * Versions:
 *      V1.0: Displays analog voltage from a potentiometer on the LCD
 *      V2.0: Reads from a photoresistor and calculates light intensity (lux), 
 *            displays on the LCD
 *      V3.0: Adds an external interrupt (via pushbutton on RC2) to flash an LED 
 *            for 10 seconds and halts ADC
 * 
 * Useful links:
 *      V3.0 from GitHub: 
 *      Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
 *      PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
 *      List of Instructions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 
 * 
 */


// PIC18F46K42 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config FEXTOSC = LP     // External Oscillator Selection (LP (crystal oscillator) optimized for 32.768 kHz; PFM set to low power)
#pragma config RSTOSC = EXTOSC  // Reset Oscillator Selection (EXTOSC operating per FEXTOSC bits (device manufacturing default))

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCK bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = ON      // Multi-vector enable bit (Multi-vector enabled, Vector table used for interrupts)
#pragma config IVT1WAY = ON     // IVTLOCK bit One-way set enable bit (IVTLOCK bit can be cleared and set only once)
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown-out Reset Voltage Selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period selection bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled; SWDTEN is ignored)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4L
#pragma config BBSIZE = BBSIZE_512// Boot Block Size selection bits (Boot Block size is 512 words)
#pragma config BBEN = OFF       // Boot Block enable bit (Boot block disabled)
#pragma config SAFEN = OFF      // Storage Area Flash enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block write protection bit (Application Block not write protected)

// CONFIG4H
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write-protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config WRTSAF = OFF     // SAF Write protection bit (SAF not Write Protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

// CONFIG5L
#pragma config CP = OFF         // PFM and Data EEPROM Code Protection bit (PFM and Data EEPROM code protection disabled)

#include <xc.h> // must have this
//#include "../../../../../Program Files/Microchip/xc8/v2.40/pic/include/proc/pic18f46k42.h"
//#include "C:\Program Files\Microchip\xc8\v2.40\pic\include\proc\pic18f46k42"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

#define RS LATD0                   /* PORTD 0 pin is used for Register Select */
#define EN LATD1                   /* PORTD 1 pin is used for Enable */
#define ldata LATB                 /* PORTB is used for transmitting data to LCD */

#define LCD_Port TRISB              
#define LCD_Control TRISD

#define Vref 5.0 // voltage reference 
int digital; // holds the digital value 
float voltage; // hold the analog value (volt))
char data[10];

void ADC_Init(void);
void LCD_Init();
void LCD_Command(char );
void LCD_Char(char x);
void LCD_String(const char *);
void LCD_String_xy(char ,char ,const char*);
void MSdelay(unsigned int );
void IOCC2_Init(void);


// Interrupt
void __interrupt(irq(IRQ_IOC), base(0x4008)) IOC_ISR(void)
{
    if (IOCCFbits.IOCCF2)               // Check if RC3 caused the interrupt
    {
        for (int i = 0; i < 20; i++) {  // 20 cycles of 500ms = 10s
                LATCbits.LATC3 = 1;     // Turn ON LED
                __delay_ms(250);
                LATCbits.LATC3 = 0;     // Turn OFF LED
                __delay_ms(250);
            }
        IOCCFbits.IOCCF2 = 0;           // Clear IOC flag
        PIR0bits.IOCIF = 0;             // Clear peripheral IOC flag
    }
}


/*****************************Main Program*******************************/

void main(void)
{
    // MAIN INITIALIZATION
    ADC_Init();            // Initialize Analog-to-Digital Converter
    LCD_Init();            // Initialize LCD display in 8-bit mode
    IOCC2_Init();          // Set up Interrupt-On-Change for button on RC2

    TRISCbits.TRISC3 = 0;  // Configure RC3 as output (LED)
    LATCbits.LATC3 = 0;    // Ensure LED is off at startup

    
/****************************** THIS IS PART 2 ***************************/   
//    LCD_String_xy(1, 0, "The Input Light:");          // Display top label
//
//    while (1)
//    {
//        ADCON0bits.GO = 1;                            //Start conversion
//        while (ADCON0bits.GO);                        //Wait for conversion done
//        digital = (ADRESH*256) | (ADRESL);            // Combine 8-bit LSB and 2-bit MSB
//        voltage = digital * ((float)Vref / 4096.0); 
//        
//        int lux = (int)(85.19 * voltage + -135.33);   // Conversion using measured 2 measured values and y=mx+b
//        if (lux < 0) lux =0;                          // Don't want negatives
//
//        //print on LCD 
//        /*It is used to convert integer value to ASCII string*/    
//        sprintf(data,"%d", lux);
//    
//        strcat(data," LUX    ");      //Concatenate result and unit to print
//        LCD_String_xy(2,4,data);      // Display LUX value
//
//        
//        __delay_ms(500);  
//    }
/****************************** END OF PART 2 ***************************/
    
/****************************** THIS IS PART 1 ***************************/
    LCD_String_xy(1, 0, "Voltage:");        // Display top label
    
    while (1)
    {
        ADCON0bits.GO = 1;                  // Start conversion
        while (ADCON0bits.GO);              // Wait for conversion done
        digital = (ADRESH*256) | (ADRESL);  // Combine 8-bit LSB and 2-bit MSB
        voltage= digital*((float)Vref/(float)(4096)); 
        
        //print on LCD 
        /*It is used to convert integer value to ASCII string*/
        sprintf(data,"%.2f",voltage);

        strcat(data," V");          // Concatenate result and unit to print
        LCD_String_xy(2,4,data);    // Send string data for printing
        
        __delay_ms(500);            // Small delay to avoid flickering on the display
    }
/****************************** END OF PART 1 ***************************/
}

/****************************Functions********************************/
void IOCC2_Init(void) {
    TRISCbits.TRISC2 = 1;           // Set RC3 as input
    ANSELCbits.ANSELC2 = 0;         // Make RC3 digital
    WPUCbits.WPUC2 = 1;             // Enable weak pull-up

    IOCCPbits.IOCCP2 = 0;           // Disable positive edge
    IOCCNbits.IOCCN2 = 1;           // Enable negative edge (HIGH to LOW)
    IOCCFbits.IOCCF2 = 0;           // Clear flag
    PIR0bits.IOCIF = 0;

    PIE0bits.IOCIE = 1;             // Enable IOC
    INTCON0bits.GIE = 1;
    INTCON0bits.IPEN = 0;

    IVTBASEU = 0x00;
    IVTBASEH = 0x40;
    IVTBASEL = 0x08;
}



void LCD_Init()
{
    MSdelay(500);           /* 15ms,16x2 LCD Power on delay */
    LCD_Port = 0x00;       /* Set PORTB as output PORT for LCD data(D0-D7) pins */
    LCD_Control = 0x00;    /* Set PORTD as output PORT LCD Control(RS,EN) Pins */
    LCD_Command(0x01);     /* clear display screen */
    LCD_Command(0x38);     /* uses 2 line and initialize 5*7 matrix of LCD */
    LCD_Command(0x0c);     /* display on cursor off */
    LCD_Command(0x06);     /* increment cursor (shift cursor to right) */
}

void LCD_Clear()
{
        LCD_Command(0x01); /* clear display screen */
}

void LCD_Command(char cmd )
{
    ldata= cmd;            /* Send data to PORT as a command for LCD */   
    RS = 0;                /* Command Register is selected */
    EN = 1;                /* High-to-Low pulse on Enable pin to latch data */ 
    NOP();
    EN = 0;
    MSdelay(3); 
}

void LCD_Char(char dat)
{
    ldata= dat;            /* Send data to LCD */  
    RS = 1;                /* Data Register is selected */
    EN=1;                  /* High-to-Low pulse on Enable pin to latch data */   
    NOP();
    EN=0;
    MSdelay(1);
}


void LCD_String(const char *msg)
{
    while((*msg)!=0)
    {       
      LCD_Char(*msg);
      msg++;    
        }
}

void LCD_String_xy(char row,char pos,const char *msg)
{
    char location=0;
    if(row<=1)
    {
        location=(0x80) | ((pos) & 0x0f); /*Print message on 1st row and desired location*/
        LCD_Command(location);
    }
    else
    {
        location=(0xC0) | ((pos) & 0x0f); /*Print message on 2nd row and desired location*/
        LCD_Command(location);    
    }  
    LCD_String(msg);

}
/*********************************Delay Function********************************/
void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<165;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
}

void ADC_Init(void)
{
       //Setup ADC
    ADCON0bits.FM = 1;  //right justify
    ADCON0bits.CS = 1; //ADCRC Clock
    
    TRISAbits.TRISA0 = 1; //Set RA0 to input
    ANSELAbits.ANSELA0 = 1; //Set RA0 to analog
    // Added 
    ADPCH = 0x00; //Set RA0 as Analog channel in ADC ADPCH
    ADCLK = 0x00; //set ADC CLOCK Selection register to zero
    
    ADRESH = 0x00; // Clear ADC Result registers
    ADRESL = 0x00; 
    
    ADPREL = 0x00; // set precharge select to 0 in register ADPERL & ADPERH
    ADPREH = 0x00; 
    
    ADACQL = 0x00;  // set acquisition low and high byte to zero 
    ADACQH = 0x00;    
    
    ADCON0bits.ON = 1; //Turn ADC On 
}