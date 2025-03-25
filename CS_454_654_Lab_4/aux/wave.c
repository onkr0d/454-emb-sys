/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                analog square wave generation     */
/*                via the LabJack U3-LV
USB DAQ     */
/*                                                  */
/****************************************************/

#include "u3.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

u3CalibrationInfo HcaliInfo;

/* This function should initialize the DAQ and return a device
 * handle. The function takes as a parameter the calibratrion info to
 * be filled up with what obtained from the device. */
HANDLE init_DAQ(u3CalibrationInfo *caliInfo) {
  HANDLE hDevice;
  int localID;

  /* Invoke openUSBConnection function here */
  hDevice = openUSBConnection(-1);

  /* Invoke getCalibrationInfo function here */
  // printf("   %ld  \n",
  getCalibrationInfo(hDevice, caliInfo)
  // )
  ;
  return hDevice;
}

struct timespec res;
HANDLE newhandle;

double v_low;
double v_high;

int state = 0;

void handler_timer1(int signum, siginfo_t *info, void *context) {
  eDO(newhandle, 1, 2, state = 1 - state);
  //  printf("  help!  \n");
}

void handler_timer2(int signum, siginfo_t *info, void *context) {
  eDAC(newhandle, &HcaliInfo, 1, (long)0, (double)(state ? v_high : v_low),
       (long)0, (long)0, (long)0);
  //    printf("  help2!  \n");
}

int main(int argc, char **argv) {
  /* Invoke init_DAQ and handle errors if needed */

  newhandle = init_DAQ(&HcaliInfo);
//  printf(" %d \n", (int)newhandle);
  int n;
  /* Provide prompt to the user for a voltage range between 0
   * and 5 V. Require a new set of inputs if an invalid range
   * has beenls -al entered. */

  do {
    printf("Enter desired voltage range between 0 and 5 V: ");
    scanf("%lf %lf", &v_low, &v_high);
  } while (v_low < 0 || v_high > 5 || v_low >= v_high);

  /* Compute the maximum resolutiuon of the CLOCK_REALTIME
   * system clock and output the theoretical maximum frequency
   * for a square wave */
   
   struct timespec res_period;
   clock_getres( CLOCK_REALTIME	, &res_period ) ; 
   printf (  "   %f  \n"  , 1.0e9 / (2.0 * res_period.tv_nsec ) ) ;
   


  /* Provide prompt to the user to input a desired square wave
   * frequency in Hz. */
  double freq;
  do {
    printf("Enter desired frequency in Hz: ");
    scanf("%lf", &freq);
  } while (freq <= 0 || freq > 500); // Arbitary range for now

  /* Program a timer to deliver a SIGRTMIN signal, and the
   * corresponding signal handler to output a square wave on
   * BOTH digital output pin FIO2 and analog pin DAC0. */
  /* The square wave generated on the DAC0 analog pin should
   * have the voltage range specified by the user in the step
   * above. */
  struct sigaction sa = {0};
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler_timer1;
  sigaction(SIGRTMIN, &sa, NULL);

  struct sigevent sev;
  memset(&sev, 0, sizeof(sev));
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGRTMIN;

  timer_t timer1;

  if (timer_create(CLOCK_REALTIME, &sev, &timer1) == -1) {
    perror("timer_create");
    exit(EXIT_FAILURE);
  }

  struct itimerspec timer1_time;
  timer1_time.it_value.tv_sec = 0;
  timer1_time.it_value.tv_nsec = (long)(0.25 * 1e9 / freq);
  timer1_time.it_interval.tv_sec = 0;
  timer1_time.it_interval.tv_nsec = (long)(0.5 * 1e9 / freq);

 

  struct sigaction sa2 = {0};
  sa2.sa_flags = SA_SIGINFO;
  sa2.sa_sigaction = handler_timer2;
  sigaction(SIGRTMAX, &sa2, NULL);

  struct sigevent sev2;
  memset(&sev2, 0, sizeof(sev2));
  sev2.sigev_notify = SIGEV_SIGNAL;
  sev2.sigev_signo = SIGRTMAX;

  timer_t timer2;
  if (timer_create(CLOCK_REALTIME, &sev2, &timer2) == -1) {
    perror("timer_create");
    exit(EXIT_FAILURE);
  }

  struct itimerspec timer2_time;
  timer2_time.it_value.tv_sec = 0;
  timer2_time.it_value.tv_nsec = ( long) (0.5 * 1e9 / freq) ;
  timer2_time.it_interval.tv_sec = 0 ;
  timer2_time.it_interval.tv_nsec = (long)(0.5 * 1e9 / freq) ;

 
  if (timer_settime(timer1, 0, &timer1_time, NULL) == -1) {
    perror("timer_settime");
    exit(EXIT_FAILURE);
  }
 
 
  if (timer_settime(timer2, 0, &timer2_time, NULL) == -1) {
    perror("timer_settime");
    exit(EXIT_FAILURE);
  }

  char input[32];
  while (1) {
    printf("Type 'exit' to quit the program: ");
    int ret;
    do {
      ret = scanf("%s", input);
    } while (ret == -1 && errno == EINTR);

    if (ret > 0 && strcmp(input, "exit") == 0)
      break;
  }

  /* Display a prompt to the user such that if the "exit"
   * command is typed, the USB DAQ is released and the program
   * is terminated. */
  timer_delete(timer1);
  timer_delete(timer2);
  closeUSBConnection(newhandle);
  return EXIT_SUCCESS;
}
