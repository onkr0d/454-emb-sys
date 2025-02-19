/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: lab3 UART2 init,TX,RX functions   */
/*                                                  */
/****************************************************/

#include "uart.h"
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "types.h"
#include "led.h"
#include <xc.h>   // Contains Nop() declaration
unsigned int i;

inline void uart2_init(uint16_t baud) {
    /* Implement me please. */
    // Configure Oscillator to operate the device at 40Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 8M*40(2*2)=80Mhz for 8M input clock
    RCONbits.SWDTEN = 0; // Disable Watch Dog Timer
    while (OSCCONbits.LOCK != 1) {
    }; // Wait for PLL to lock
    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Auto-Baud Disabled
    U2MODEbits.BRGH = 0; // Low Speed mode
//    U2BRG = BRGVAL; // BAUD Rate Setting for 9600
    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;
//    IEC0bits.U2TXIE = 1; // Enable UART Tx interrupt
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART Tx
    /* wait at least 104 usec (1/9600) before sending first char */
    for (i = 0; i < 4160; i++) {
        Nop();
    }
    U1TXREG = 'a'; // Transmit one character
    while (1) {
    }
}

void __attribute__((__interrupt__)) _U2TXInterrupt(void) {
//    IFS0bits.U2TXIF = 0; // clear TX interrupt flag
    U2TXREG = 'a'; // Transmit one character
}

void uart2_send_8(int8_t data) {
    /* Implement me please. */
}

int8_t uart2_recv(uint8_t* data) {
    /* Implement me please. */
}
