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

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF &POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

/**
 * Switches the ADC sampling between X and Y axis of the joystick.
 * Does not sample the joystick - you should switchToXAxis() first, then sampleJoystick().
 */
void switchToXAxis(bool xAxis) {
    if (xAxis) {
        // Read x-axis
        AD2CHS0bits.CH0SA = 15; // set ADC to Sample AN4 pin
    } else {
        // Read y-axis
        AD2CHS0bits.CH0SA = 9; // set ADC to Sample AN5 pin
    }
}

/**
 * Samples the joystick and returns the value.
 * Should be preceded by a call to switchToXAxis().
 */
int sampleJoystick() {
    // Joystick value sampling code
    SETBIT(AD2CON1bits.SAMP); // start to sample
    while (!AD2CON1bits.DONE); // wait for conversion to finish
    // reduce risk of ADC sampling
    int ret = ADC2BUF0;
    CLEARBIT(AD2CON1bits.DONE); // MUST HAVE! clear conversion done bit
    return ret;
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

    // lab manual page 29, for x axis touch screen
    {
        CLEARBIT(TRISEbits.TRISE1);
        CLEARBIT(TRISEbits.TRISE2);
        CLEARBIT(TRISEbits.TRISE3);

        SETBIT(LATEbits.LATE1);
        CLEARBIT(LATEbits.LATE2);
        CLEARBIT(LATEbits.LATE3);
    }
    // disable ADC
    CLEARBIT(AD2CON1bits.ADON);

    // initialize TRIS pins as inputs
    SETBIT(TRISBbits.TRISB4); // Reads RB4 (x-axis)
    SETBIT(TRISBbits.TRISB5); // Reads RB5 (y-axis)


    {
        // set pins to be analog, 3.6.1 from lab manual, page 15
        CLEARBIT(AD2PCFGLbits.PCFG15); // analog for x-axis
        CLEARBIT(AD2PCFGLbits.PCFG9); // analog for y-axis
    }
    // Configure AD1CON2
    SETBIT(AD2CON1bits.AD12B); // choose 12 bit operation mode
    AD2CON1bits.FORM = 0; // set integer output
    AD2CON1bits.SSRC = 0x7; // set automatic conversion

    // Configure AD1CON2
    AD2CON2 = 0; // not using scanning sampling

    // Configure AD1CON3
    CLEARBIT(AD2CON3bits.ADRC); // internal clock source
    AD2CON3bits.SAMC = 0x1F; // sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; // Tad = 3Tcy (Time cycles)

    // Leave AD1CON4 at its default value
    // enable ADC
    SETBIT(AD2CON1bits.ADON);

//    CLEARBIT(T2CONbits.TON);
//    CLEARBIT(T2CONbits.TCS);
//    CLEARBIT(T2CONbits.TGATE);
//    TMR2 = 0;
//    T2CONbits.TCKPS = 0b10;
//    CLEARBIT(IEC0bits.T2IE);
//    CLEARBIT(IEC0bits.T2IE);
//    PR2 = 4000;

//    CLEARBIT(TRISDbits.TRISD6);
//    OC7R = 3700;
//    OC7RS = 3700;
//    OC7CONbits.OCM = 0b110;
    //    SETBIT(T2CONbits.TON);

//    CLEARBIT(TRISDbits.TRISD7);
//    OC8R = 3700;
//    OC8RS = 3700;
//    OC8CONbits.OCM = 0b110;
//    SETBIT(T2CONbits.TON);

    lcd_clear();

    while (true) {
        // just get the x and y value of the ball location
        switchToXAxis(true);
        lcd_locate(0, 0);
        lcd_printf("x:%06d \n", sampleJoystick());
        lcd_locate(0, 0);


        __delay_ms(50);

        switchToXAxis(false);
        lcd_locate(0, 2);
        lcd_printf("y:%06d \n", sampleJoystick());
        lcd_locate(0, 2);

        __delay_ms(50);

        continue;
        //        uint16_t xservo = 0;
        //        uint16_t yservo = 0;
        //        int joystick_val = 0;
        //
        //        {
        //            // x move servo!
        //            switchToXAxis(true);
        //            int joystick_val = sampleJoystick();
        //            xservo = floor((joystick_val - x_min) * (double) 240 / (x_max - x_min) + 3580);
        //            OC7R = xservo;
        //            OC7RS = xservo;
        //            OC7CONbits.OCM = 0b110;
        //        }
        //
        //
        //        {
        //            // move y servo!
        //            switchToXAxis(false);
        //            joystick_val = sampleJoystick();
        //            yservo = floor((y_max - joystick_val) * (double) 240 / (y_max - y_min) + 3580);
        //            OC8R = yservo;
        //            OC8RS = yservo;
        //            OC8CONbits.OCM = 0b110;
        //        }

    }
    return 0;
}