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
#include "timer.h"


//#
// Primary (XT, HS, EC) Oscillator without PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystanl
_FOSC(OSCIOFNC_ON & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);


unsigned char buffer[500];
unsigned int failures = 0;
unsigned int ACK = 0x1;
unsigned int NACK = 0x0;

unsigned int fail = 0;

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
    CLEARBIT(LED5_TRIS); // Set Pin to Output

    // serial test -- see uart.c      
    uart2_init(9600);


    CLEARLED(LED1_PORT);
    CLEARLED(LED2_PORT);
    CLEARLED(LED3_PORT);
    CLEARLED(LED4_PORT);
    CLEARLED(LED5_PORT);

    lcd_locate(0, 0);
    lcd_printf("hello");
    lcd_locate(0, 1);

    //    SETLED(LED1_PORT);

    int n = 0;
    while (1) {
        for (n = 0; n < 4;) {
            n += 1 + uart2_recv(buffer + n);
        }

 //       CLEARLED(LED5_PORT);

        SETLED(LED1_PORT);

        //        lcd_locate(0, 5);
        //        lcd_printf("                 ");
        //        lcd_locate(0, 5);

        set_timer1(0); // ignored = 1000ms*10^-3 * 12.8Mhz*10^6 * 1/256prescaler

        //        set_timer1(50000);
        lcd_locate(0, 7);
        lcd_printf("PR1:%u TMR1:%u", PR1, TMR1);


        uint16_t crc_local;
        for (crc_local = 0; !fail && n < 4 + buffer[3];) {
            if (0 == uart2_recv(buffer + n)) {
                crc_local = crc_update(crc_local, buffer[ n++ ]);
            }
        }

        SETLED(LED2_PORT);

        // end string 
        buffer[n] = 0;

        lcd_locate(0, 1);
        lcd_printf("length: %d  %x", *(buffer + 3), crc_local);
        lcd_locate(0, 2);
        lcd_printf("msg: %s ", buffer + 4);
        lcd_locate(0, 3);
        lcd_printf("fails: %d", failures);


        // check for extra byte
        //        __delay_ms(50); // wait multiple 9600 byte periods
        //         n += 1 +  uart2_recv(buffer + n);



        if (*(buffer) != 0x0) {
                SETLED(LED5_PORT);
            // start byte is incorrect
            lcd_locate(0, 4);
            lcd_printf("IFB");
            failures++;
            uart2_send_8(NACK);
            disableTimer();
            continue;
        }

        if ((crc_local != 256 * (uint16_t) buffer[1] + buffer[2]) /* || *(buffer) != 0x0 */) {
            // RETRY
            failures++;
            lcd_locate(0, 4);
            lcd_printf("CRC");
            lcd_locate(0, 4);

            uart2_send_8(NACK);
            SETLED(LED3_PORT);
        } else {
            // SUCCESS
            uart2_send_8(ACK);
            SETLED(LED4_PORT);
            lcd_locate(0, 4);
            lcd_printf("OK ");
            lcd_locate(0, 4);
            fail = 0;
            failures = 0;
        }

        // disable timer again
        disableTimer();
    }
}

int flip1 = 0;

void __attribute__((__interrupt__)) _T1Interrupt(void) {
    fail = 1;
    //    lcd_locate(0, 5);
    //    lcd_printf("INTR");
    //    lcd_locate(0, 5);
    flip1 = 1 - flip1;
    //    if (flip1) {
//    SETLED(LED5_PORT);
    //        Nop();
    //    } else {
    //        CLEARLED(LED4_PORT);
    //        Nop();
    //    }

    //    if (++fail > 3) {
    //        CLEARBIT(IFS0bits.T1IF);
    //        TMR1 = 00;
    //
    //        fail = 0;
    //        IFS0bits.T1IF = 1; // Set Timer1 Interrupt Flag
    //        IEC0bits.T1IE = 0; // Disable Timer1 interrupt
    //        T1CONbits.TON = 0; // Stop Timer
    //        return;
    //    }
    CLEARBIT(IFS0bits.T1IF);
//    return;
    lcd_locate(0, 5);
    lcd_printf("INTR");
    lcd_locate(0, 5);


    failures++;
    uart2_send_8(NACK);
    disableTimer();
    
    CLEARBIT(IFS0bits.T1IF);
}

void disableTimer() {
    TMR1 = 00;
    //    T2CONbits.TON = 0; //disable timer
    //    SETBIT(IFS0bits.T2IF); // set Timer1 Interrupt Flag - don't interrupt, we're done here

    T1CONbits.TON = 0; // Stop Timer    
    IEC0bits.T1IE = 0; // Disable Timer1 interrupt
    IFS0bits.T1IF = 0; // Set Timer1 Interrupt Flag
}