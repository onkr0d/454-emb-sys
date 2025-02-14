//
// CS 454/654 - Serial I/O debugger
// PC/Linux (provided)
//
// Sends raw data through the serial port.  When an integer
// is entered, it is sent as a raw integer.  Otherwise,
// a character string is sent.
//
// Change Log:
//   07/14/04  Created.  -Tim Eriksson & Greg Dhuse
//   02/11/19  Modified. -Renato Mancuso (BU)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUF_LEN 256

int main(int argc, char* argv[])
{
	int sfd;			// Serial file descriptor
	unsigned int num;		// Integer input 
	char str[BUF_LEN], *sptr;	// String input
	struct termios oldtio, tio;	// Serial configuration parameters

	if (argc < 2) {
	    fprintf(stderr, "You must specify a serial device to open\n");
	    exit(EXIT_FAILURE);
	}
	
	// Open the serial port (/dev/ttyS1) for writing only
	sfd = open(argv[1], O_WRONLY | O_NOCTTY);
	if (sfd < 0) { perror(argv[1]); exit(-1); }

	/*
 	 * Set up the serial port
	 * Control modes (c_cflag):
	 *  - B9600:	9600 Baud
	 *  - CS8:	8 data bits
	 *  - CLOCAL:	Ignore modem control lines
	 *  1 stop bit, no parity, no flow control is default 
	 *******************************************************
	 * The CREAD flag is also needed if reading is desired *
	 *******************************************************
	 * Output modes (c_oflag):
	 *  defaults
	 */
	tcgetattr(sfd, &oldtio);
	tio.c_cflag 	= B9600 | CS8 | CLOCAL;
	tio.c_iflag 	= 0;
	tio.c_oflag 	= 0;
	tio.c_lflag 	= 0;
	tcflush(sfd, TCIFLUSH);
	tcsetattr(sfd, TCSANOW, &tio);


	printf("CS 454/654 - Lab 3 Serial debugging tool\n(type \"quit\" to exit)\n");
	while(1)
	{
		sptr = str;
		printf("> ");

		// Read a line of input
		while ((sptr < str + BUF_LEN - 1) && 
		       ((*sptr++ = fgetc(stdin)) != '\n'));
		if (sptr >= str + BUF_LEN - 1) 
			printf("Warning: %d characters exceeded\n", BUF_LEN);
		*(--sptr) = 0; 	// terminate string, removing newline

		if (strcmp(str, "quit") == 0) break;

		// If an integer is entered, send its char value
		// If a string is entered, send it as a string of characters
		if (sscanf(str, "%d", &num) == 1)
			dprintf(sfd, "%c", num);
		else dprintf(sfd, "%s", str);
			
	}

	// Reset the serial port parameters
	tcflush(sfd, TCIFLUSH);
	tcsetattr(sfd, TCSANOW, &oldtio);

	// Close the serial port
	close(sfd);
	
	return 0;
}
