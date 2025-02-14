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

#ifndef __LAB_3_TROLL
#define __LAB_3_TROLL

void send_troll(int ofd, char str[1], double pct);

#endif
