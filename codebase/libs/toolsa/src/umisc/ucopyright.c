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
/**********************************************************************
 * ucopyright.c
 *
 * displays copyright message to stderr
 *
 * utility routine
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **********************************************************************/

#include <toolsa/umisc.h>
#include <time.h>

void ucopyright(const char *prog_name)

{

  date_time_t now;

  now.unix_time = time(NULL);
  uconvert_from_utime(&now);

  fprintf(stderr, "======================================================================\n");
  fprintf(stderr, "Program '%s'\n", prog_name);
  fprintf(stderr, "Run-time %s.\n", utimstr(time(NULL)));
  fprintf(stderr, "\n");
  fprintf(stderr, "Copyright (c) 1992 - %.4d\n", now.year);
  fprintf(stderr, "University Corporation for Atmospheric Research (UCAR)\n");
  fprintf(stderr, "National Center for Atmospheric Research (NCAR)\n");
  fprintf(stderr, "Boulder, Colorado, USA.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Redistribution and use in source and binary forms, with\n");
  fprintf(stderr, "or without modification, are permitted provided that the following\n");
  fprintf(stderr, "conditions are met:\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "1) Redistributions of source code must retain the above copyright\n");
  fprintf(stderr, "notice, this list of conditions and the following disclaimer.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "2) Redistributions in binary form must reproduce the above copyright\n");
  fprintf(stderr, "notice, this list of conditions and the following disclaimer in the\n");
  fprintf(stderr, "documentation and/or other materials provided with the distribution.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "3) Neither the name of UCAR, NCAR nor the names of its contributors, if\n");
  fprintf(stderr, "any, may be used to endorse or promote products derived from this\n");
  fprintf(stderr, "software without specific prior written permission.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "4) If the software is modified to produce derivative works, such modified\n");
  fprintf(stderr, "software should be clearly marked, so as not to confuse it with the\n");
  fprintf(stderr, "version available from UCAR.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "======================================================================\n");

  fflush(stderr);

}

