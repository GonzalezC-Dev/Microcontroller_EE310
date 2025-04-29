/* Title: Assignment 8 ; Interfacing with Sensors
 * 
 * Program Description:
 * This project implements a secure safebox system using the PIC18F47K42 microcontroller.
 * The system allows a user to unlock the box by entering a pre-set SECRET_CODE using
 * two photo-resistors (PR1 and PR2) as touch-less binary inputs (0?4 range).
 * A 3x4 keypad is used to set or change the secret code by pressing '*'.
 * A 7-segment display shows the digits being entered. 
 *  
 * I/O:
 * Inputs:  Interrupt Button (one end connected to GND and the other to port RB0)
 *          Confirmation Button (one end connected to GND and the other to port RC4)
 *          3x4 Keypad (Rows connected to RB1-RB4, columns connected to RB5-RB7)
 *          Photo-resistor 1 connected to RE0 and photo-resistor 2 connected to RE1
 * Outputs: LED (cathode connected to GND, anode connected to RC3)
 *          7-Segment connected to RD0-RD6 in alphabetical order from 0(a) to 6(g)
 *          Relay (RC7 to IN, DC+ to positive logic power supply, DC- to GND)
 *              (Other end to motor: NO to one pin of motor, COM to other motor pin)
 *          Buzzer connected one end to GND other to RC6
 * 
 * Setup: C-Simulator
 * Date: April 29, 2025
 * File Dependencies / Libraries: 
 *      - Header file "config.h" for microcontroller settings
 *      - Initialization file "init.h" to initialize pins on microcontroller
 *      - Functions file "functions.h" that holds all functions of this project
 *      - <xc.h> for compiler-specific and device-specific features
 * IDE: MPLAB X IDE v6.20
 * Compiler: XC8, 3.00
 * 
 * Author: Christian Gonzalez
 * 
 * Versions:
 *      V1.0: Initial setup, no motor features, no keypad
 *      V2.0: All features integrated, including the ability to enter and change code using the keypad
 * 
 * Useful links:
 *      V2.0 from GitHub: https://github.com/GonzalezC-Dev/Microcontroller_EE310/tree/main/Assignments/InterfacingWithSensors_A8.X
 *      Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
 *      PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
 *      List of Instructions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 
 * 
 */


#include "config.h"
#include "init.h"
#include "functions.h"
#include <xc.h> // must have this


#define _XTAL_FREQ 4000000  // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

void main(void) {
    init_system(); // Initialize the system
    PORTCbits.RC3 = 1; // SYS_LED turned on
    SECRET_CODE = set_new_secret_code(); // Set the first secret code

    while (1) {
        PORTCbits.RC3 = 1;
        // Check if user wants to set a new code
        check_for_change_code_request();
        // Check if there is an input from PR1 or PR2
        check_for_PR_input();
        // Check for code correct or wrong
        code_correct_or_wrong();
    }
}
