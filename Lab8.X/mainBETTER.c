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
    __delay_ms(11);

}

int compareInts(const void *a, const void *b) {
    unsigned int int_a = *(const signed int *) a;
    unsigned int int_b = *(const signed int *) b;
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
    signed int samples[3] = {0, 0, 0, 0, 0};

    while (numSamples < 3 ) {

        // Joystick value sampling code
        SETBIT(AD1CON1bits.SAMP); // start to sample
        // wait for sampling or no?
        //        CLEARBIT(AD1CON1bits.SAMP); // start to sample

        __delay_ms(1);
//        CLEARBIT(AD1CON1bits.SAMP);

        while (!AD1CON1bits.DONE); // wait for conversion to finish
        // reduce risk of ADC sampling
        signed int val = ADC1BUF0;
        CLEARBIT(AD1CON1bits.DONE); // MUST HAVE! clear conversion done bit
        samples[numSamples] = val;
        numSamples++;
    }

    // median of samples
//    qsort(samples, 3, sizeof (signed int), compareInts);
    CLEARLED(LED4_PORT);
    return samples[1];
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


/* TIMER 3 -- PID TIMING  & WATCHDOG  */

    CLEARBIT(T3CONbits.TON);
    CLEARBIT(T3CONbits.TCS);
    CLEARBIT(T3CONbits.TGATE);
    TMR3 = 0;
    T3CONbits.TCKPS = 0b11;     // divide by 256
    PR3 = 0xffff;
    CLEARBIT(IFS0bits.T3IF);
    CLEARBIT(IEC0bits.T3IE);

    SETBIT(T3CONbits.TON);



    AD2CON3 = 0x0002; //manual sample, from example Example 16-1 in dsPIC33F.16_-_ADC


    //lcd for debug
    CLEARBIT(LED4_TRIS); // Set Pin to Output

    lcd_clear();

//    struct pos setPoint = {.x = 1600, .y = 1470};
    struct pos setPoint = {.x = 1600, .y = 1520};
    struct pos currPos = {.x = 0, .y = 0};
    struct state prevState = 
    {
        .x = 0, 
        .y = 0, 
        .ex = 0, 
        .ey = 0,
        .i_ex = 0,
        .i_ey = 0
    } ;


    SETLED(LED4_PORT);

    signed int x_clipped = 0 , y_clipped = 0,
    out_x = 0 , out_y = 0 /* , ex = 0 , ey = 0  */ ;

    unsigned bwoff = 0 ;

/* BUTERWORTH CONSTANTS */
            
//0.289003621918   0.867010865753  0.867010865753  0.289003621918  
//1.000000000000   0.751546918407  0.483668797629  0.076813259304  
            
//  CUTOFF 6.3 HZ            
  /*         
#define B0      0.289003621918 
#define B1      0.867010865753
#define B2      B1
#define B3      B0

#define A3     0.751546918407
#define A2     0.483668797629 
#define A1     0.076813259304

#define AZ0     0
*/
          



// cutoff 2.1
  /*          
#define B0      0.020483504914  
#define B1      0.061450514743 
#define B2      B1
#define B3      B0

#define A1     -1.699513191903 
#define A2      1.123447819405
#define A3     -0.260066588186          
            
#define AZ0     0.0    
*/


// cutoff 3.4
            
#define B0     0.066957965254
#define B1     0.200873895763 
#define B2      B1
#define B3      B0

#define A1     -0.926717845969
#define A2       0.562689163506 
#define A3     -0.100307595502          
            
#define AZ0     0.0    


/*
            
// cutoff 1.3
            
#define B0      0.005886216155
#define B1      0.017658648465 
#define B2      B1
#define B3      B0ate.i_ex += ex * dt;
        prevState.i_ey += ey * dt;

#define A1      -2.188288161659 
#define A2      1.674598618846 
#define A3      -0.439220727946         
            ate.i_ex += ex * dt;
        prevState.i_ey += ey * dt;
#define AZ0     0.0            
*/
            
// cutoff 9.9
            /*ate.i_ex += ex * dt;
        prevState.i_ey += ey * dt;
#define B0      0.969071174032
#define B1      2.90ate.i_ex += ex * dt;
        prevState.i_ey += ey * dt;7213ate.i_ex += ex * dt;
        prevState.i_ey += ey * dt;522095
#define B2      B1            
#define B3      B0

#define A1      2.937170728450 
#define A2      2.876299723479 
#define A3      0.939098940325         
            
#define AZ0     0.0                 
            */

// cutoff 0.3            
       /*     
#define B0      0.000095442508 
#define B1      0.000286327525
#define B2      B1            
#define B3      B0

#define A1      -2.811573677325
#define A2      2.640483492778 
#define A3      -0.828146275386          
            
#define AZ0     0.0    
            
                      
         */   
            
                        
    double x_rw[4] = { 0.0 } ,
        y_rw[4] = { 0.0 } ,
        x_fl[4] = { 0.0 } , 
        y_fl[4] = { 0.0 } ;
    
    double b[4] =  { B0  , B1  , B2  , B3 } ;       
    double a[4] =  { AZ0 , A1  , A2  , A3  } ;  
        

    switchToXAxis(true);
    currPos.x = sampleABS();
    switchToXAxis(false);
    currPos.y = sampleABS();
/*
    int loop;
    for (loop = 0; loop < 4; loop++) {
        x_rw[loop] = currPos.x;
        y_rw[loop] = currPos.y;
        x_fl[loop] = currPos.x;
        y_fl[loop] = currPos.y;
    }
*/
    
    OC7CONbits.OCM = 0b110;
    OC8CONbits.OCM = 0b110;
    
    uint16_t lastT3 = TMR3 , nowT3 , whatT3;
    
    unsigned togLED1 = 0 ;
    
    
    SETBIT(LED3_PORT);    
    


    while (true) {
        togLED1 = ( 1 + togLED1 ) % 20 ;
        if ( 10 > togLED1 ) 
            SETBIT(   LED1_PORT ) ;
        else 
            CLEARBIT( LED1_PORT) ;
        
         for( nowT3 = TMR3 ; (whatT3 = nowT3 - lastT3) < 2500 ; nowT3 = TMR3 ) ;
         if (2500 < nowT3 - lastT3 ) 
             SETBIT(LED5_PORT);
         

         lastT3 = nowT3 ;
        
        CLEARLED(LED2_PORT);

        // Sampling
        switchToXAxis(true);
        currPos.x = sampleABS();
        switchToXAxis(false);
        currPos.y = sampleABS();
        

        /* ------ Butterworth below here ------  */
  //      bwoff = ( 1 + bwoff ) % 4 ;
        
        
        x_rw[3]=x_rw[2];
        x_rw[2]=x_rw[1];
        x_rw[1]=x_rw[0];
                    
        y_rw[3]=x_rw[2];
        y_rw[2]=y_rw[1];
        y_rw[1]=y_rw[0];
                    
        x_fl[3]=x_fl[2];
        x_fl[2]=x_fl[1];
        x_fl[1]=x_fl[0];
                    
        y_fl[3]=y_fl[2];
        y_fl[2]=y_fl[1];
        y_fl[1]=y_fl[0];  
        
        
        
        
        x_rw[0] = currPos.x;
        y_rw[0] = currPos.y;
        
        int idx0 = (bwoff + 0) % 4;
        int idx1 = (bwoff - 1 + 4) % 4;    struct pos setPoint = {.x = 1600, .y = 1470};
        int idx2 = (bwoff - 2 + 4) % 4;
        int idx3 = (bwoff - 3 + 4) % 4;
       
/*
                x_fl[bwoff] =
            b[0] * x_rw[idx0] +
            b[1] * x_rw[idx1] +
            b[2] * x_rw[idx2] +
            b[3] * x_rw[idx3]
          - a[1] * x_fl[idx1]
          - a[2] * x_fl[idx2]
          - a[3] * x_fl[idx3];
  */      
        
        x_fl[0] =
            b[0] * x_rw[0] +
            b[1] * x_rw[1] +
            b[2] * x_rw[2] +
            b[3] * x_rw[3]
          - a[1] * x_fl[1]
          - a[2] * x_fl[2]
          - a[3] * x_fl[3];
        
        y_fl[0] =
            b[0] * y_rw[0] +
            b[1] * y_rw[1] +
            b[2] * y_rw[2] +
            b[3] * y_rw[3]
          - a[1] * y_fl[1]
          - a[2] * y_fl[2]
          - a[3] * y_fl[3];
        
        currPos.x = x_fl[bwoff];
        currPos.y = y_fl[bwoff];

        /* -------------- PID Controller calculations --------------*/
        double x_kp = 0.05;
        double x_kd = 0.1;
        double x_ki = 0.0;

        double y_kp = 0.03;
        double y_kd = 0.05;
        double y_ki = 0.0;

        
        
        double dt = 1 / 20.0 ;
        
        // Error value
        double ex = (currPos.x - setPoint.x) ;
        double ey = (currPos.y - setPoint.y) ;
        
        
        // Integral
//        prevState.i_ex += ex * dt;
//        prevState.i_ey += ey * dt;


        prevState.i_ex += 0.05 * ( ex - prevState.i_ex ) ;
        prevState.i_ey += 0.05 * ( ey - prevState.i_ey ) ;
        
        
        // Velocity value
        double dex = (double) (ex - prevState.ex) / dt;
        double dey = (double) (ey - prevState.ey) / dt;

        // PID calculation
        out_x = -x_kp * ex - x_ki * prevState.i_ex - x_kd * dex;
        out_y = -y_kp * ey - y_ki * prevState.i_ey - y_kd * dey;
        
        // Clamp PID outputs
        if (out_x > 200) out_x = 200;
        if (out_x < -150) out_x = -200;

        if (out_y > 200) out_y = 200;
        if (out_y < -200) out_y = -200;

        // Save old state
        prevState.x = currPos.x;
        prevState.y = currPos.y;
        prevState.ex = ex;
        prevState.ey = ey;
      
    
        CLEARBIT(TRISDbits.TRISD6);
        y_clipped =  3670 -  1 * out_y ; 
//        y_clipped = ( ( 3820 > y_clipped ) ? y_clipped : 3820 ) ;
//        y_clipped =  ( (3580 < y_clipped ) ? y_clipped : 3580 ) ;

        y_clipped = ( ( 3695 > y_clipped ) ? y_clipped : 3695 ) ;
        y_clipped =  ( (3645 < y_clipped ) ? y_clipped : 3645 ) ;


        OC7RS =  y_clipped ;


        CLEARBIT(TRISDbits.TRISD7);
        x_clipped =  3665  -  1 * out_x ; 
//        x_clipped = ( ( 3820 > x_clipped ) ? x_clipped : 3820 ) ;
//        x_clipped =  ( (3580 < x_clipped ) ? x_clipped : 3580 ) ;

        x_clipped = ( ( 3690 > x_clipped ) ? x_clipped : 3690 ) ;
        x_clipped =  ( (3640 < x_clipped ) ? x_clipped : 3640 ) ;


        OC8RS =  x_clipped ;
     
          

        if ( 19 == togLED1 )  {
          // Display
          lcd_clear();
          lcd_locate(0, 0);
      //          lcd_printf("%.4e %.4e", x_fl[0] , x_fl[1] ) ;
          lcd_printf("X=%04d Y=%04d", whatT3 , currPos.y);
          lcd_locate(0, 0);
  


/*
          if ( 19 == togLED1 )  {
        // Display
        lcd_clear();
        lcd_locate(0, 0);
//                       lcd_printf("X=%04d, Y=%04d", limy, currPos.y);
        lcd_printf("%.4e %.4e", x_fl[0] , x_fl[1] ) ;
                lcd_locate(0, 1);
               lcd_printf("%.4e %.4e", x_fl[2] , x_fl[3] ) ;
        lcd_locate(0, 2);
       lcd_printf("%.4e %.4e", x_rw[0] , x_rw[1] ) ;
               lcd_locate(0, 3);
              lcd_printf("%.4e %.4e", x_rw[2] , x_rw[3] ) ;
        
              while(1);
          
  */        
          

        
        }
    }
    return 0;
}
