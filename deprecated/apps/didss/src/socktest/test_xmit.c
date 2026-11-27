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
 * test_xmit()
 *
 * sends packets for testing
 *
 *********************************************************************/

/* ANSI */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


#include "socktest.h"

/*
 * Forward declarations.
 */

static void parse_args(char **argv, int argc);
static void print_usage(void);


/*
 * Global variables from command line.
 */

int Port = DEFAULT_PORT;
int PacketSize = DEFAULT_PACKET_SIZE;

int Forever = TRUE;


int main (int argc, char **argv)
{
  int proto_fd;
  int sock_dev;
  int count = 0;
  char *packet_data;
  long nwritten;

  /*
   * Parse the command line arguments.
   */

  parse_args(argv, argc);
  
  /*
   * Allocate space for the packet data.
   */

  if ((packet_data = malloc(PacketSize)) == NULL)
  {
    fprintf(stderr,
	    "Error allocating %d bytes for outgoing packet -- exiting\n",
	    PacketSize);
    exit(-1);
  }
  
    
  /*
   * open server
   */

  if ((proto_fd =
       SKU_open_server(Port)) < 0) {
    fprintf(stderr, "Cannot bind and listen\n");
    return (-1);
  }
  
  if ((sock_dev =
       SKU_get_client(proto_fd)) < 0) {
    fprintf(stderr, "Cannot get client\n");
    return (-1);
  }
  
  fprintf(stderr, "Got client\n");
  
  while (Forever) {

    nwritten = SKU_write(sock_dev, packet_data,
			 PacketSize, -1);

    if (nwritten != PacketSize)
      fprintf(stderr,
	      "*** Tried to write %d bytes, only wrote %d bytes\n",
	      PacketSize, nwritten);
      
    count++;
    if (count % 50 == 0) {
      fprintf(stderr, "count, nwritten: %d, %d\n", count, nwritten);
    }
    
  }
    
}


static void parse_args(char **argv, int argc)
{
  int i;
  int error = FALSE;
  
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
      fprintf(stderr, "ERROR: Invalid command line argument: %s\n", argv[i]);
	
      error = TRUE;
      break;
    }
      
  } /* endfor - i */
  
  /*
   * Check for errors.
   */

  if (error)
  {
    print_usage();
    exit(-1);
  }
  
  return;
}


static void print_usage(void)
{
  fprintf(stdout, "Usage:  test_xmit [options]\n");
  fprintf(stdout, "Options:    -port <port>          (default = %d)\n", DEFAULT_PORT);
  fprintf(stdout, "            -packet_size <bytes>  (default = %d)\n", DEFAULT_PACKET_SIZE);
  fprintf(stdout, "\n");
  
  return;
}
