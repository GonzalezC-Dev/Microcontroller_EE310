//-----------------------------
// Title: HVAC Control System
//-----------------------------
// Purpose: To determine if an HVAC System shall turn on cooling, heating, or neither. 
// Dependencies: NONE
// Compiler: xc8, v3.00
// Author: Christian Gonzalez
// OUTPUTS: PORTD 
// INPUTS: measuredTemp, refTemp 
// Versions:
//  	V1.0: Mar 11, 2025 - First version
//-----------------------------
    
;---------------------
; Initialization - make sure the path is correct
;---------------------
#include ".\myConfigFile.inc"
#include <xc.inc>
    
;----------------
; PROGRAM INPUTS
;----------------
;The DEFINE directive is used to create macros or symbolic names for values.
;It is more flexible and can be used to define complex expressions or sequences of instructions.
;It is processed by the preprocessor before the assembly begins.

#define  measuredTempInput 	60 ; this is the input value
#define  refTempInput	 	10 ; this is the input value

;---------------------
; Definitions
;---------------------
#define SWITCH    LATD,0  
#define LED1      PORTD,1
#define LED2	  PORTD,2
#define LED3	  PORTD,3
    
;---------------------
; Memory Register Assignments
;---------------------
NUME    EQU   0x23  ; register for number being converted
QU      EQU   0x24  ; reg for quotient
RMND_L	EQU   0x25  ; reg for least significant remainder (ones place)
RMND_M  EQU   0x26  ; reg for middle remainder (tens place)
RMND_H  EQU   0x27  ; reg for most significant remainder (hundreds place)

refTemp		EQU 0x20    ; reg for reference temp
measuredTemp	EQU 0x21    ; reg for measured temp
contReg		EQU 0x22    ; reg for control

negStatus   EQU 0x28	; reg tracks if we have a negative measured temp
   
;---------------------
; Program Constants
;---------------------
; The EQU (Equals) directive is used to assign a constant value to a symbolic name or label.
; It is simpler and is typically used for straightforward assignments.
;It directly substitutes the defined value into the code during the assembly process.
    
MYDEN	equ   10    ; decimal divisor

;---------------------
; Main Program
;---------------------
    PSECT absdata,abs,ovrld        ; Do not change
		
;---------------------
; Start Program Memory at 0x20
;---------------------
    ORG	    0x20
    GOTO    START

;---------------------
; Hex-to-Decimal Conversion Subroutine
;---------------------
CONVERT_DECIMAL:
    MOVLW   MYDEN
    CLRF    QU, 1
D_1:
    INCF    QU, 1
    SUBWF   NUME
    BC	    D_1
    ADDWF   NUME
    DECF    QU, 1
    MOVFF   NUME, RMND_L  ; store ones place
    MOVFF   QU, NUME
    CLRF    QU
D_2:
    INCF    QU, 1
    SUBWF   NUME
    BC	    D_2
    ADDWF   NUME
    DECF    QU, 1
    MOVFF   NUME, RMND_M  ; store tens place
    MOVFF   QU, RMND_H    ; store hundreds place
    RETURN
    
START:
    
    BANKSEL ANSELD   ; select the correct bank for ANSELD
    CLRF    ANSELD   ; set PORTD to digital mode (disable analog functions)

    BANKSEL TRISD    ; select the correct bank for TRISD
    CLRF    TRISD    ; set PORTD as output (0 = output, 1 = input)

    BANKSEL LATD     ; select the correct bank for LATD
    CLRF    LATD     ; clear LATD to ensure no previous states affect it

    
;---------------------
; Load Inputs
;---------------------
    MOVLW   refTempInput	
    MOVWF   refTemp
    MOVLW   measuredTempInput	
    MOVWF   measuredTemp
    
;---------------------
; Check if refTemp is within range (10 deg celsius to 50 deg celsius)
;---------------------
    MOVLW   10		; load lower limit (10 deg celsius)
    SUBWF   refTemp, 0  ; refTemp - 10
    BTFSS   STATUS, 0   ; if C is clear, refTemp < 10
    GOTO    ERROR_STATE ; out of range = ERROR

    MOVLW   51		; load upper limit (50 deg celsius)
    SUBWF   refTemp, 0  ; refTemp - 50
    BTFSC   STATUS, 0   ; if C is set, refTemp > 50
    GOTO    ERROR_STATE	; out of range = ERROR

