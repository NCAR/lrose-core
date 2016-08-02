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
 * calc_range.c
 *
 * calculates range at which a radar beam reaches a given
 * altitude.
 *
 * The first arg is the elevation angle in degrees.
 * The second arg is the height in feet.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA.
 *
 * April 1992
 *
 *********************************************************************/

#define PI 3.14159265358987
#define RAD_TO_DEG 57.29577951308092
#define DEG_TO_RAD 0.01745329251994372
#define PSEUDO_RADIUS 8533.0

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char **argv)
{

  char *end_pt;
  double elev, sin_elev;
  double height, height_km, beam_ht;
  double slant_range;
  
  /*
   * check no of args
   */

  if (argc != 3) {
    fprintf(stderr, "Usage: %s elev(deg) height(ft)\n", argv[0]);
    return (1);
  }

  /*
   * decode args
   */

  errno = 0;
  elev = strtod(argv[1], &end_pt);
  if (errno) {
    fprintf(stderr, "Usage: %s elev(deg) height(ft)\n", argv[0]);
    return (1);
  }

  errno = 0;
  height = strtod(argv[2], &end_pt);
  if (errno) {
    fprintf(stderr, "Usage: %s elev(deg) height(ft)\n", argv[0]);
    return (1);
  }

  /*
   * get sin of elev 
   */

  sin_elev = sin(elev * DEG_TO_RAD);
  
  /*
   * convert height to km
   */

  height_km = height * 0.3048 / 1000.0;

  slant_range = 0.0;
  beam_ht = 0.0;

  while (beam_ht < height_km) {

    beam_ht = (slant_range * sin_elev +
	       slant_range * slant_range / (2.0 * PSEUDO_RADIUS));

    slant_range += 1.0;
    
  } /* while */

  printf("Range, ht = %g, %g\n", slant_range, beam_ht);

  return (0);

}
