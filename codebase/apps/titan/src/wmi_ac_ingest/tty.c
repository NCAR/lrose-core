/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * tty.c
 *
 * Handles the input serial port
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1996
 *
 ****************************************************************************/

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "wmi_ac_ingest.h"

/*
 * file scope
 */

static char *Tty_name;
static int Tty_fd = -1;
static FILE *Tty_file = NULL;
static struct termios Term_old;
static struct termios Term_new;

static char *speed_str(speed_t speed);

/*******************************************************
 *
 * tty_open()
 *
 * open serial port
 *
 * set the charateristic for the particular tty to be:
 *   
 * Input Modes:	IGNBRK (ignore break)
 * ICRNL (terminate by <cr>)
 * 
 * Local Modes:	ICANON (cannonical input processing)
 * ~ECHO (no echo)
 * ~ISIG (signal characters off)
 * ~IEXTEN (extended input processing off)
 * 
 * Output Modes: ~OPOST (output processing off),
 * 
 * Control Modes: B1200 (1200 bps)
 * CS8 (8-bit character with no parity)
 * 
 * and return file pointer
 */

FILE *tty_open(char *ttyname, int baudrate)

{

  speed_t speed;
  speed_t ispeed;
  speed_t ospeed;

  /*
   * copy in name
   */
  
  Tty_name = umalloc(strlen(ttyname) + 1);
  strcpy(Tty_name, ttyname);
  
  Tty_fd = open (ttyname, O_RDONLY);
  
  if (Tty_fd < 0) {
    perror (Tty_name);
    return (NULL);
  }
	
  /*
   * get the termio characterstics for the tty
   */
  
  if (tcgetattr(Tty_fd, &Term_old) < 0) {
    fprintf(stderr, "Error on tcgetattr()\n");
    perror (Tty_name);
    return(NULL);
  }
  
  /*
   * copy terminal characteristics
   */

  Term_new = Term_old;

  /*
   * set desired characteristics
   */

  Term_new.c_lflag |= ICANON;
  Term_new.c_lflag &= ~(ISIG | ECHO | IEXTEN);

  Term_new.c_iflag |= (IGNBRK | IGNCR);

#ifdef NOTNOW
  Term_new.c_iflag &= ~IGNCR;
  Term_new.c_oflag &= ~OPOST;
  Term_new.c_cflag = 0;
#endif

  if (tcsetattr(Tty_fd, TCSADRAIN, &Term_new) < 0) {
    fprintf(stderr, "Error on tcsetattr()\n");
    perror (Tty_name);
    return(NULL);
  }
  
  switch (baudrate) {

  case BAUD_300:
    speed = B300;
    break;

  case BAUD_1200:
    speed = B1200;
    break;

  case BAUD_2400:
    speed = B2400;
    break;

  case BAUD_4800:
    speed = B4800;
    break;

  case BAUD_9600:
    speed = B9600;
    break;

  case BAUD_19200:
    speed = B19200;
    break;

  case BAUD_38400:
    speed = B38400;
    break;

  default:
    speed = B9600;

  }

  cfsetispeed(&Term_new, speed);
  cfsetospeed(&Term_new, speed);
  
  ispeed = cfgetospeed(&Term_new);
  ospeed = cfgetispeed(&Term_new);

  if (Glob->params.debug) {
    fprintf(stderr, "output speed %s\n", speed_str(ospeed));
    fprintf(stderr, "input speed %s\n", speed_str(ispeed));
  }

  return (Tty_file = (FILE *) fdopen(Tty_fd, "r"));

}

char tty_read_char(void)

{
  
  char c;
  
  if (read(Tty_fd, &c, 1) != 1) {
    return (-1);
  } else {
    return (c);
  }

}

void tty_close(void)

{
  
  if (Tty_file != NULL) {

    /*
     * reset port to earlier state
     */

    if (tcsetattr(Tty_fd, TCSADRAIN, &Term_old) < 0) {
      fprintf(stderr, "Error on tcsetattr()\n");
      perror (Tty_name);
    }
  
    /*
     * close the serial port
     */
    
    fclose(Tty_file);

  }

  return;

}

static char *speed_str(speed_t speed)

{

  switch (speed) {

  case B300:
    return ("300");
    break;

  case B1200:
    return ("1200");
    break;

  case B2400:
    return ("2400");
    break;

  case B4800:
    return ("4800");
    break;

  case B9600:
    return ("9600");
    break;

  case B19200:
    return ("19200");
    break;

  case B38400:
    return ("38400");
    break;

  default:
    return ("unknown");

  }
  
}


