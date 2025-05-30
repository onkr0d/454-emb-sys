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
// do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <libpic30.h>
#include <stdio.h>
#include <uart.h>
// #include "outcompare.h"

#include <math.h>
#include <stdbool.h>
#include <xc.h>  // Contains Nop() declaration

#include "adc.h"
#include "joystick.h"
#include "lcd.h"
#include "led.h"
#include "types.h"
#include "stdlib.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF &POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protecd_printf(ction
_FGS(GCP_OFF);

/*
 * Switches the ADC sampling between X and Y axis of the Amazing Board System (ABS).
 * Does not sample the ABS - you should switchToXAxis() first, then sampleJoystick().
 */
void switchToXAxis(bool xAxis) {
    if (xAxis) {
        // Switch to x-axis
        SETBIT(LATEbits.LATE2);
        SETBIT(LATEbits.LATE3);
        CLEARBIT(LATEbits.LATE1);

        AD1CHS0bits.CH0SA = 15; // set ADC to Sample AN4 pin
    } else {
        // Switch to y-axis
        SETBIT(LATEbits.LATE1);
        CLEARBIT(LATEbits.LATE3);
        CLEARBIT(LATEbits.LATE2);


        AD1CHS0bits.CH0SA = 9; // set ADC to Sample AN5 pin
    }
    // important: delay to clear channels
    //        t2cycle( 2 );
    __delay_ms(50);

}

