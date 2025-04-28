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
    __delay_ms(7);

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
signed int sampleABS() {
    SETLED(LED4_PORT);

    int numSamples = 0;
    signed int samples[4] = {0, 0, 0, 0};

    while (numSamples < 3) {

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
//    qsort(samples, 3, sizeof (unsigned int), compareInts);
    CLEARLED(LED4_PORT);
    return samples[0];
}

struct pos {
    signed int x;
    signed int y;
};

struct state {
    double x;
    double  y;
    double  ex;
    double  ey;
    double i_ex;
    double i_ey;
};

struct cornerPos {
    signed int x;
    signed int y;
};

void t2cycle(uint16_t amount) { // wait 20 ms
    uint16_t tStart;

    while (amount--) {
        tStart = TMR2;
        while (tStart == TMR2);

        while (tStart != TMR2);


    }
}

double x_rw[4], y_rw[4], x_fl[4] , y_fl[4] = { 0.0 , 0.0 , 0.0 , 0.0 } ;

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


/* TIMER23 -- 50 HZ -- SERVO PWM */

    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0;
    T2CONbits.TCKPS = 0b10;     // divide by 64
    PR2 = 4000  - 1 ;
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE);

    SETBIT(T2CONbits.TON);


/* TIMER 3 -- 20 HZ -- PID TIMING */

    CLEARBIT(T3CONbits.TON);
    CLEARBIT(T3CONbits.TCS);
    CLEARBIT(T3CONbits.TGATE);
    TMR3 = 0;
    T3CONbits.TCKPS = 0b11;     // divide by 256
    PR3 = 50000  - 1 ;
    CLEARBIT(IFS0bits.T3IF);
    CLEARBIT(IEC0bits.T3IE);

    SETBIT(T3CONbits.TON);



    AD2CON3 = 0x0002; //manual sample, from example Example 16-1 in dsPIC33F.16_-_ADC


    //lcd for debug
    CLEARBIT(LED4_TRIS); // Set Pin to Output

    lcd_clear();

    int cornerCounter = 0;
    struct pos setPoint = {.x = 1594, .y = 1433};
//    struct pos setPoint = {.y = 1594, .x = 1433};
    //    struct pos setPoint = {.x = 1594, .y = 1600};
    struct pos currPos = {.x = 0, .y = 0};
    struct state prevState = 
    {
        .x = 0, 
        .y = 0, 
        .ex = 0, 
        .ey = 0,
        .i_ex = 0,
        .i_ey = 0
    };   
    
    
    struct cornerPos corner0 = {.x = 0, .y = 0};
    struct cornerPos corner1 = {.x = 0, .y = 0};
    struct cornerPos corner2 = {.x = 0, .y = 0};
    struct cornerPos corner3 = {.x = 0, .y = 0};

    SETLED(LED4_PORT);

            signed int limy = 0, out_x = 0 , out_y = 0 /* , ex = 0 , ey = 0  */ ;
    
            unsigned bwoff = 0 ;
//            double x_rw[4], y_rw[4], x_fl[4] , y_fl[4] ; 

/* BUTERWORTH CONSTANTS */
            

#define B0      0.005886216155
#define B1      0.017658648465
#define B2      B1
#define B3      B0

#define A1     -2.188288161659 
#define A2      1.674598618846
#define A3     -0.439220727946

#define AZ0     0

  
            /*
            
#define B0      0.5
#define B1      0.0
#define B2      0
#define B3      0

#define A1      - 
#define A2      0.0
#define A3      0.0            
            
#define AZ0     0.0            

            */
            
            
    double b[4][4] = { { B0  , B3  , B2  , B1 } ,       
                       { B1  , B0  , B3  , B2 } ,       
                       { B2  , B1  , B0  , B3 } ,       
                       { B3  , B2  , B1  , B0 } } ;

    double a[4][4] = { { AZ0 , A3  , A2  , A1  } ,                  
                       { A1  , AZ0 , A3  , A2  } ,      
                       { A2  , A1  , AZ0 , A3  } ,      
                       { A3  , A2  , A1  , AZ0 } } ;

            
    while (true) {

        bwoff = ( 1 + bwoff ) % 4 ;

        
        SETLED(LED1_PORT);
//        t2cycle(2);
                __delay_ms(26);
        CLEARLED(LED1_PORT);
        //        t2cycle(50); 

        //        __delay_ms(700);
        CLEARLED(LED2_PORT);
//        __delay_ms(500);

        // Sampling
        switchToXAxis(true);
        currPos.x = sampleABS();
        switchToXAxis(false);
        currPos.y = sampleABS();
        
        x_rw[bwoff] = currPos.x ;
        y_rw[bwoff] = currPos.y ;

        y_fl[bwoff] = 
                b[bwoff][0]*y_rw[0] + 
                b[bwoff][1]*y_rw[1] +     
                b[bwoff][2]*y_rw[2] + 
                b[bwoff][3]*y_rw[3] -    
                                                                      
                a[bwoff][0]*y_fl[0] - 
                a[bwoff][1]*y_fl[1] -     
                a[bwoff][2]*y_fl[2] - 
                a[bwoff][3]*y_fl[3]   ;
  
        
//        currPos.y = y_fl[bwoff] ;
        

        double kp = 0.16;
        double kd = 0.000;
        double ki = 0.02;

        /* -------------- PID Controller calculations --------------*/
        
        double dt = 0.05; // Unsure abt this value
        
        // Error value
        double ex = (currPos.x - setPoint.x) ;
        double ey = (currPos.y - setPoint.y) ;
        
        // Integral
        prevState.i_ex += ex * dt;
        prevState.i_ey += ey * dt;
        
        // Velocity value
        double dex = (double) (ex - prevState.ex) / dt;
        double dey = (double) (ey - prevState.ey) / dt;

        // PID calculation
        out_x = -kp * ex * prevState.i_ex - kd * dex;
        out_y = -kp * ey * prevState.i_ey - kd * dey;

        // Save old state
        prevState.x = currPos.x;
        prevState.y = currPos.y;
        prevState.ex = ex;
        prevState.ey = ey;

        
        
        



        
        
        int corner = cornerCounter % 4;

    
         CLEARBIT(TRISDbits.TRISD6);
//         OC7R = 1;
          limy =  3700  -  out_y ; 
          limy = limy < 3820 ? limy : 3820  ;
//          OC7RS = limy > 3580 ? limy : 3580 ;
   
//          OC7CONbits.OCM = 0b110;
                 
                 
                 
                //                SETBIT(T2CONbits.TON);

//                CLEARBIT(TRISDbits.TRISD7);
//                OC8R = 1;
//                OC8RS = 3700;
//                OC8CONbits.OCM = 0b110;
                //                SETBIT(T2CONbits.TON);
 

          if ( 0 == bwoff) {
        // Display
        lcd_clear();
        lcd_locate(0, 0);
//        lcd_printf("X=%04d, Y=%04d", limy, currPos.y);
                lcd_printf("X=%04d, Y=%f", currPos.y , y_fl[bwoff] );
        lcd_locate(0, 0);

          }
//        __delay_ms(1850); // not exactly 2 seconds because switching axis adds a 50ms delay, plus other stuff. double check this number before demo!



        

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
