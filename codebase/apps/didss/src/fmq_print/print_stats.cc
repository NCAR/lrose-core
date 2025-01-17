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
 * print_stats.c
 *
 * Print out simple FMQ stats
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "fmq_print.hh"

int print_stats(char *fmq_path)

{

  FMQ_handle_t fmq;
  double slots_usage, buffer_usage;

  if (FMQ_init(&fmq, fmq_path, FALSE, 
	       Glob->prog_name)) {
    fprintf(stderr, "FMQ_init failed.\n");
    return (-1);
  }

  if (FMQ_open_rdonly(&fmq)) {
    fprintf(stderr, "FMQ_open_rdonly failed.\n");
    return (-1);
  }

  if (FMQ_print_stat(&fmq, stdout)) {
    fprintf(stderr, "FMQ_print_stat failed.\n");
    return (-1);
  }

  if (FMQ_fraction_used(&fmq, &slots_usage, &buffer_usage)) {
    fprintf(stderr, "FMQ_fraction_used failed.\n");
    return (-1);
  }

  fprintf(stdout, "\n");
  fprintf(stdout, "Percent slots  used: %5.1f %%\n",
	  slots_usage * 100.0);
  fprintf(stdout, "Percent buffer used: %5.1f %%\n",
	  buffer_usage * 100.0);
  fprintf(stdout, "\n");

  FMQ_free(&fmq);

  return (0);

}
