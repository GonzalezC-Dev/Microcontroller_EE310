;-------------------------
; Title: 7-Segment Counter
;-------------------------
; Program Details:
; The purpose of this project is to create a system that can increment, decrement, and reset 
;    a displayed number based on user input. The 7-segment display, connected directly to PORTB, shows 
;    the current count (0-15 in hexadecimal). Switch A (RA0) increments the count, 
;    Switch B (RA1) decrements it, and pressing both switches resets it to zero. The 
;    program uses assembly language with a lookup table for segment encoding, a delay 
;    function using CALL, and table pointers for efficiency.
    
    
; Inputs: RA1 (decrement switch), RA0 (increment switch)
; Outputs: PORTB(RB0-RB7)
; Setup: The Curiosity Board, Common Cathode 7-Segment Display, Breadboard Power Supply (9v battery)
;   Microchip: PIC18F46K42
    
; Date: Mar 25, 2025
; File Dependencies / Libraries: It is required to include the 
;   myConfigFile.inc in the Header Folder
; Compiler: pic-as, 3.0
; System Information:
;   Name:Dell Inspiron 16 Plus 7630
;   OS:  Windows 11
;   CPU: 13th Gen Intel i7-13700H, 2400Mhz, 14 Cores, 20 Logical Processors
;   RAM: 32 GB
; Author: Christian Gonzalez
; Versions:
;       V1.0: First implementation
; Useful links: 
;       Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
;       PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
;       List of Instrcutions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 

; === Initialization ===
#include "./myConfigFile.inc"
#include <xc.inc>
    
; === Program Inputs ===
Inner_loop  equ 255 // in decimal
Middle_loop equ	255
Outer_loop  equ 4
  
; === Program Constants ===
REG10   equ     10h   // in HEX
REG11   equ     11h
REG12	equ	12h
   
; === Definitions ===
#define	SW_A	PORTD, 0  ; RA0 (Switch A - Increment)
#define SW_B	PORTD, 1  ; RA1 (Switch B - Decrement)

; === Main Program ===   
    PSECT absdata,abs,ovrld ; Do not change
    
    ORG	    0               ;Reset vector

    ORG 0x100  ; starting address of table

; === Segment Table ===
SEG_TABLE:
    ; the following two lines place the values 0 through F
    ; (based on the 7-segment display output needed) starting
    ; at address 0x100 through 0x10F
    DB 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07
    DB 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71

    ORG 0x200  ; start of program, placed far for no problems

; === Initialize ===
START:
    CLRF    TBLPTRU	    ; ensure upper byte of table pointer is clear
    MOVLW   HIGH SEG_TABLE  ; the high byte where SEG_TABLE starts
    MOVWF   TBLPTRH	    ; set high byte of table pointer to 0x10
    MOVLW   LOW SEG_TABLE   ; the low byte where SEG_TABLE starts
    MOVWF   TBLPTRL	    ; set low byte of table pointer to 0x00

; === Setup PORTB for 7-segment display ===
    BANKSEL	PORTB	    ; select bank for PORTB register
    CLRF	PORTB	    ; clear PORTB
    BANKSEL	LATB	    ; select bank for LATB register
    CLRF	LATB	    ; clear LATB
    BANKSEL	ANSELB	    ; select bank for ANSELB register
    CLRF	ANSELB	    ; set PORTB as digital
    BANKSEL	TRISB	    ; select bank for TRISB register
    MOVLW	0b00000000  ; load WREG with 0
    MOVWF	TRISB	    ; set RB[7:0] as outputs
    
; === Setup PORTD for switches ===
    BANKSEL PORTD	; select bank for PORTD register
    CLRF    PORTD	; clear PORTD
    BANKSEL LATD	; select bank for LATD register
    CLRF    LATD	; clear LATD
    BANKSEL ANSELD	; select bank for ANSELD register
    CLRF    ANSELD	; set PORTD as digital
    BANKSEL TRISD	; select bank for TRISD register
    MOVLW   0b00000011	; load WREG with 0b00000011
    MOVWF   TRISD	; set RD[1:0] as inputs
    TBLRD*		; read table memory at TBLPTR
    MOVF    TABLAT, 0	; move first table value to WREG
    MOVWF   PORTB	; output to 7-segment display
    CALL    DELAY	; wait before updating display

CHECK_SWITCHES:
    ; check switch A (PORTD, 0)
    BTFSC   SW_A	    ; skip next instruction if Switch A is not pressed
    GOTO    CHECK_B_WHILE_A ; if switch A is pressed, check switch B next

    ; if switch A is NOT pressed, check switch B
    BTFSC   SW_B	    ; skip next instruction if switch B is not pressed
    CALL    DECREMENT       ; call DECREMENT if only switch B is pressed
    GOTO    CHECK_SWITCHES  ; loop back if neither switch is pressed

