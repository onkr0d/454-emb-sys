/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Tiger, Ivan, Tom                               */
/*                                                  */
/*   Description: simple serial application          */
/*                for Amazing Ball platform         */
/*                                                  */
/****************************************************/



#include <p33Fxxxx.h>
// do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

// #include <uart.h>


#include "lcd.h"
#include "types.h"
#include "led.h"
#include <xc.h>   // Contains Nop() declaration

#include "uart.h"
#include "crc16.h"
#include "lab3.h"
//  #include "timer.h"




#
// Primary (XT, HS, EC) Oscillator without PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystanl
_FOSC(OSCIOFNC_ON & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

int main(void) {
    /* Q: What is my purpose? */
    /* A: You pass butter. */
    /* Q: Oh. My. God. */

    /* LCD Initialization Sequence */
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();


    CLEARBIT(LED1_TRIS); // Set Pin to Output
    CLEARBIT(LED2_TRIS); // Set Pin to Output
    CLEARBIT(LED3_TRIS); // Set Pin to Output
    CLEARBIT(LED4_TRIS); // Set Pin to Output


    // serial test -- see uart.c
    SETLED(LED1_PORT);
    CLEARLED(LED2_PORT);
    uart2_init(9600);

    CLEARLED(LED1_PORT);
    SETLED(LED2_PORT);


}

