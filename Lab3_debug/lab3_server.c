/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file for serial          */
/*                communication server              */
/*                                                  */
/****************************************************/

#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <termios.h>

#include <unistd.h>

#include <sys/types.h>

#include <fcntl.h>

#include "pc_crc16.h"

#include "lab3.h"

#include "lab3_troll.h"

#define GREETING_STR\
  "CS454/654 - Lab 3 Server\n"\
"Author: Renato Mancuso (BU)\n"\
"(Enter a message to send.  Type \"quit\" to exit)\n"

#define USAGE_STR\
  "\nUSAGE: %s [-v] [-t percentage] <dev>\n"\
"   -v \t\t Verbose output\n"\
"   -t \t\t Invoke troll with specified bit flipping percentage\n"\
"   <dev> \t Path to serial terminal device to use, e.g. /dev/ttyUSB0\n\n"

#define TROLL_PATH "./lab3_troll"

int main(int argc, char * argv[]) {
  double troll_pct = 0.3; // Perturbation % for the troll (if needed)
  int ifd, ofd, i, N, troll = 0; // Input and Output file descriptors (serial/troll)
  char str[MSG_BYTES_MSG], opt; // String input
  struct termios oldtio, tio; // Serial configuration parameters
  int VERBOSE = 0; // Verbose output - can be overriden with -v
  int dev_name_len;
  char * dev_name = NULL;

  /* Parse command line options */
  while ((opt = getopt(argc, argv, "-t:v")) != -1) {
    switch (opt) {
    case 1:
      dev_name_len = strlen(optarg);
      dev_name = (char * ) malloc(dev_name_len); // MEMORY LEEAAAAAK
      strncpy(dev_name, optarg, dev_name_len);
      break;
    case 't':
      troll = 1;
      troll_pct = atof(optarg);
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
    printf("zaza");
    fprintf(stderr, USAGE_STR, argv[0]);
    exit(EXIT_FAILURE);
  }

  // Open the serial port (/dev/ttyS1) read-write
  ifd = open(dev_name, O_RDWR | O_NOCTTY);
  if (ifd < 0) {
    perror(dev_name);
    exit(EXIT_FAILURE);
  }

  printf(GREETING_STR);

  // Start the troll if necessary
  if (troll) {
    // Open troll process (lab5_troll) for output only
    FILE * pfile; // Process FILE for troll (used locally only)
    char cmd[128]; // Shell command

    snprintf(cmd, 128, TROLL_PATH " -p%f %s %s", troll_pct,
      (VERBOSE) ? "-v" : "", dev_name);

    pfile = popen(cmd, "w");
    if (!pfile) {
      perror(TROLL_PATH);
      exit(-1);
    }
    ofd = fileno(pfile);
  } else ofd = ifd; // Use the serial port for both input and output

  /*
   * Set up the serial port
   * Control modes (c_cflag):
   *  - B2400:	9600 Baud // INCORRECT COMMENT!?
   *  - CS8:	8 data bits
   *  - CLOCAL:	Ignore modem control lines
   *  - CREAD:	Enable receiving
   *  1 stop bit, no parity, no flow control is default 
   * Output modes (c_oflag):
   *  defaults
   * Input modes (c_iflag):
   *  - IGNPAR:	Ignore parity errors
   * Control characters (c_cc):
   *  - VMIN:	Minimum recv length
   */
  tcgetattr(ifd, & oldtio);
  tio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  tio.c_iflag = IGNPAR;
  tio.c_oflag = 0;
  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tcflush(ifd, TCIFLUSH);
  tcsetattr(ifd, TCSANOW, & tio);

  while (1) {
    unsigned char ack = MSG_NACK;
    int crc = 0xff, attempts = 0;

    printf("> ");
/*
    // Read a line of input
    for (i = 0;(i < MSG_BYTES_MSG) && ((str[i] = fgetc(stdin)) != '\n'); i++) {}
    
    if (i >= MSG_BYTES_MSG - 1) {
	printf("Warning: %d characters exceeded\n", MSG_BYTES_MSG);
    }
    
    str[i] = 0; // terminate string, removing newline
    N = i;

    // og code tweaking ngl
    //     if (strcmp(str, "quit") == 0) break;
    if (!strcmp(str, "quit")) break;

    // Compute crc (only lowest 16 bits are returned)
    crc = pc_crc16(str, N);
    printf("crc: %x\n", crc);
/*
    while (!ack) {
      printf("Sending (attempt %d)...\n", ++attempts);

      // Send message
      dprintf(ofd, "%c", 0x0); // Start byte

      for (i = MSG_BYTES_CRC - 1; i >= 0; i--) { // CRC
        dprintf(ofd, "%c", crc >> (8 * i));
      }
      for (i = MSG_BYTES_MSG_LEN - 1; i >= 0; i--) { // Message length
        dprintf(ofd, "%c", N >> (8 * i));
      }

      dprintf(ofd, "%s", str); // Message
*/
      printf("Message sent, waiting for ack... ");
while(1) {
      // Wait for MSG_ACK or MSG_NAK

      printf("%x\n", 140 );
      read(ifd, &ack, 1);
           printf("%x\n", ack );
  }  
    printf("\n");
  }

  // Reset the serial port parameters
  tcflush(ifd, TCIFLUSH);
  tcsetattr(ifd, TCSANOW, & oldtio);

  // Close the serial port
  close(ifd);

  return EXIT_SUCCESS;
}
