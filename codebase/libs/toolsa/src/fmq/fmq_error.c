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
/******************************************************************
 * fmq_error.c
 *
 * Error routines for FMQ
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#include <stdarg.h>
#include <toolsa/fmq_private.h>

/*******************
 * fmq_print_error
 *
 * Prints error message to stderr.
 * If format is not NULL, it is used to print the remaining
 * args in the var_args list.
 *
 */

void fmq_print_error(FMQ_handle_t *handle,
		     char *routine,
		     char *format,
		     ...)
     
{

  va_list args;

  fprintf(stderr, "ERROR - %s:%s\n", handle->prog_name, routine);
  fprintf(stderr, "FMQ path '%s'\n", handle->fmq_path);
  if (format != NULL) {
    va_start (args, format);
    vfprintf (stderr, format, args);
    va_end (args);
  }
}

/***********************
 * fmq_print_debug_error
 *
 * Prints error message to stderr if debug is set.
 *
 * If format is not NULL, it is used to print the remaining
 * args in the var_args list.
 *
 */

void fmq_print_debug_error(FMQ_handle_t *handle,
			   char *routine,
			   char *format,
			   ...)
     
{

  va_list args;

  if (handle->debug) {
    fprintf(stderr, "ERROR - %s:%s\n", handle->prog_name, routine);
    fprintf(stderr, "FMQ path '%s'\n", handle->fmq_path);
    if (format != NULL) {
      va_start (args, format);
      vfprintf (stderr, format, args);
      va_end (args);
    }
  }

}

/*********************
 * fmq_print_debug_msg
 *
 * Prints message to stderr if debug is set.
 *
 * If format is not NULL, it is used to print the remaining
 * args in the var_args list.
 *
 */

void fmq_print_debug_msg(FMQ_handle_t *handle,
			 char *routine,
			 char *format,
			 ...)
     
{

  va_list args;

  if (handle->debug) {
    fprintf(stderr, "MESSAGE - %s:%s\n", handle->prog_name, routine);
    fprintf(stderr, "FMQ path '%s'\n", handle->fmq_path);
    if (format != NULL) {
      va_start (args, format);
      vfprintf (stderr, format, args);
      va_end (args);
    }
  }

}

/*********************
 * fmq_debug_perror
 *
 * Performs perror if debug is set.
 *
 */

void fmq_debug_perror(FMQ_handle_t *handle,
		      char *label)
     
{

  if (handle->debug) {
    perror(label);
  }

}


