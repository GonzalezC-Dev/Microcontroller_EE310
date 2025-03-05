;---------------------
; Title: Waveform Generator with Delay
;---------------------
; Program Details:
; The purpose of this program is to demonstrate how to call a delay function. 

; Inputs: Inner_loop ,Outer_loop 
; Outputs: PORTD
; Date: Feb 24, 2024
; File Dependencies / Libraries: None 
; Compiler: xc8, 2.4
; Author: Farid Farahmand
; Versions:
;       V1.3: Changes the loop size
; Useful links: 
;       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
;       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
;       List of Instrcutions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 


;---------------------
; Initialization - make sure the path is correct
;---------------------
#include ".\myConfigFile.inc"
;#include "C:\Users\student\Documents\myMPLABXProjects\ProjectFirstAssemblyMPLAB\FirstAssemblyMPLAB.X\AssemblyConfig.inc"

#include <xc.inc>

;---------------------
; Program Inputs
;---------------------
Inner_loop  equ 5 // in decimal
Outer_loop  equ 5
 
;---------------------
; Program Constants
;---------------------
REG10   equ     10h   // in HEX
REG11   equ     11h
REG01   equ     1h

;---------------------
; Definitions
;---------------------
#define SWITCH    LATD,2  
#define LED0      PORTD,0
#define LED1	  PORTD,1

;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld        ; Do not change
    
    ORG          0                ;Reset vector
    GOTO        _start1

    ORG          0020H           ; Begin assembly at 0020H

_start1:
    MOVLW       0b11111100	;binary number moved to WREG (WREG = 0b11111100 = 0xFC)
    MOVWF       TRISD,0		;output PORTD can only adjust bit 0 and 1
    MOVLW       0b11111110	;binary number moved to WREG (WREG = 0b11111110 = 0xFC)
    MOVWF       REG01,0		;copy WREG to register 1

_onoff:
    MOVFF       REG01,PORTD	;copy register 1 value to PORTD so PORTD1 set
    MOVLW       Inner_loop	;copy Inner_loop value to WREG
    MOVWF       REG10		;copy WREG value to REG10 for loop
    MOVLW       Outer_loop	;copy Outer_loop value to WREG
    MOVWF       REG11		;copy WREG value to REG11 for another loop (not used)

_loop1:
    DECF        REG10,1		;decrement REG10 by 1
    BNZ         _loop1		;loop until REG10 equals zero (delay)
    ;DECF        REG11,1 // outer loop
    ;BNZ        _loop1
    
    COMF        REG01,1		;negate the register 
    BRA         _onoff		;branch to _onoff label to flip
END


