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


#
// Primary (XT, HS, EC) Oscillator without PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystanl
_FOSC(OSCIOFNC_ON & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);


unsigned char buffer[] = "abcdefghijklmn\0";

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
    uart2_init(9600);

    SETLED(LED1_PORT);
    CLEARLED(LED2_PORT);
    CLEARLED(LED3_PORT);
    CLEARLED(LED4_PORT);


    //    uart2_send_8('q');


    lcd_locate(0, 0);
    lcd_printf("hello");
    lcd_locate(0, 1);

    SETLED(LED2_PORT);

    int print = 0;
    int n = 0;
    while (1) {



        for (n = 0; n < 4;)
            n += 1 + uart2_recv(buffer + n);




        uint16_t crc_local;
        for (crc_local = 0; n < 4 + buffer[3];) {
            if (0 == uart2_recv(buffer + n))
                crc_local = crc_update(crc_local, buffer[ n++ ]);
        }



        // end string 
        buffer[n] = 0;

        // concatenate hex values
        if (crc_local != ((uint16_t) buffer[1] * 256 + buffer[2])) {
            lcd_locate(0, 0);
            lcd_printf("our:%x t:%x", crc_local, 256 * buffer[1] + buffer[2]);
            lcd_locate(0, 1);
        }

        //            while( *buffer != 0 );
        //    CLEARLED( LED1_PORT);
        SETLED(LED3_PORT);
        //        while(1);

        // Task 1: Display Group Name
        // lcd_locate (column, row)
        /*        if (++print % 100 == 0) { */
        //        lcd_locate(0, 0);
        //       lcd_printf("test:  %x  %x  %x  %s", *(buffer + 1), *(buffer + 2), *(buffer + 3), buffer + 4);
        lcd_locate(0, 1);
        lcd_printf("length: %d %x", *(buffer + 3), crc_local);
        lcd_locate(0, 2);
        lcd_printf("val: %s ", buffer + 4);
        lcd_locate(0, 3);
        lcd_printf("%x  %x ", buffer[1], buffer[2]);


        // check for extra byte
        __delay_ms(50); // wait multiple 9600 byte periods
        //         n += 1 +  uart2_recv(buffer + n);

        if ((crc_local != 256 * (uint16_t) buffer[1] + buffer[2]) /* || 0x00 != buffer[0] */) {

            // RETRY
            uart2_send_8(0x00);

            SETLED(LED4_PORT);


        } else {


            // SUCCESS
            print = 0;

            uart2_send_8(0x01);
        }

    }

    //    uart2_send_8( data + 0x41 );  
    //    U1TXREG = data + 0x41; // Transmit one character

    //    CLEARLED(LED2_PORT);
    //    SETLED(LED4_PORT);







}