;---------------------
; Validate measuredTemp Range (-10 measuredTemp 60)
;---------------------
    BTFSC   measuredTemp,7   ; check if negative
    GOTO    NEG_CHECK

    MOVLW   61		    ; load upper limit (60 deg celsius)
    SUBWF   measuredTemp, 0 ; refTemp - 60
    BTFSC   STATUS, 0	    ; if C is set, refTemp > 60
    GOTO    ERROR_STATE	    ; out of range = ERROR
    GOTO    CONT_PROG

NEG_CHECK:		    ; used if we have a negative measured temp
    MOVLW   0x01
    MOVWF   negStatus	    ; set the negStatus register
    NEGF    measuredTemp    ; 2's comp of measuredTemp
    MOVLW   11		    ; load limit (-10, but here 10 since we did NEGF)
    SUBWF   measuredTemp, 0 ; measuredTemp - 10
    BTFSC   STATUS, 0	    ; if C is set, measuredTemp > 10 (or less than -10)
    GOTO    ERROR_STATE	    ; out of range = ERROR
    GOTO    CONT_PROG	    ; both values are valid, continue

;---------------------
; ERROR STATE: Handle out-of-range values
;---------------------
ERROR_STATE:
    BSF	    LED3	; turn on an error LED
    GOTO    ERROR_STATE ; stay here forever

CONT_PROG:

;---------------------
; Convert refTemp to Decimal
;---------------------
    MOVFF   refTemp, NUME   ; store refTemp in NUME
    CALL    CONVERT_DECIMAL ; convert refTemp

    MOVFF   RMND_L,0x60 ; store ones place
    MOVFF   RMND_M,0x61 ; store tens place
    MOVFF   RMND_H,0x62	; store hundreds place

;---------------------
; Convert measuredTemp to Decimal
;---------------------
    MOVFF   measuredTemp, NUME	; store measuredTemp in NUME
    CALL    CONVERT_DECIMAL	; convert measuredTemp

    MOVFF   RMND_L,0x70	; store ones place
    MOVFF   RMND_M,0x71 ; store tens place
    MOVFF   RMND_H,0x72 ; store hundreds place
    
    ;MOVLW   measuredTempInput	; used to replace back to negative value
    ;MOVWF   measuredTemp

;---------------------
; HVAC Control Logic for Positive Temps
;---------------------
HVAC_SYS:
    BTFSC   negStatus, 0    ; check if we have a negative measuredTemp
    GOTO    HVAC_NEG_SYS
    
    MOVF    refTemp, 0      ; load refTemp into W
    SUBWF   measuredTemp, 0 ; perform W = measuredTemp - refTemp

    BTFSC   STATUS, 2       ; if Zero flag is set, measuredTemp == refTemp
    GOTO    LED_OFF         ; turn off both heating & cooling

    BTFSC   STATUS, 0       ; if Carry flag is set, measuredTemp >= refTemp
    GOTO    LED_COOL        ; measuredTemp > refTemp, so turn on cooling

    GOTO    LED_HEAT        ; else: measuredTemp < refTemp, so turn on heating
    
;---------------------
; HVAC Control Logic With Negative Measured Temp
;---------------------
HVAC_NEG_SYS:		; measuredTemp is negative so turn on HEAT
    GOTO    LED_HEAT

;---------------------
; Turn on Cooling (measuredTemp > refTemp)
;---------------------
LED_COOL:
    MOVLW   2
    MOVWF   contReg	; set the control register appropriately
    BSF     LED2        ; turn on cooling
    BCF     LED1        ; ensure heating is off
    GOTO    END_LOGIC	

;---------------------
; Turn on Heating (measuredTemp < refTemp)
;---------------------
LED_HEAT:
    MOVLW   1
    MOVWF   contReg	; set control register appropriately
    BSF     LED1        ; turn on heating
    BCF     LED2        ; ensure cooling is off
    GOTO    END_LOGIC

;---------------------
; Turn off both (measuredTemp == refTemp)
;---------------------
LED_OFF:
    MOVLW   0
    MOVWF   contReg ; set control register appropriately
    BCF     LED1    ; ensure heating is off
    BCF     LED2    ; ensure cooling is off

END_LOGIC:
    NOP
    SLEEP
END