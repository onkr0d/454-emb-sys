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

    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Auto-Baud Disabled
    U2MODEbits.BRGH = 0; // Low Speed mode

    U2BRG = ((FCY) / (9600 * 16UL)) - 1; // BAUD Rate Setting for 9600

    U2STAbits.UTXISEL0 = 0; // Interrupt after one Tx character is transmitted
    U2STAbits.UTXISEL1 = 0;

    //IEC0bits.U2TXIE = 1; // Enable UART Tx interrupt
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART Tx

    /* wait at least 104 use (1/9600) before sending first char */
    for (i = 0; i < 4160; i++) {
        Nop();
    }

}

void uart2_send_8(int8_t data) {

    while (U2STAbits.UTXBF);
    U2TXREG = data & 0x00ff;
    while (!U2STAbits.TRMT);

}

int8_t uart2_recv(uint8_t* data) {

    if (U2STAbits.OERR)
        U2STAbits.OERR = 0;

    // data received
    if (U2STAbits.URXDA) {
        *data = U2RXREG & 0x00ff;
        return 0;
    }

    // still waiting
    return -1;

    //    while (U1STAbits.UTXBF);

    //  Return 0 for success, < 0 for failure
}
