#ifndef INIT_H
#define INIT_H

#include <xc.h>

#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

void init_system(void) {
    // I/O setup
    TRISB = 0xE1;  // Keypad RB1-RB6 + Input for emergency interrupt (RB0)
    TRISC = 0x10;  // Input for confirmation button (RC4), SYS_LED (RC3), buzzer (RC6)
    TRISD = 0x00;  // Output for 7-segment
    TRISE = 0x03;  // Inputs for photoresistors

    //ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;
    
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;

    LATB = 0x1E;  
    LATD = 0;

    // Interrupt on RB0
    // Enable interrupt priority bit in INTCON0 (check INTCON0 register and find the bit)
    INTCON0bits.IPEN = 1;
    // Enable high priority interrupts using bits in INTCON0
    INTCON0bits.GIEH = 1;
    // Enable low priority interrupts using bits in INTCON0
    INTCON0bits.GIEL = 1;
    // Interrupt on rising edge of INT0 pin using bits in INTCON0
    INTCON0bits.INT0EDG = 1;

    // Set the interrupt high priority (IP) for INT0 - INT0IP
    IPR1bits.INT0IP = 1;
    // Enable the interrupt (IE) for INT0
    PIE1bits.INT0IE = 1;

    //Clear interrupt flag for INT0
    PIR1bits.INT0IF = 0;
  
    // Change IVTBASE by doing the following
    // Set IVTBASEU to 0x00 
    IVTBASEU = 0x00;
    // Set IVTBASEH to  0x40; 
    IVTBASEH = 0x40;
    // Set IVTBASEL to 0x08; 
    IVTBASEL = 0x08;
}

#endif
