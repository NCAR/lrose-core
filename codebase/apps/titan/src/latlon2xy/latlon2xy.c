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
/***********************************************************************
 * latlon2xy.c
 *
 * computes xy location from lat lon pairs
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <toolsa/pjg.h>

static void usage(void);

int main(int argc, char **argv)

{

  double start_lat, start_lon, end_lat, end_lon;
  double xx, yy;

  /*
   * check number of args
   */

  if (argc != 5) {
    usage();
  }

  /*
   * set input args
   */

  if (sscanf(argv[1], "%lg", &start_lat) != 1) {
    fprintf(stderr, "Error with start_lat\n");
    usage();
  }

  if (sscanf(argv[2], "%lg", &start_lon) != 1) {
    fprintf(stderr, "Error with start_lon\n");
    usage();
  }

  if (sscanf(argv[3], "%lg", &end_lat) != 1) {
    fprintf(stderr, "Error with end_lat\n");
    usage();
  }

  if (sscanf(argv[4], "%lg", &end_lon) != 1) {
    fprintf(stderr, "Error with end_lon\n");
    usage();
  }

  /*
   * compute (x,y)
   */

  PJGLatLon2DxDy(start_lat, start_lon,
                 end_lat, end_lon,
                 &xx, &yy);

  /*
   * output
   */

  fprintf(stdout, "%10s %10s %10s %10s %10s %10s\n",
	  "start_lat", "start_lon", "end_lat", "end_lon", "x", "y");

  fprintf(stdout, "%10g %10g %10g %10g %10g %10g\n",
	  start_lat, start_lon, end_lat, end_lon, xx, yy);

  return (0);

}

static void usage(void)

{

  fprintf(stderr,
	  "usage: latlon2xy start_lat start_lon end_lat end_lon\n");
  exit(-1);

}

