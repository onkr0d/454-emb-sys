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

unsigned long milliseconds = 0;

/**
 * Switches the ADC sampling between X and Y axis of the joystick.
 * Does not sample the joystick - you should switchToXAxis() first, then sampleJoystick().
 */
void switchToXAxis(bool xAxis) {
    if (xAxis) {
        // Read x-axis
        AD2CHS0bits.CH0SA = 4; // set ADC to Sample AN4 pin
    } else {
        // Read y-axis
        AD2CHS0bits.CH0SA = 5; // set ADC to Sample AN5 pin
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

    // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
    SETBIT(TRISEbits.TRISE8);
    SETBIT(INTCON2bits.INT1EP);
    SETBIT(AD1PCFGHbits.PCFG20);

    /*

        // configuration of joystick registers according to Lab Manual 4.6.2, "Button #1 is located on PORTE PIN8..."
        IEC1bits.INT1IE = 0; // Disable Timer1 interrupt
        SETBIT(TRISEbits.TRISE8);
        SETBIT(INTCON2bits.INT1EP);

        SETBIT(AD1PCFGHbits.PCFG20);
        INTCON2bits.INT1EP = 0; // Trigger on falling edge
        IEC1bits.INT1IE = 1; // Enable Timer1 interrupt

     */

    // disable ADC
    CLEARBIT(AD2CON1bits.ADON);

    // initialize PIN
    SETBIT(TRISBbits.TRISB4); // Reads RB4 (x-axis)
    SETBIT(TRISBbits.TRISB5); // Reads RB5 (y-axis)

    CLEARBIT(AD2PCFGLbits.PCFG4); // AN4, analog for x-axis
    CLEARBIT(AD2PCFGLbits.PCFG5); // AN5, analog for y-axis

    // Configure AD1CON1
    CLEARBIT(AD2CON1bits.AD12B); // set 10b Operation Mode
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

    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0;
    T2CONbits.TCKPS = 0b10;
    CLEARBIT(IEC0bits.T2IE);
    CLEARBIT(IEC0bits.T2IE);
    PR2 = 4000;

    CLEARBIT(TRISDbits.TRISD6);
    OC7R = 3700;
    OC7RS = 3700;
    OC7CONbits.OCM = 0b110;
    //    SETBIT(T2CONbits.TON);

    CLEARBIT(TRISDbits.TRISD7);
    OC8R = 3700;
    OC8RS = 3700;
    OC8CONbits.OCM = 0b110;
    SETBIT(T2CONbits.TON);

    unsigned int holdBuffer = 0;
    unsigned int pressed = 0;
    uint16_t joystick_val = 0;
    uint16_t x_min = -1;
    uint16_t x_max = -1;
    uint16_t y_min = -1;
    uint16_t y_max = -1;
    bool xLock = false;
    bool yLock = false;
    bool calibrationMode = true; // track whether we're in calibration or operation mode

    lcd_clear();
    lcd_locate(0, 0);
    lcd_printf("Move X, Y to MAX");

    while (calibrationMode) {
        joystick_val = sampleJoystick();

        // show current joystick value
        switchToXAxis(true);
        lcd_locate(0, 1);
        lcd_printf("joystick val: %d", sampleJoystick());

        if (BTN1_PRESSED()) {
            if (holdBuffer++ > 20) {
                if (calibrationMode) {
                    if (x_max == -1 && y_max == -1) {
                        // max and min values are not set yet, let's fix that
                        switchToXAxis(true);
                        x_max = sampleJoystick();
                        switchToXAxis(false);
                        y_max = sampleJoystick();
                        lcd_locate(0, 2);
                        // print the max values
                        lcd_printf("x: %d, y: %d", x_max, y_max);
                        lcd_locate(0, 0);
                        lcd_printf("Move X, Y to MIN");
                    } else if (x_min == -1 && y_min == -1) {
                        // set both min values at the same time
                        switchToXAxis(true);
                        x_min = sampleJoystick();
                        switchToXAxis(false);
                        y_min = sampleJoystick();

                        lcd_locate(0, 3);
                        lcd_printf("x: %d, y: %d", x_min, y_min);

                        // switch to operation mode after calibration is complete
                        calibrationMode = false;
                    }
                }
                holdBuffer = 0;
                __delay_ms(500);
            }
        } else {
            holdBuffer = 0;
        }
        __delay_ms(50);
    };

    // Operation mode
    uint16_t xservo;
    uint16_t yservo;
    unsigned int lockBuffer = 0;
    bool buttonWasPressed = false;
    lcd_locate(0, 0);
    lcd_printf("Calib done...........");


    while (1) {
        if (BTN1_PRESSED()) {
            if (lockBuffer++ > 20 && !buttonWasPressed) {
                if (!xLock) {
                    xLock = true;
                    lcd_locate(0, 1);
                    lcd_printf("X Locked");
                } else if (!yLock) {
                    yLock = true;
                    lcd_locate(0, 2);
                    lcd_printf("Y Locked");
                }
                buttonWasPressed = true;
                lockBuffer = 0;
                __delay_ms(500);
            }
        } else {
            lockBuffer = 0;
            buttonWasPressed = false;
        }

        if (!xLock) {
            switchToXAxis(true);
            joystick_val = sampleJoystick();
            xservo = floor((joystick_val - x_min) * (double) 240 / (x_max - x_min) + 3580);
            OC7R = xservo;
            OC7RS = xservo;
            OC7CONbits.OCM = 0b110;
        }

        if (!yLock) {
            switchToXAxis(false);
            joystick_val = sampleJoystick();
            yservo = floor((y_max - joystick_val) * (double) 240 / (y_max - y_min) + 3580);
            OC8R = yservo;
            OC8RS = yservo;
            OC8CONbits.OCM = 0b110;
        }

        lcd_locate(0, 3);
        lcd_printf("X: %d Y: %d", xservo, yservo / 10);
        __delay_ms(50);
    }

    lcd_clear();
    lcd_locate(0, 0);
    lcd_printf("x-min: %X", x_min);
    lcd_locate(0, 1);
    lcd_printf("x-max: %X", x_max);
    lcd_locate(0, 2);
    lcd_printf("y-min: %X", y_min);
    lcd_locate(0, 3);
    lcd_printf("y-max: %X", y_max);

    while (1);

    return 0;
} 