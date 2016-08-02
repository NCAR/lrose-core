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
/******************************************************
 * ttape.h
 *
 * Tololsa Tape utilities
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#ifndef TTAPE_H
#define TTAPE_H

#ifdef __cplusplus
 extern "C" {
#endif

/***************************************************************************
 * TTAPE_fwd_space_file()
 *
 * spaces fwd tape by given number of files
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_fwd_space_file(int tape, int nfiles);

/***************************************************************************
 * TTAPE_fwd_space_record()
 *
 * spaces fwd tape by given number of records
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_fwd_space_record(int tape, int nrec);

/***************************************************************************
 * TTAPE_write_eof()
 *
 * Writes 'n' End-Of-File marks.
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_write_eof(int tape, int n);

/***************************************************************************
 * TTAPE_rewind()
 *
 * rewinds tape
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_rewind(int tape);

/***************************************************************************
 * TTAPE_rewoffl()
 *
 * rewinds tape and puts offline (ejects Exabyte)
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_rewoffl(int tape);

/***************************************************************************
 * TTAPE_set_var()
 *
 * Set variable block size mode
 *
 * returns -1 if error, 0 if success
 *
 **************************************************************************/

extern int TTAPE_set_var(int tape);

/***************************************************************************
 * REMOTE TAPE functions
 *
 * Code copied from Gary Blackburn, May 1997
 ***************************************************************************/

/* Open a tape device on the system specified in PATH, as the given user.
   PATH has the form `[user@]system:/dev/????'.
   If successful, return the remote tape pipe number.  On error, return -1.  
 */

extern int RMT_open (char *path, int oflag, int mode);

/***************************************************************************/

/* Close remote tape connection file descriptor and shut down.
Return 0 if successful, -1 on error.  */

extern int RMT_close (void);

/***************************************************************************/

/* Read up to NBYTE bytes into BUF from remote tape connection file descriptor.
Return the number of bytes read on success, -1 on error.  */

extern int RMT_read (char *buf, unsigned int nbyte);

/***************************************************************************/

/* Write NBYTE bytes from BUF to remote tape connection file descriptor.
   Return the number of bytes written on success, -1 on error.  */

extern int RMT_write (char *buf, unsigned int nbyte);

/* Perform an imitation lseek operation on remote tape connection file desc.
   Return the new file offset if successful, -1 if on error.  */

/***************************************************************************/

extern long RMT_lseek (long offset, int whence);

/* Perform a raw tape operation on remote tape connection file descriptor.
   Return the results of the ioctl, or -1 on error.  */

extern int RMT_ioctl (int op, char *arg);


#ifdef __cplusplus
}
#endif

#endif

