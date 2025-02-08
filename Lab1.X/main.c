/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: simple HelloWorld application     */
/*                for Amazing Ball platform         */
/*                                                  */
/****************************************************/

#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>
#include <uart.h>

#include "lcd.h"
#include "types.h"
#include "led.h"
#include <xc.h>   // Contains Nop() declaration
#include "joystick.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  


int main(){
	/* LCD Initialization Sequence */
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
    
    // Task 1: Display Group Name
    // lcd_locate (column, row)
	lcd_locate(0,0);
	lcd_printf("Tiger");
    
    lcd_locate(0,1);
    lcd_printf("Ivan");
    
    lcd_locate(0,2);
    lcd_printf("Tom");
    /*
     Task 2:
     * 1. Blink LED4 3 times
     * 2. 1 second between blinks
     * 3. LED4 off after 3 blinks
     */
//    int led4_count = 0;
//    CLEARBIT(LED4_TRIS); // Set Pin to Output
//    while(led4_count < 3) {
//        SETLED(LED4_PORT);
//        Nop();
//        __delay_ms(100);
//        
//        CLEARLED(LED4_PORT);
//        Nop();
//        __delay_ms(1000);
//        
//        led4_count++;
//    };
//    
    
    
    CLEARBIT(LED1_TRIS); // Set Pin to Output
    CLEARBIT(LED2_TRIS); // Set Pin to Output
    CLEARBIT(LED3_TRIS);
    
    // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
    SETBIT(TRISEbits.TRISE8);
    SETBIT(INTCON2bits.INT1EP);
    SETBIT(AD1PCFGHbits.PCFG20);
    
    unsigned int buttonCount = 0;
    unsigned int holdBuffer = 0;
    unsigned int alreadyPressed = 0;
    
    while (1) {
        if (BTN1_PRESSED()) {
            // no point in increasing the buffer past 10, set to 20 here for fun
            if (holdBuffer <= 20) {
                holdBuffer++;
            }
            if (holdBuffer > 10 && !alreadyPressed) {
                SETLED(LED1_PORT);
                Nop();
                buttonCount++;
                alreadyPressed = 1;
            }
        } else {
            holdBuffer = 0;
            alreadyPressed = 0;
            CLEARLED(LED1_PORT);
            Nop();
        }
        if (BTN2_PRESSED()) {
            SETLED(LED2_PORT); 
            Nop();
        } else {
            CLEARLED(LED2_PORT);
            Nop();
        }
        if (BTN1_PRESSED() ^ BTN2_PRESSED()) {
            SETLED(LED3_PORT);
        } else {
            CLEARLED(LED3_PORT);
        }
        lcd_locate(0,3);
        lcd_printf("%d %x", buttonCount, buttonCount);
        lcd_locate(0,3);
    }
    
  	
	/* Do nothing */
	while(1){

	}
    
    return 0;
}