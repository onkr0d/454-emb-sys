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

    // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
    IEC1bits.INT1IE = 0; // Disable Timer1 interrupt
    SETBIT(TRISEbits.TRISE8);
    SETBIT(INTCON2bits.INT1EP);
    SETBIT(AD1PCFGHbits.PCFG20);
    INTCON2bits.INT1EP = 0; // Trigger on falling edge
    IEC1bits.INT1IE = 1; // Enable Timer1 interrupt

    // Timer 2
    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0;
    T2CONbits.TCKPS = 0b11; //256
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE);
    PR2 = 50 - 1; // 1*10^-3 * 12.8*10^6 * 1/256
    SETBIT(T2CONbits.TON);
    SETBIT(IEC0bits.T2IE);

    // Timer 1
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
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    T1CONbits.TON = 1; // Start Timer


    // Timer 3 code
    CLEARBIT(T3CONbits.TON);
    CLEARBIT(T3CONbits.TCS);
    CLEARBIT(T3CONbits.TGATE);
    TMR3 = 0x00;
    T3CONbits.TCKPS = 0b01; // Prescaler
    CLEARBIT(IFS0bits.T3IF);
    CLEARBIT(IEC0bits.T3IE);
    PR3 = 0; // free run
    SETBIT(T3CONbits.TON);

    /* SETBIT(IEC0bits.T3IE); */

    // task 4
    unsigned int iters = 0;


    uint16_t newtime = 0, oldtime = 0, clk_cycles;
    double ms_time;

    int flip0 = 0, flip3 = 0;

    while (1) {
        // task 4 printing
        if (iters++ == 25000) {
            // reset iterations
            iters = 0;
            lcd_locate(0, 0);
            // FIXED very funny edge cases:
            // 1. when minutes > 0, and 0 < seconds < 10, the ms timer displays 4 spaces, ie, xxxx, instead of xxx
            // 2. after you reset it with the trigger (and seconds > 10), the above case changes to minutes >= 0. 
            // how is this possible? there should be no possible values > 1000, seeing as we mod % 1000.
            // only god knows.
            lcd_printf("since: %02lu:%02lu.%03lu",
                    (milliseconds / (1000UL * 60UL)),
                    (milliseconds / 1000UL) % 60,
                    milliseconds % 1000UL);
            // locate again because jank
            lcd_locate(0, 1);
            lcd_printf("cycles: %d  %.3f", clk_cycles, ms_time)
            lcd_locate(0, 1);
        }
        // time main loop

        newtime = TMR3;
        clk_cycles = newtime - oldtime;
        ms_time = (float) clk_cycles / 1600;


        /*        
        lcd_locate(0, 1);
        lcd_printf("%hx %2.4f", clk_cycles, ms_time);
        lcd_printf("%d  ", clk_cycles);
        lcd_locate(0, 2);
        lcd_printf("%d  \n", 4);
        lcd_locate(0, 1);
         */
        
        oldtime = newtime;

        flip0 = 1 - flip0;
        if (flip0) {
            SETLED(LED4_PORT);
            Nop();

        } else {
            CLEARLED(LED4_PORT);
            Nop();

        }
    }

    return 0;
}

int flip2 = 0;
int count2 = 0;

/**
 * Timer2 interrupt
 */
void __attribute__((__interrupt__)) _T2Interrupt(void) {
    milliseconds++;
    if (5 - 1 == count2++) {
        count2 = 0;

        flip2 = 1 - flip2;
        if (flip2) {
            SETLED(LED1_PORT);
            Nop();
        } else {
            CLEARLED(LED1_PORT);
            Nop();
        }
    }
    CLEARBIT(IFS0bits.T2IF);


}

/**
 * Timer 3 interrupt
 */
/**/
int T3toggle = 0;

void __attribute__((__interrupt__)) _T3Interrupt(void) {
    if (T3toggle = 1 - T3toggle) {
        SETLED(LED3_PORT);
        Nop();
    } else {
        CLEARLED(LED3_PORT);
        Nop();
    }

    CLEARBIT(IFS0bits.T3IF);
}





int flip1 = 0;

/**
 * Timer1 interrupt
 */
void __attribute__((__interrupt__)) _T1Interrupt(void) {
    flip1 = 1 - flip1;
    if (flip1) {
        SETLED(LED2_PORT);
        Nop();
    } else {

        CLEARLED(LED2_PORT);
        Nop();
    }
    CLEARBIT(IFS0bits.T1IF);
}

/**
 * Copy of Timer1 interrupt
 */
void __attribute__((__interrupt__)) _INT1Interrupt(void) {

    milliseconds = 0;

    CLEARBIT(IFS1bits.INT1IF);
}
