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
/****************************************************************************
 * copy_mh_tape.c : copies a mile high radar tape from one exabyte to another
 *
 * Mike Dixon  RAP NCAR   Sept 1990
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>

#if !defined(__linux)
extern int ioctl (int fildes, int request, ...);
#endif
static int write_eof(int tape, int num);

#define MAXRECSIZE (int) 65536
#define MAXFILES 9999999

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  unsigned char buffer[MAXRECSIZE];
  char *in_tape_name, *out_tape_name, *endpt;
  int nfiles;
  int in_tape, out_tape;
  int nread, nwritten;
  int file_num = 0;
  int eof_flag = 0; /* end-of-file flag */
  int eot_flag = 0; /* end-of-tape flag */
  int bof_flag = 1; /* beginning-of-file flag */

  /*
   * check command line args
   */

  if (argc != 3 && argc != 4) {
    fprintf(stderr, "Usage: %s input-tape output-tape [nfiles]\n", argv[0]);
    exit(1);
  }

  /*
   * copy command line args to file names
   */

  in_tape_name = (char *) malloc((u_int) strlen(argv[1]) + 1);
  strcpy(in_tape_name, argv[1]);
  out_tape_name = (char *) malloc((u_int) strlen(argv[2]) + 1);
  strcpy(out_tape_name, argv[2]);

  if (argc == 4) {
    errno = 0;
    nfiles = strtol(argv[3], &endpt, 10);
    if (errno != 0)
      nfiles = MAXFILES;
  } else {
    nfiles = MAXFILES;
  }

  /*
   * open tape units
   */

  if((in_tape = open(in_tape_name, O_RDONLY)) < 0) {
    fprintf(stderr, "\nERROR - copy_mh_tape.\n");
    fprintf(stderr, "Opening input tape unit '%s'\n", in_tape_name);
    perror(" ");
    exit(1);
  }

  if((out_tape = open(out_tape_name, O_RDWR)) < 0) {
    fprintf(stderr, "\nERROR - copy_mh_tape.\n");
    fprintf(stderr, "Opening output tape unit '%s'\n", out_tape_name);
    perror(" ");
    exit(1);
  }

  /*
   * copy data until logical end of tape found - 2 end-of-files
   */

  printf("Program %s\n\n", argv[0]);
  printf("Copying tape '%s' to '%s'\n", in_tape_name, out_tape_name);

  while(eot_flag != 1) {

    nread = read (in_tape, (char *) buffer, MAXRECSIZE);

    if (nread <= 0) {

      if (eof_flag >= 2) {
	printf ("Logical end of tape encountered\n");
	eot_flag = 1;
      } else {
	eof_flag++;
	bof_flag = 1;

	if (write_eof(out_tape, 1) != 0) {
	  fprintf(stderr, "ERROR - copy_mh_tape\n");
	  fprintf(stderr, "Writing end of file.\n");
	  perror(out_tape_name);
	  exit(1);
	}

	/*
	 * if nfiles written, write second eof and quit
	 */

	if (file_num == nfiles) {

	  if (write_eof(out_tape, 1) != 0) {
	    fprintf(stderr, "ERROR - copy_mh_tape\n");
	    fprintf(stderr, "Writing end of file.\n");
	    perror(out_tape_name);
	    exit(1);
	  }

	  exit(0);

	}
	 	
      }

    } else {

      /*
       * if beginning of file ...
       */

      if (bof_flag == 1) {
	file_num++;
	printf("Copying file number %3d\n", file_num);
	bof_flag = 0;
	eof_flag = 0;
      }

      nwritten = write(out_tape, (char *) buffer, nread);

      if (nwritten != nread) {

	fprintf(stderr, "ERROR - copy_mh_tape.\n");
	fprintf(stderr, "Writing record to output tape.\n");
	fprintf(stderr, "%d bytes read, %d bytes written.\n",
		nread, nwritten);
	perror(out_tape_name);
	exit(1);

      }

    } /* if (nread <= 0) */

  } /* while (eot_flag != 1) */

  /*
   * close tape units
   */

  close(in_tape);
  close(out_tape);

  return(0);

}

/***************************************************************************
 * write_eof.c: writes end-of-file record
 *
 * returns -1 if error, 0 if success
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

static int write_eof(int tape, int num)

{

  struct mtop mt_command;

  mt_command.mt_op = MTWEOF;
  mt_command.mt_count = num;
  return ioctl(tape, MTIOCTOP, &mt_command);

}
