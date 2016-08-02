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
/*******************************************************************************
  * uintersect.c : tests whether two straight lines intersect
  *
  * utility routine
  *
  * args:
  *       xx1, yy1, xx2, yy2 - floats - end points of one line
  *       uu1, vv1, uu2, vv2 - floats - end points of the other line
  *
  * returns 1 if they intersect, 0 if not
  *
  * Mike Dixon, RAP, NCAR, August 1990
  ******************************************************************************/

#include <rapmath/umath.h>
#include <math.h>

double get_dirn(double xx1, double yy1, double xx2, double yy2);

int uintersect(double xx1, double yy1, double xx2, double yy2,
	       double uu1, double vv1, double uu2, double vv2)
{

  double dirn[5];
  double angle_turned = 0;
  double check_value;
  int i;

  /*
   * check if any of the end points intersect
   */

  if (xx1 == uu1 && yy1 == vv1)
    return(1);

  if (xx1 == uu2 && yy1 == vv2)
    return(1);

  if (xx2 == uu1 && yy2 == vv1)
    return(1);

  if (xx2 == uu2 && yy2 == vv2)
    return(1);

  
  /*
   * get the direction of the lines (xx1, yy1) to (uu1, vv1)
   *                                (uu1, vv1) to (xx2, yy2)
   *                                (xx2, yy2) to (uu2, vv2)
   *                                (uu2, vv2) to (xx1, yy1)
   */

  dirn[0] = get_dirn(xx1, yy1, uu1, vv1);
  dirn[1] = get_dirn(uu1, vv1, xx2, yy2);
  dirn[2] = get_dirn(xx2, yy2, uu2, vv2);
  dirn[3] = get_dirn(uu2, vv2, xx1, yy1);
  dirn[4] = dirn[0];

  /*
   * sum the angle turned left to move along these lines
   * to complete one circuit
   */

  for (i = 0; i < 4; i++) {

    if (dirn[i] == dirn[i+1]) {
      return(1); /* two dirns equal, lines intersect */
    }

    if (dirn[i] <= dirn[i+1])
      angle_turned += dirn[i+1] - dirn[i];
    else
      angle_turned += dirn[i+1] + TWO_PI - dirn[i];

  }

  check_value = angle_turned / M_PI;

  if ((check_value > 1.99 && check_value < 2.01) ||
      (check_value > 5.99 && check_value < 6.01)) {
    return(1);
  } else if (check_value > 3.99 && check_value < 4.01) {
    return(0);
  } else {
    fprintf(stderr, "ERROR - uintersect\n");
    fprintf(stderr, "Angle turned not logical.\n");
    return(0);
  }

}
      
/****************************************************************************
 * get_dirn: gets angle of a line relative to the x axis,
 * given the end points.
 */

double get_dirn(double xx1, double yy1, double xx2, double yy2)
{
  
  double dirn;

  if (xx1 == xx2) {

    if (yy1 < yy2)
      return(M_PI_2);
    else if (yy1 > yy2)
      return(M_PI + M_PI_2);
    else
      return((double) 0.0);

  }

  dirn = atan2((double)(yy2 - yy1), (double) (xx2 - xx1));

  if (dirn < 0)
    dirn += TWO_PI;

  return dirn;

}
