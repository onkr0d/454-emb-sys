/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file for digital and     */
/*                analog square wave generation     */
/*                via the LabJack U3-LV 
USB DAQ     */
/*                                                  */
/****************************************************/

#include "u3.h"
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

u3CalibrationInfo HcaliInfo ;


/* This function should initialize the DAQ and return a device
 * handle. The function takes as a parameter the calibratrion info to
 * be filled up with what obtained from the device. */
HANDLE init_DAQ(u3CalibrationInfo * caliInfo)
{
	HANDLE hDevice;
	int localID;
	
	/* Invoke openUSBConnection function here */
	 hDevice = openUSBConnection( -1 );

	/* Invoke getCalibrationInfo function here */
	 getCalibrationInfo( hDevice, caliInfo );
	return hDevice;
}

struct timespec  res;
HANDLE newhandle;

int main(int argc, char **argv)
{
	clock_getres( CLOCK_REALTIME , &res ); 
	printf( "  %ld  foo \n", res.tv_nsec ) ;
	/* Invoke init_DAQ and handle errors if needed */

	newhandle = init_DAQ( &HcaliInfo);
printf( " %d \n" , (int) newhandle ) ;
	int n;
	while(1)
	{
	for (n=0;;n++<1000);
		eDO( newhandle, 1 , 2 , 0 );
	for (n=0;;n++<1000);
		eDO( newhandle, 1 , 2 , 0 );
}
	/* Provide prompt to the user for a voltage range between 0
	 * and 5 V. Require a new set of inputs if an invalid range
	 * has been entered. */
	
	
	
	/* Compute the maximum resolutiuon of the CLOCK_REALTIME
	 * system clock and output the theoretical maximum frequency
	 * for a square wave */
	
	/* Provide prompt to the user to input a desired square wave
	 * frequency in Hz. */

	/* Program a timer to deliver a SIGRTMIN signal, and the
	 * corresponding signal handler to output a square wave on
	 * BOTH digital output pin FIO2 and analog pin DAC0. */

	/* The square wave generated on the DAC0 analog pin should
	 * have the voltage range specified by the user in the step
	 * above. */

	/* Display a prompt to the user such that if the "exit"
	 * command is typed, the USB DAQ is released and the program
	 * is terminated. */
	
	return EXIT_SUCCESS;
}

