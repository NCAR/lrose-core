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
/*********************************************************************
 * test_recv()
 *
 * reads packets for testing using the sockutil module of the toolsa
 * library
 *
 *********************************************************************/

/* ANSI */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>

#include <toolsa/sockutil.h>

#include "socktest.h"


/*
 * Forward declarations.
 */

static void parse_args(char **argv, int argc);
static void print_usage(void);


/*
 * Global variables from command arguments.
 */

int PacketSize = DEFAULT_PACKET_SIZE;
int Port = DEFAULT_PORT;
char *Host = NULL;

int Forever = TRUE;


int main (int argc, char **argv)
{
  int sock_dev;
  int count = 0;
  int tot_bytes = 0;
  char *packet_data;
  
  int read_success;
  long nread;

  time_t start, now;

  /*
   * Parse the command line arguments.
   */

  parse_args(argv, argc);
  
  /*
   * Allocate space for the incoming packet.
   */

  if ((packet_data = malloc(PacketSize)) == NULL)
  {
    fprintf(stderr,
	    "Error allocating %d bytes for incoming packet -- exiting\n",
	    PacketSize);
    exit(-1);
  }
  
  /*
   * try to connect
   */

  if ((sock_dev =
       SKU_open_client(Host, Port)) < 0) {
    fprintf(stderr, "Cannot connect\n");
    return (-1);
  }

  fprintf(stderr, "Connected to server\n");

  start = time(NULL);

  while (Forever) {

    nread = SKU_read(sock_dev, packet_data,
		     PacketSize, -1);

    if (nread == 0) {
      exit(-1);
    }

    if (nread != PacketSize)
      fprintf(stderr, "*** Expected %d bytes, read %d bytes\n",
	      PacketSize, nread);
      
    count++;
    tot_bytes += nread;

    if (count % 50 == 0) {
      now = time(NULL);
      fprintf(stderr,
	      "count, nread, rate(byte/sec): %d, %d, %lg\n",
	      count, nread,
	      (double) tot_bytes / (double) (now - start));
    }
    
  }
    
}


static void parse_args(char **argv, int argc)
{
  int i;
  int error = FALSE;
  int host_found = FALSE;
  
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-port") == 0)
    {
      i++;
      if (i >= argc)
      {
	fprintf(stderr, "ERROR:  Port number not specified\n\n");
	
	error = TRUE;
	break;
      }
      
      Port = atoi(argv[i]);
    }
    else if (strcmp(argv[i], "-packet_size") == 0)
    {
      i++;
      if (i >= argc)
      {
	fprintf(stderr, "ERROR: Packet size not specified\n\n");
	
	error = TRUE;
	break;
      }
      
      PacketSize = atoi(argv[i]);
    }
    else
    {
      if (i != argc-1 ||
	  argv[i][0] == '-')
      {
	fprintf(stderr, "ERROR: Invalid command line argument: %s\n", argv[i]);
	
	error = TRUE;
	break;
      }
      
      host_found = TRUE;
      Host = argv[i];
    }
      
  } /* endfor - i */
  
  /*
   * Check for errors.
   */

  if (!host_found)
    fprintf(stderr, "ERROR: Host name not specified\n\n");
  
  if (error || !host_found)
  {
    print_usage();
    exit(-1);
  }
  
  return;
}


static void print_usage(void)
{
  fprintf(stdout, "Usage:  test_recv [options] host\n");
  fprintf(stdout, "Options:    -port <port>          (default = %d)\n", DEFAULT_PORT);
  fprintf(stdout, "            -packet_size <bytes>  (default = %d)\n", DEFAULT_PACKET_SIZE);
  fprintf(stdout, "\n");
  
  return;
}
