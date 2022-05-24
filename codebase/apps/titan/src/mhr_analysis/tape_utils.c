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
 * tape utility routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 **************************************************************************/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <unistd.h>

#ifdef JUNK
#include <sys/stat.h>
#include <fcntl.h>
#endif

int fwd_space_file(int tape, int nfiles);
int fwd_space_record(int tape, int nrec);
struct mtget *get_tape_status(int tape);
int rewind_tape(int tape);

/***************************************************************************
 * fwd_space_file()
 *
 * spaces fwd tape by given number of files
 *
 * returns -1 if error, 0 if success
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

int fwd_space_file(int tape, int nfiles)

{

  struct mtop mt_command;
  int retval;

  mt_command.mt_op = MTFSF;
  mt_command.mt_count = nfiles;
  retval = ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);
  return retval;

}

/***************************************************************************
 * fwd_space_record()
 *
 *  spaces fwd tape by given number of records
 *
 * returns -1 if error, 0 if success
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

int fwd_space_record(int tape, int nrec)

{

  struct mtop mt_command;
  int retval;

  mt_command.mt_op = MTFSR;
  mt_command.mt_count = nrec;
  retval = ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);
  return retval;

}

/***************************************************************************
 * get_tape_status()
 *
 * returns pointer to tape status
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

struct mtget *get_tape_status(int tape)

{

  static struct mtget mt_status;

  ioctl(tape, MTIOCGET, (char *) (caddr_t) &mt_status);

  return &mt_status;

}

/***************************************************************************
 * rewind_tape()
 *
 *  rewinds tape
 *
 * returns -1 if error, 0 if success
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

int rewind_tape(int tape)

{

  struct mtop mt_command;

  mt_command.mt_op = MTREW;
  return ioctl(tape, MTIOCTOP, (caddr_t) &mt_command);

}