CHECK_B_WHILE_A:
    ; if switch A is pressed, check if switch B is also pressed
    BTFSS   SW_B	    ; skip next instruction if switch B is also pressed
    CALL    INCREMENT       ; if only switch A is pressed, call INCREMENT

    CALL    RESET_COUNTER   ; if both switches are pressed, call RESET_COUNTER
    GOTO    CHECK_SWITCHES  ; loop back to continue checking

INCREMENT:
    MOVLW   0x0F	    ; max limit for TBLPTRL (end of table)
    CPFSLT  TBLPTRL	    ; if TBLPTRL is at max, reset pointer to start
    CALL    TO_START	    ; reset pointer to beginning if at end
    TBLRD+*		    ; increment table pointer, then read value
    MOVF    TABLAT, 0	    ; move table value to WREG
    MOVWF   PORTB	    ; output to 7-segment display
    CALL    DELAY	    ; wait before allowing another input
    GOTO    CHECK_SWITCHES  ; continue checking for inputs

DECREMENT:
    MOVLW   0x00	    ; min limit for TBLPTRL (start of table)
    CPFSGT  TBLPTRL	    ; if at start, need to wrap around to end
    CALL    TO_END	    ; have table pointer point at end + 1
    DECF    TBLPTRL, F	    ; TBLPTRL = TBLPTRL - 1
    TBLRD*		    ; read value at TBLPTR
    MOVF    TABLAT, 0	    ; move table value to WREG
    MOVWF   PORTB	    ; output to 7-segment display
    CALL    DELAY	    ; wait before allowing another input
    GOTO    CHECK_SWITCHES  ; continue checking for inputs

RESET_COUNTER:
    MOVLW   HIGH SEG_TABLE  ; WREG = high byte of SEG_TABLE
    MOVWF   TBLPTRH	    ; set high byte of table pointer to SEG_TABLE high byte
    MOVLW   LOW SEG_TABLE   ; WREG = low byte of SEG_TABLE
    MOVWF   TBLPTRL	    ; set low byte of table pointer to SEG_TABLE low byte
    TBLRD*		    ; read value that table pointer is pointing to
    MOVF    TABLAT, 0	    ; move table value to WREG
    MOVWF   PORTB	    ; output to 7-segment display
    CALL    DELAY           ; wait before updating display
    RETURN

TO_START:
    ; moves table pointer back to the beginning of SEG_TABLE - 1 (bc we use pre-increment)
    MOVLW   0x00    ; SEG_TABLE address - 1 is equal to 0x00 for high byte
    MOVWF   TBLPTRH ; set high byte of table pointer to SEG_TABLE - 1 high byte
    MOVLW   0xFF    ; SEG_TABLE address - 1 is equal to 0xFF for low byte
    MOVWF   TBLPTRL ; set low byte of table pointer to SEG_TABLE - 1 low byte
    RETURN
    
TO_END:
    ; moves table pointer to the end of SEG_TABLE + 1 (bc we use pre-decrement)
    MOVLW   0x01    ; SEG_TABLE end address + 1 is equal to 0x01 for high byte
    MOVWF   TBLPTRH ; set high byte of table pointer to SEG_TABLE end + 1 high byte
    MOVLW   0x10    ; SEG_TABLE end address + 1 is equal to 0x10 for low byte
    MOVWF   TBLPTRL ; set low byte of table pointer to SEG_TABLE end + 1 low byte
    RETURN

; ==== Delay Subroutine ====
DELAY:
    MOVLW   Inner_loop	; load inner loop value
    MOVWF   REG10       ; store in REG10
    MOVLW   Middle_loop	; load middle loop value
    MOVWF   REG11       ; store in REG11
    MOVLW   Outer_loop  ; load outer loop value
    MOVWF   REG12       ; store in REG12

_loop1:
    DECF    REG10, 1    ; decrement inner loop counter
    BNZ     _loop1      ; repeat until inner loop counter is zero
    MOVLW   Inner_loop  ; re-initialize the inner loop when outer loop decrements
    MOVWF   REG10	; copy WREG value to REG10
    DECF    REG11, 1    ; decrement middle loop counter
    BNZ     _loop1      ; repeat until middle loop counter is zero
    MOVLW   Middle_loop ; re-initialize the middle loop when outer loop decrements
    MOVWF   REG11	; copy WREG value to REG11
    DECF    REG12, 1    ; decrement outer loop counter
    BNZ     _loop1      ; repeat until outer loop counter is zero
    RETURN              ; return from delay subroutine

    END
