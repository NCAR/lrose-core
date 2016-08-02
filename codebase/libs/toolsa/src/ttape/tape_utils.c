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
 * tape_utils.c
 *
 * toolsa tape utility routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *
 **************************************************************************/

#include <toolsa/ttape.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#if defined (AIX)
#include <sys/tape.h>
#elif defined (__linux)
#include <sys/mtio.h>
#endif

#if defined (__linux)
# ifndef __daddr_t_defined
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;
#  define __daddr_t_defined
# endif
#endif

/***************************************************************************
 * TTAPE_fwd_space_file()
 *
 * spaces fwd tape by given number of files
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_fwd_space_file(int tape, int nfiles)

{

#if defined (AIX)

  struct stop st_command;
  int retval;

  st_command.st_op = STFSF;
  st_command.st_count = nfiles;
  retval = ioctl(tape, STIOCTOP, (caddr_t) &st_command);
  return retval;

#elif defined (__linux)

  struct mtop mt_command;
  int retval;

  mt_command.mt_op = MTFSF;
  mt_command.mt_count = nfiles;
  retval = ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);
  return retval;

#else

  return -1;

#endif

}

/***************************************************************************
 * TTAPE_fwd_space_record()
 *
 * spaces fwd tape by given number of records
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_fwd_space_record(int tape, int nrec)

{

#if defined (AIX)

  struct stop st_command;
  int retval;

  st_command.st_op = STFSR;
  st_command.st_count = nrec;
  retval = ioctl(tape, STIOCTOP, (caddr_t) &st_command);
  return retval;

#elif defined (__linux)

  struct mtop mt_command;
  int retval;

  mt_command.mt_op = MTFSR;
  mt_command.mt_count = nrec;
  retval = ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);
  return retval;

#else

  return -1;

#endif

}

/***************************************************************************
 * TTAPE_write_eof()
 *
 * Writes 'n' End-Of-File marks.
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_write_eof(int tape, int n)

{

#if defined (AIX)

  struct stop st_command;
  int retval;

  st_command.st_op = STWEOF;
  st_command.st_count = n;
  retval = ioctl(tape, STIOCTOP, (caddr_t) &st_command);
  return retval;

#elif defined (__linux)

  struct mtop mt_command;
  int retval;

  mt_command.mt_op = MTWEOF;
  mt_command.mt_count = n;
  retval = ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);
  return retval;

#else

  return -1;

#endif

}

/***************************************************************************
 * TTAPE_rewind()
 *
 * rewinds tape
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_rewind(int tape)

{

#if defined (AIX)

  struct stop st_command;

  st_command.st_op = STREW;
  return ioctl(tape, STIOCTOP, (caddr_t) &st_command);

#elif defined (__linux)

  struct mtop mt_command;

  mt_command.mt_op = MTREW;
  return ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);

#else

  return -1;

#endif

}

/***************************************************************************
 * TTAPE_rewoffl()
 *
 * rewinds tape and puts offline (ejects Exabyte)
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_rewoffl(int tape)

{

#if defined (AIX)

  struct stop st_command;

  st_command.st_op = STOFFL;
  return ioctl(tape, STIOCTOP, (caddr_t) &st_command);

#elif defined (__linux)

  struct mtop mt_command;

  mt_command.mt_op = MTOFFL;
  return ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);

#else

  return -1;

#endif

}

/***************************************************************************
 * TTAPE_set_var()
 *
 * Set variable block size mode
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

int TTAPE_set_var(int tape)

{

#if defined(__linux)

  struct mtop mt_command;

  mt_command.mt_op = MTSETBLK;
  mt_command.mt_count = 0;
  return ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);

#else

  return 0;

#endif

}


