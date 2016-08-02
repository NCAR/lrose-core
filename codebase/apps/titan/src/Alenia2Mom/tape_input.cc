// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/****************************************************************************
 * tape_input.c - tape read module
 *
 *      this routine reads tape data, seqments the data into logical records,
 *      and divides each record into simulated ethernet packets.  The 
 *	justification for this methodology is to provide a mechanism for 
 *	testing modifications made to the formatter without requiring the radar
 *	to be on line.  
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1991
 *
 * Gary Blackburn
 *
 ****************************************************************************/

#include "Alenia2Mom.h"
#include <dataport/bigend.h>
#include <toolsa/ttape.h>
#include <toolsa/sockutil.h>
using namespace std;

#define MAX_TAPE_READ 65536

/*
 * file scope
 */

static int Debug = FALSE;
static int Remote_tape;
static int Tape_fd = -1;
static int Wait_usecs;
static int Init_done = FALSE;

static int get_physical_rec (ui08 *buffer);

/**************************************************************************
 * open_input_tape()
 *
 * Open tape device, wait until ready
 */
  
int open_input_tape(char *device, int wait_msecs, int debug)

{

  Debug = debug;
  Wait_usecs = wait_msecs * 1000;

  if (strchr(device, ':') == NULL) {

    Remote_tape = FALSE;
    if ((Tape_fd = open (device, O_RDONLY)) < 0) {
      perror (device);
      return (-1);
    }
    TTAPE_set_var(Tape_fd);

    if (Debug) {
      fprintf (stderr, "Successfully opened tape %s, fd = %d\n",
	       device, Tape_fd);
    }
    
  } else {

    Remote_tape = TRUE;
    if (RMT_open (device, O_RDONLY, 0666) < 0) {
      perror (device);
      return (-1);
    }
    
    if (Debug) {
      fprintf (stderr, "Successfully opened remote tape %s\n",
	       device);
    }
    
  }

  Init_done = TRUE;
  return (0);

}

/******************************************************************
 * Read a tape record. Checks for the correct record type. If
 * correct, setns number of bytes read and returns. Otherwise
 * keeps reading for good record.
 *
 * Returns:
 *   0 on success, -1 on error.
 *
 */

int read_input_tape (ui08 **buf_p, int *len_p)
     
{
     
  static ui08 physical_rec[MAX_TAPE_READ];
  int nread;

  if (!Init_done) {
    fprintf(stderr, "ERROR - read_input_tape - init not done\n");
    return (-1);
  }
  
  nread = get_physical_rec (physical_rec);
  *buf_p = physical_rec;
  *len_p = nread;

  return (0);

}

/******************************************************************
 * close_input_tape()
 *
 */

void close_input_tape(void)

{
  if (Init_done) {
    if (Tape_fd >= 0) {
      close(Tape_fd);
      Tape_fd = -1;
      if (Debug) {
	fprintf(stderr, "Closing tape\n");
      }
    }
  }
}

/*------------------------------------------------------------------------
 * get_physical_rec()
 *
 * Gets a physical tape record.
 *
 * If successful, returns number of bytes read.
 * If failure, returns -1.
 */


static int get_physical_rec (ui08 *buffer)
     
{

  int nread;
  int nerr;

  PMU_auto_register("Reading physical tape record");

  if (Remote_tape) {

    return (RMT_read ((char *) buffer, MAX_TAPE_READ));

  } else {

    PMU_auto_register("Reading tape");

    nerr = 0;
    while (nerr < 50) {

      errno = EINTR;
      while (errno == EINTR) {
	errno = 0;
	nread = read (Tape_fd, (char *) buffer, MAX_TAPE_READ);
      }
      
      if (nread > 0) {
	return (nread);
      } else {
	nerr++;
      }

      uusleep(1000);

    }

    fprintf(stderr, "Tape not ready, or no data left\n");
    return (-1);

  }

}



