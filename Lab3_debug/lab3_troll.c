//
// CS 454/654 - Lab3 Troll
// PC/Linux
//
// Description:
//   The troll acts as a middleman that intercepts serial 
//   transmission from the lab3 server to the lab3 client.
//   The troll can alter arbitrary bits in the transmission
//   with a probability specified in a command line parameter.
//
// Change Log:
//   07/19/04  Created.  -Greg Dhuse
//   02/11/19  Modified. -Renato Mancuso (BU)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "lab3.h"

#define GREETING_STR						\
	"\n\nCS454/654 - Lab 3 Troll\n"				\
	"Author: Renato Mancuso (BU)\n"				\
	"Running with %.3f disturbance\n"

#define USAGE_STR							\
	"\nUSAGE: %s [-v] [-p percentage] <dev>\n"			\
	"   -v \t\t Verbose output\n"					\
	"   -t \t\t Bit flipping percentage\n"				\
	"   <dev> \t Path to serial terminal device to use, e.g. /dev/ttyUSB0\n\n"

int main(int argc, char* argv[])
{
	char opt;			
	double pct=0;			// Byte flip percentage
	int sfd,N;			// Serial file descriptor
	int VERBOSE = 0;		// Verbose output - can be enabled with the -v flag
	unsigned int mask;		// Bit flip mask
	unsigned char str[1];		// String input
	int dev_name_len;
	char * dev_name = NULL;

	// Read byte flip percentage from command line
	while ((opt = getopt(argc, argv, "-p:v")) != -1) {
	    switch (opt) {
	    case 1:
		dev_name_len = strlen(optarg);
		dev_name = (char *)malloc(dev_name_len);
		strncpy(dev_name, optarg, dev_name_len);
		break;
	    case 'p':
		pct = atof(optarg);
		break;
	    case 'v':
		VERBOSE = 1;
		break;
	    default:
		break;
	    }
	}
	

	/* Check if a device name has been passed */
	if (!dev_name) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// Open the serial port (e.g. /dev/ttyS1) read-write
	sfd = open(dev_name, O_RDWR | O_NOCTTY);
	if (sfd < 0) {
		perror(dev_name);
		exit(EXIT_FAILURE);
	}

	printf(GREETING_STR, pct);
	
	/*
	 * Note: Setting up the serial port doesn't need to be done here,
	 * 	 only because lab3_server already does it for us
	 */


	// Continuously read data from stdin, perturb it, and write it to the serial port
	while((N = fread(str, 1, 1, stdin)))
	{
		if ((rand()/(double)RAND_MAX) <= pct) {
			mask = (unsigned int)((rand()/(double)RAND_MAX) * 0xff);
			dprintf(sfd, "%c", str[0]^mask);
			if (VERBOSE) printf("Troll: Sending BAD: %d (0x%x)\n", str[0]^mask, str[0]^mask);
		}
		else	{
			dprintf(sfd, "%c", str[0]);
			if (VERBOSE) printf("Troll: Sending GOOD: %d (0x%x)\n", str[0], str[0]);
		}
	}

	// Reset the serial port parameters
	tcflush(sfd, TCIFLUSH);
	//tcsetattr(sfd, TCSANOW, &oldtio);

	// Close the serial port
	close(sfd);
	
	return 0;
}

