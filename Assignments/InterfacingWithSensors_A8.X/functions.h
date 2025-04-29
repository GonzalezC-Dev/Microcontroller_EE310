#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

uint8_t SECRET_CODE = 00;
uint8_t high_digit = 0;
uint8_t low_digit = 0;
uint8_t confirmation = 0;
uint8_t user_code;
uint8_t input_digit(uint8_t sensor_pin);
void display_digit(char digit);
bool check_code(uint8_t user_code);
void activate_motor(void);
void activate_buzzer(void);
void emergency_melody(void);
void __interrupt(irq(IRQ_INT0), base(0x400)) INT0_ISR(void);
char get_keypad_key(void);
uint8_t wait_for_keypad_digit(void);
void check_for_change_code_request(void);
void check_for_PR_input(void);
void code_correct_or_wrong(void);
uint8_t set_new_secret_code(void);

// Handle inputs from the photo-resistors
uint8_t input_digit(uint8_t sensor_pin) {
    uint8_t count = 1;
    while (PORTEbits.RE0 == 1 || PORTEbits.RE1 == 1); // wait til uncovered
    display_digit((char)(count + '0'));
    
    while (1) {
        
        if (sensor_pin == 0 && PORTEbits.RE0 == 1 && count < 4) { // PR1
            __delay_ms(500);
            count++;
            while (PORTEbits.RE0 == 1); // wait until uncovered
            PORTE = 0;
            display_digit((char)(count + '0'));
        }
        else if (sensor_pin == 1 && PORTEbits.RE1 == 1 && count < 4) { // PR2
            __delay_ms(500);
            count++;
            while (PORTEbits.RE1 == 1); // wait until uncovered
            PORTE = 0;
            display_digit((char)(count + '0'));
        }

        // Wait for confirm button press (e.g., RC4)
        if (PORTCbits.RC4 == 1) {
            __delay_ms(50);
            confirmation++;
            display_digit('E');  // Show 3 lines while waiting
            return count;
        }
    }
}

// Helper to display digits on the 7-segment
void display_digit(char value) {
    switch (value) {
        case '0': LATD = 0b00111111; break;
        case '1': LATD = 0b00000110; break;
        case '2': LATD = 0b01011011; break;
        case '3': LATD = 0b01001111; break;
        case '4': LATD = 0b01100110; break;
        case '5': LATD = 0b01101101; break;
        case '6': LATD = 0b01111101; break;
        case '7': LATD = 0b00000111; break;
        case '8': LATD = 0b01111111; break;
        case '9': LATD = 0b01101111; break;
        case '*': LATD = 0b01100011; break;
        case '#': LATD = 0b01111001; break;
        case 'E': LATD = 0b01001001; break;
        default:  LATD = 0b01001001; break;
    }
}

// Checks if the code is a match
bool check_code(uint8_t user_code) {
    return user_code == SECRET_CODE;
}

// Turn motor on for small duration
void activate_motor(void) {
    PORTCbits.RC7 = 1; // Motor on
    __delay_ms(3000);
    PORTCbits.RC7 = 0; // Motor off
}

// Turn buzzer on for small duration
void activate_buzzer(void) {
    PORTCbits.RC6 = 1; // Buzzer on
    __delay_ms(2000);
    PORTCbits.RC6 = 0; // Buzzer off
}

// Play a melody through the buzzer (during interrupts)
void emergency_melody(void) {
    for (uint8_t i = 0; i < 5; i++) {  
        PORTCbits.RC3 = 0;
        PORTCbits.RC6 = 1;
        __delay_ms(500);
        PORTCbits.RC3 = 1;
        PORTCbits.RC6 = 0;
        __delay_ms(500);
    }
}

// Interrupt function
void __interrupt(irq(IRQ_INT0), base(0x4008)) INT0_ISR(void) {
    emergency_melody();
    PIR1bits.INT0IF = 0;    // always clear the interrupt flag for INT0 when done
    //while (1); // halt system
}

// Keypad lookup
char get_keypad_key(void) {
    char keys[4][3] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}
    };

    for (int row = 0; row < 4; row++) {
        // Set one row LOW, others HIGH
        LATBbits.LATB1 = (row == 0) ? 0 : 1;
        LATBbits.LATB2 = (row == 1) ? 0 : 1;
        LATBbits.LATB3 = (row == 2) ? 0 : 1;
        LATBbits.LATB4 = (row == 3) ? 0 : 1;

        __delay_ms(5);  // debounce

        // Check each column (RA4-RA6)
        if (!PORTBbits.RB5) return keys[row][0];
        if (!PORTBbits.RB6) return keys[row][1];
        if (!PORTBbits.RB7) return keys[row][2];
    }

    return 0; // no key pressed
}



// If setting a new code we wait for the keypad to be pressed
uint8_t wait_for_keypad_digit(void) {
    char key;
    do {
        LATD = 0b00000001;
        __delay_ms(10);
        LATD = 0b00000010;
        __delay_ms(10);
        LATD = 0b00000100;
        __delay_ms(10);
        LATD = 0b00001000;
        __delay_ms(10);
        LATD = 0b00010000;
        __delay_ms(10);
        LATD = 0b00100000;
        __delay_ms(10);
        key = get_keypad_key();
    } while (key < '0' || key > '4'); // Only accept digits 0?4
    return key - '0';
}

// Function to check if a new code is wanting to be set, set it
void check_for_change_code_request(void) {
    char key = get_keypad_key();
    if (key == '*'){
            SECRET_CODE = set_new_secret_code();
        }

}

// Function to check for photo-resistor inputs
void check_for_PR_input(void) {
    if(PORTEbits.RE0 == 1 || PORTEbits.RE1 == 1){
        if (PORTEbits.RE0 == 1){
            high_digit = input_digit(0); // PR1
        
            __delay_ms(500);
        }else if (PORTEbits.RE1 == 1){
            low_digit = input_digit(1); // PR2
            
            __delay_ms(500);
        }
    
        user_code = (high_digit * 10) + low_digit;
    }
}

// Handles if the code is correct or wrong then resets variables
void code_correct_or_wrong(void) {
    if (confirmation == 2) {
        if (check_code(user_code)) {
            activate_motor();
        } else {
            activate_buzzer();
        }
        high_digit = 0;
        low_digit = 0;
        confirmation = 0;
    }
}

// Function to set the secret code
uint8_t set_new_secret_code(void) {

    uint8_t high = wait_for_keypad_digit();  // First digit
    display_digit((char)(high + '0'));
    __delay_ms(1000);

    uint8_t low = wait_for_keypad_digit();   // Second digit
    display_digit((char)(low + '0'));
    __delay_ms(1000);
    
    display_digit('E');
    
    uint8_t newCode = (high * 10) + low;
    return newCode;
}


#endif
