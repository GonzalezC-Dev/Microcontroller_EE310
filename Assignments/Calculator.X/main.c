/*
 * Title: 4x4 Keypad Calculator with LED Display
 * ---------------------
 * Program Details:
 *  The purpose of this program is to read inputs from a 4x4 matrix keypad, perform 
 *  basic arithmetic operations (addition, subtraction, multiplication, and division), 
 *  and display the binary result on 8 LEDs.
 *  The calculator supports two-digit inputs for both operands, and the result is 
 *  displayed on PORTD (8 LEDs).
 * Inputs: Keypad (4x4 matrix keypad)
 * Outputs: PORTA (8 LEDs)
 * Setup: C-Simulator
 * Date: April 7, 2025
 * File Dependencies / Libraries: 
 *      - Header file "header.h" for microcontroller settings
 *      - Standard libraries for math and string operations
 * Compiler: xc8, 3.00
 * Author: Christian Gonzalez
 * Versions:
 *      V1.0: Initial implementation
 * Useful links:
 *      Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
 *      PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
 *      List of Instructions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 
 * 
 * Code Description:
 *  - The program scans the 4x4 keypad for key presses, which correspond to numbers or operators.
 *  - When a digit key (0-9) is pressed, the program updates the first or second operand based on the 
 *    current state.
 *  - Operation keys ('A' for addition, 'B' for subtraction, 'C' for multiplication, 'D' for division) 
 *    are stored in the `Operation_REG` variable.
 *  - Pressing the `#` key will trigger the calculation and display the result on the LEDs connected to PORTA.
 *  - Pressing the `*` key resets the calculator to its initial state.
 *  
 *  Keypad functionality:
 *  - Rows (RB0?RB3) are used as output to activate one row at a time.
 *  - Columns (RB4?RB7) are inputs used to detect which key is pressed.
 *  - The program scans the keypad by setting one row to LOW and checking each column to detect which key is pressed.
 *  - If a key is pressed, it updates the corresponding operand or operator, or performs the calculation.
 *  - After the calculation, the result is displayed on the 8 LEDs connected to PORTA.
 * 
 */

#include <xc.h> // must have this
#include "C:/Program Files/Microchip/xc8/v3.00/pic/include/proc/pic18f47k42.h"
#include "header.h"

#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

// Keypad connections on PORTB
// RB0-RB3 = Rows (outputs)
// RB4-RB7 = Columns (inputs)

/*
 * This function is used to configure the microcontroller for inputs and outputs
 * params: none
 * return: N/A
 */
void setup() {
    // Setup keypad: RB0-RB3 as outputs (rows), RB4-RB7 as inputs (columns)
    LATB = 0x0F;
    ANSELB = 0x00;
    TRISB = 0b11110000;  // Lower 4 bits output, upper 4 bits input
    PORTB = 0xFF;        // Make sure PORTB is initialized

    // Setup LEDs: PORTD as output
    ANSELD = 0x00;
    TRISD = 0x00;
    LATD = 0x00;
    PORTD = 0x00;
}

/*
 * This function is used to scan the keypad for inputs using row activation
 * (checks row by row).
 * params: none
 * return: key pressed
 */
char getKeyPressed() {
    char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    // Array for row output patterns to check rows one by one
    unsigned char row_values[4] = {0b11111110, 0b11111101, 0b11111011, 0b11110111};

    for (int row = 0; row < 4; row++) {
        LATB = row_values[row]; // Set one row LOW, others HIGH
        __delay_ms(5);          // Make sure there's no misinterpretation

        // Depending on he key pressed, will return the corresponding char from the 
        // keys 2D array
        if (!PORTBbits.RB4) return keys[row][0];
        if (!PORTBbits.RB5) return keys[row][1];
        if (!PORTBbits.RB6) return keys[row][2];
        if (!PORTBbits.RB7) return keys[row][3];
    }

    return 0; // No key is pressed
}

// Global variables
int X_Input_REG = 0;        // First input
int Y_Input_REG = 0;        // Second input
int Display_Result_REG = 0; // Result
char Operation_REG = 0;     // Type of operation (Holds A-D)
int digitCount = 0;         // Keeps track of how many digits have been input for each operand
int isSecond = 0;           // Keeps track of whether we are on the second operand

/*
 * This function is used to reset all variables and LEDs
 * params: none
 * return: none
 */
void resetAll() {
    X_Input_REG = 0;
    Y_Input_REG = 0;
    Display_Result_REG = 0;
    Operation_REG = 0;
    digitCount = 0;
    isSecond = 0;
    PORTD = 0x00; // Turn off LEDs
}

/*
 * This function is used to show the result in binary on the 8 LEDs
 * params: value: the result in hex
 * return: none
 */void displayOnLEDs(int value) {
    if (value < 0)          // If the result is negative we negate the result and add 1
        value = ~value + 1; // This is the same as 2s complement
    PORTD = (char)value;    // Output the result to the LEDs, char because only need 8 bits
}

/*
 * This function is used to do the calculation needed based on the inputs
 * params: none
 * return: none
 */void calculate() {
    if (Operation_REG == 'A') Display_Result_REG = X_Input_REG + Y_Input_REG;      
    else if (Operation_REG == 'B') Display_Result_REG = X_Input_REG - Y_Input_REG;  
    else if (Operation_REG == 'C') Display_Result_REG = X_Input_REG * Y_Input_REG;
    else if (Operation_REG == 'D' && Y_Input_REG != 0) Display_Result_REG = X_Input_REG / Y_Input_REG;

    displayOnLEDs(Display_Result_REG);  
    
    // Reset variables to 0. Similar to resetAll but resetAll turns LEDs off too
    // Not necessarily needed as reset when * is pressed, just precaution
    X_Input_REG = 0;
    Y_Input_REG = 0;
    digitCount = 0;
    isSecond = 0;
}

 /*
  * This function is used to handle the input from the keypad. If it is a digit
  * it places it it in its corresponding operand variable. If it is a letter, it 
  * places it in the operation variable. If it is # it calculates. And if it is 
  * *, then it resets everything.
  * params: key: the input from the keyboard
  * return: none
  */
void handleInput(char key) {
    // If digit
    if (key >= '0' && key <= '9') {
        if (isSecond == 0) {
            X_Input_REG = (X_Input_REG * 10) + (key - '0');
            digitCount++;
            if (digitCount == 2) {
                isSecond = 1;
                digitCount = 0;
                PORTD = 0x01; // LED1 ON
            }
        } else {
            Y_Input_REG = (Y_Input_REG * 10) + (key - '0');
            digitCount++;
            if (digitCount == 2) {
                digitCount = 0;
                PORTD = 0x02; // LED2 ON
            }
        }
    }

    // If operation key
    else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
        Operation_REG = key;
    }

    // If '#' = calculate
    else if (key == '#') {
        PORTD = 0x00;
        calculate();
    }

    // If '*' = reset
    else if (key == '*') {
        resetAll();
    }
}

/*
 * This function is the main function. It does initialization/configuration
 * and then infinitely loops through checking the keypad for inputs and does
 * operations as needed.
 * params: none
 * return: knone
 */
void main(void) {
    setup();
    resetAll();

    while (1) {
        char key = getKeyPressed();     // Variable for key pressed
        if (key != 0) {             
            handleInput(key);
            __delay_ms(300);            // Not really needed as we have the next line (precaution))
            while (getKeyPressed());    // Wait until key released for no issues
        }
    }
}