int compareInts(const void *a, const void *b) {
    unsigned int int_a = *(const unsigned int *) a;
    unsigned int int_b = *(const unsigned int *) b;
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

/*
 * Samples the Amazing Ball System (ABS) and returns the value.
 * Should be preceded by a call to switchToXAxis().
 */
unsigned int sampleABS() {
    SETLED(LED4_PORT);

    int numSamples = 0;
    unsigned int samples[5] = {0, 0, 0, 0, 0};

    while (numSamples < 5) {

        // Joystick value sampling code
        SETBIT(AD1CON1bits.SAMP); // start to sample
        // wait for sampling or no?
        //        CLEARBIT(AD1CON1bits.SAMP); // start to sample

        __delay_ms(5);
        CLEARBIT(AD1CON1bits.SAMP);

        while (!AD1CON1bits.DONE); // wait for conversion to finish
        // reduce risk of ADC sampling
        unsigned int val = ADC1BUF0;
        CLEARBIT(AD1CON1bits.DONE); // MUST HAVE! clear conversion done bit
        samples[numSamples] = val;
        numSamples++;
    }

    // median of samples
    qsort(samples, 5, sizeof (unsigned int), compareInts);
    CLEARLED(LED4_PORT);
    return samples[2];
}

struct cornerPos {
    unsigned int x;
    unsigned int y;
};

void t2cycle(uint16_t amount) { // wait 20 ms
    uint16_t tStart;

    while (amount--) {
        tStart = TMR2;
        while (tStart == TMR2);

        while (tStart != TMR2);


    }
}

int main() {
    /* LCD Initialization Sequence */
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();

    // LED setup
    CLEARBIT(LED1_TRIS); // Set Pin to Output
    CLEARBIT(LED2_TRIS); // Set Pin to Output
    CLEARBIT(LED3_TRIS); // Set Pin to Output
    CLEARBIT(LED4_TRIS); // Set Pin to Output
    CLEARBIT(LED5_TRIS); // Set Pin to Output


    // lab manual page 29, for x axis touch screen
    {
        CLEARBIT(TRISEbits.TRISE1);
        CLEARBIT(TRISEbits.TRISE2);
        CLEARBIT(TRISEbits.TRISE3);

        CLEARBIT(LATEbits.LATE1);
        SETBIT(LATEbits.LATE2);
        SETBIT(LATEbits.LATE3);
    }
    // disable ADC
    CLEARBIT(AD1CON1bits.ADON);

    // initialize TRIS pins as inputs
    SETBIT(TRISBbits.TRISB15); // Reads RB15 (x-axis)
    SETBIT(TRISBbits.TRISB9); // Reads RB9 (y-axis)

    {
        // set pins to be analog, 3.6.1 from lab manual, page 15
        CLEARBIT(AD1PCFGLbits.PCFG15); // analog for x-axis
        CLEARBIT(AD1PCFGLbits.PCFG9); // analog for y-axis
    }
    
    // Configure AD1CON2
    SETBIT(AD1CON1bits.AD12B); // choose 12 bit operation mode
    AD1CON1bits.FORM = 0; // set integer output
    AD1CON1bits.SSRC = 0x7; // set automatic conversion

    // Configure AD1CON2
    AD1CON2 = 0; // not using scanning sampling

    // Configure AD1CON3
    CLEARBIT(AD1CON3bits.ADRC); // internal clock source
    AD1CON3bits.SAMC = 0x1F; // sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2; // Tad = 3Tcy (Time cycles)

    // enable ADC
    SETBIT(AD1CON1bits.ADON);

    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0;
    T2CONbits.TCKPS = 0b10;
    PR2 = 4000;
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE);

    SETBIT(T2CONbits.TON);

    AD2CON3 = 0x0002; //manual sample, from example Example 16-1 in dsPIC33F.16_-_ADC


    //lcd for debug
    CLEARBIT(LED4_TRIS); // Set Pin to Output

    lcd_clear();

    int cornerCounter = 0;
    struct cornerPos corner0 = {.x = 0, .y = 0};
    struct cornerPos corner1 = {.x = 0, .y = 0};
    struct cornerPos corner2 = {.x = 0, .y = 0};
    struct cornerPos corner3 = {.x = 0, .y = 0};

    SETLED(LED4_PORT);

    while (true) {
        int corner = cornerCounter % 4;

        switch (corner) {
            case 0:
                CLEARBIT(TRISDbits.TRISD6);
                OC7R = 3580;
                OC7RS = 3580;
                OC7CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);

                CLEARBIT(TRISDbits.TRISD7);
                OC8R = 3580;
                OC8RS = 3580;
                OC8CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);
                break;

            case 1:
                CLEARBIT(TRISDbits.TRISD6);
                OC7R = 3800;
                OC7RS = 3800;
                OC7CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);

                CLEARBIT(TRISDbits.TRISD7);
                OC8R = 3580;
                OC8RS = 3580;
                OC8CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);
                break;

            case 2:
                CLEARBIT(TRISDbits.TRISD6);
                OC7R = 3800;
                OC7RS = 3800;
                OC7CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);

                CLEARBIT(TRISDbits.TRISD7);
                OC8R = 3800;
                OC8RS = 3800;
                OC8CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);
                break;

            case 3:
                CLEARBIT(TRISDbits.TRISD6);
                OC7R = 3580;
                OC7RS = 3580;
                OC7CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);

                CLEARBIT(TRISDbits.TRISD7);
                OC8R = 3800;
                OC8RS = 3800;
                OC8CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);
                break;
        }

        __delay_ms(1850); // not exactly 2 seconds because switching axis adds a 50ms delay, plus other stuff. double check this number before demo!

        lcd_clear();
        lcd_locate(0, 0);
        lcd_printf("C1: X=%04d, Y=%04d %s", corner0.x, corner0.y, corner == 0 ? "<-" : "");
        lcd_locate(0, 0);
        lcd_locate(0, 1);
        lcd_printf("C2: X=%04d, Y=%04d %s", corner1.x, corner1.y, corner == 1 ? "<-" : "");
        lcd_locate(0, 1);
        lcd_locate(0, 2);
        lcd_printf("C3: X=%04d, Y=%04d %s", corner2.x, corner2.y, corner == 2 ? "<-" : "");
        lcd_locate(0, 2);
        lcd_locate(0, 3);
        lcd_printf("C4: X=%04d, Y=%04d %s", corner3.x, corner3.y, corner == 3 ? "<-" : "");
        lcd_locate(0, 3);

        switch (corner) {
            case 0:
                switchToXAxis(true);
                corner0.x = sampleABS();
                switchToXAxis(false);
                corner0.y = sampleABS();
                break;
            case 1:
                switchToXAxis(true);
                corner1.x = sampleABS();
                switchToXAxis(false);
                corner1.y = sampleABS();
                break;
            case 2:
                switchToXAxis(true);
                corner2.x = sampleABS();
                switchToXAxis(false);
                corner2.y = sampleABS();
                break;
            case 3:
                switchToXAxis(true);
                corner3.x = sampleABS();
                switchToXAxis(false);
                corner3.y = sampleABS();
                break;
        }

        // move to corner






        cornerCounter++;

        // below: buncha crap to 'flush' out the buffer, or whatever
        //        switchToXAxis(true);
        //        sampleABS();
        //        switchToXAxis(false);
        //        sampleABS();

        //        t2cycle( 5 * 50 );

        //        switchToXAxis(true);
        //        sampleABS();
        //        switchToXAxis(false);
        //        sampleABS();
    }
    return 0;
}
