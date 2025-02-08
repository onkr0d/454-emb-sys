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
    

    CLEARBIT(LED1_TRIS); // Set Pin to Output
    CLEARBIT(LED2_TRIS); // Set Pin to Output
    CLEARBIT(LED3_TRIS); // Set Pin to Output
    CLEARBIT(LED4_TRIS); // Set Pin to Output
    
    // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
    SETBIT(TRISEbits.TRISE8);
    SETBIT(INTCON2bits.INT1EP);
    SETBIT(AD1PCFGHbits.PCFG20);
    
    unsigned int buttonCount = 0;
    unsigned int holdBuffer = 0;
    unsigned int alreadyPressed = 0;
    
//  Timer 2 code
    CLEARBIT( T2CONbits.TON ) ;
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT( T2CONbits.TGATE);
    TMR2 = 0;
    T2CONbits.TCKPS = 0b11;
    CLEARBIT(IFS0bits.T2IF) ;
    CLEARBIT( IEC0bits.T2IE);
    PR2 = 16000 ; 
    SETBIT(T2CONbits.TON);
    SETBIT( IEC0bits.T2IE);
   
    // Timer 1 code
    //enable LPOSCEN
    __builtin_write_OSCCONL(OSCCONL | 2);
    T1CONbits.TON = 0; //Disable Timer
    T1CONbits.TCS = 1; //Select external clock
    T1CONbits.TSYNC = 0; //Disable Synchronization
    T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
    TMR1 = 0x00; //Clear timer register
    PR1 = 32767; //Load the period value
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1;// Enable Timer1 interrupt
    T1CONbits.TON = 1;// Start Timer
    
      
      
    int flip0 = 0;
    while (1) {
  
        flip0 = 1 - flip0;
        if( flip0 ) {
            SETLED( LED4_PORT );
            Nop();
            __delay_ms(300); 
            }
        else {
            CLEARLED(LED4_PORT);
            Nop();
            __delay_ms(300); 
        }
    }
    
    
    return 0;
}

int flip2 = 0;

void __attribute__(( __interrupt__ )) _T2Interrupt(void)
{
    flip2 = 1 - flip2;
    if( flip2 ) {
        SETLED( LED1_PORT );
        Nop();
        }
    else {

        CLEARLED(LED1_PORT);
        Nop();
    }    
 CLEARBIT(IFS0bits.T2IF) ;
    
    
}

int flip1 = 0;
void __attribute__(( __interrupt__ )) _T1Interrupt(void)
{
    flip1 = 1 - flip1;
    if( flip1) {
        SETLED( LED2_PORT );
        Nop();
        }
    else {

        CLEARLED(LED2_PORT);
        Nop();
    }    
 CLEARBIT(IFS0bits.T1IF) ;
    
    
}