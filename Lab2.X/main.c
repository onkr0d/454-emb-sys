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
#include "adc.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

unsigned long milliseconds = 0;

int main() {
    /* LCD Initialization Sequence */
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();


    CLEARBIT(LED1_TRIS); // Set Pin to Output
    CLEARBIT(LED2_TRIS); // Set Pin to Output
    CLEARBIT(LED3_TRIS); // Set Pin to Output
    CLEARBIT(LED4_TRIS); // Set Pin to Output

    
/*   
    
    // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
    IEC1bits.INT1IE = 0; // Disable Timer1 interrupt
    SETBIT(TRISEbits.TRISE8);
    SETBIT(INTCON2bits.INT1EP);
    
    SETBIT(AD1PCFGHbits.PCFG20);
    INTCON2bits.INT1EP = 0; // Trigger on falling edge
    IEC1bits.INT1IE = 1; // Enable Timer1 interrupt

    */

    
    
    
    //disable ADC
    CLEARBIT(AD2CON1bits.ADON);
    //initialize PIN
    SETBIT(TRISBbits.TRISB4); //set TRISB RB15 to input
    CLEARBIT(AD2PCFGLbits.PCFG4); //set AD1 AN15 input pin as analog
    //Configure AD1CON1
    CLEARBIT(AD2CON1bits.AD12B) ; //set 10b Operation Mode
    AD2CON1bits.FORM = 0; //set integer output
    AD2CON1bits.SSRC = 0x7; //set automatic conversion
    //Configure AD1CON2
    AD2CON2 = 0; //not using scanning sampling
    //Configure AD1CON3
    CLEARBIT(AD2CON3bits.ADRC); //internal clock source
    AD2CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave AD1CON4 at its default value
    //enable ADC
    SETBIT(AD2CON1bits.ADON);


 uint16_t val = 3 ;   

     
lcd_locate(0, 1);
lcd_printf( "  version D  \n"  ) ;
lcd_locate(0, 1);
 
 
    
AD2CHS0bits.CH0SA = 4; //set ADC to Sample AN15 pin
SETBIT(AD2CON1bits.SAMP); //start to sample
while(!AD2CON1bits.DONE); //wait for conversion to finish
CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
val = ADC2BUF0; //return sample
    
lcd_locate(0, 2);
lcd_printf( "  %X  \n" ,    val ) ;
lcd_locate(0, 2);

while (1) ;

    return 0;
}

