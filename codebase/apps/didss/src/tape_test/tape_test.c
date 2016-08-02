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
/*
 * tape_test.c - reads tape, prints out block sizes
 */

#include <toolsa/umisc.h>
#include <toolsa/ttape.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define MAX_TAPE_BLOCK 65536

static void print_usage(char **argv, FILE *out)

{
  fprintf(out, "Usage: %s tape_name\n", argv[0]);
}

/*************************************************************/

int main (int argc, char **argv)

{

  char *tape_name;
  char buf[MAX_TAPE_BLOCK];
  int nerr = 0;
  int nread;
  int tape_fd;
  int forever = TRUE;

  /*
   * check args
   */

  if (argc < 2) {
    print_usage(argv, stderr);
    exit (-1);
  }

  /*
   * get tape name
   */

  tape_name = argv[1];

  /*
   * open tape
   */
  
  if ((tape_fd = open (tape_name, O_RDONLY)) < 0) {
    fprintf(stderr, "Cannot open tape '%s'\n", tape_name);
    perror (tape_name);
    return (-1);
  }

  /*
   * set variable block size
   */
  
  TTAPE_set_var(tape_fd);

  while (forever) {

    nread = read (tape_fd, buf, MAX_TAPE_BLOCK);

    if (nread <= 0) {
      fprintf(stderr, "Error reading tape, nread = %d\n", nread);
      nerr++;
    } else {
      fprintf(stderr, "Read block, %d bytes\n", nread);
      nerr = 0;
    }

    uusleep(100);

    if (nerr > 20) {
      fprintf(stderr, "Too many errors reading tape %s\n", tape_name);
      return (-1);
    }

  }

  return (0);

}
