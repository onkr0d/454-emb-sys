/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: lab3 timer initialization func.   */
/*                                                  */
/****************************************************/


#include "timer.h"


//period in ms

void set_timer1(uint16_t period) {

    // timer 1 gabagoo,
//    T1CONbits.TON = 0; // Disable Timer
//    // Use internal clock for predictable timing
//    T1CONbits.TCS = 0; // Select internal instruction cycle clock
//    T1CONbits.TGATE = 0; // Disable Gated Timer mode
//    T1CONbits.TSYNC = 0; //Disable Synchronization
//    T1CONbits.TCKPS = 0b10; //Select 1:64 Prescaler
//    TMR1 = 0x00; //Clear timer register
//    PR1 = period; //Load the period value, should calculate for 1second
//    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
//    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
//    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
//    T1CONbits.TON = 1; // Start Timer


    // timer 1 gabagoo from lab 2
    // Timer 1
    //enable LPOSCEN
    __builtin_write_OSCCONL(OSCCONL | 2);
    T1CONbits.TON = 0; //Disable Timer
    T1CONbits.TCS = 1; //Select external clock
    T1CONbits.TSYNC = 0; //Disable Synchronization
    T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
    TMR1 = 0x00; //Clear timer register
    PR1 = 32767; //Load the period value
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    
    
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    T1CONbits.TON = 1; // Start Timer


    //
    //    T2CONbits.TON = 0;     // Disable Timer
    //    TMR2 = 0;              // Clear timer register
    //    
    //    T2CONbits.TCS = 0;     // Select internal instruction cycle clock
    //    T2CONbits.TGATE = 0;   // Disable Gated Timer mode
    //    T2CONbits.TCKPS = 0b11; // 1:256 prescaler
    //    
    //    PR2 = period;          // Set the period
    //    
    //    // Make sure interrupt flag is cleared before enabling
    //    IFS0bits.T2IF = 0;     // Clear Timer2 Interrupt Flag
    //    IPC1bits.T2IP = 0x01;  // Set Timer2 Interrupt Priority Level
    //    IEC0bits.T2IE = 1;     // Enable Timer2 interrupt
    //    
    //    
    //    T2CONbits.TON = 1;     // Start Timer
}

