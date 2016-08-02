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
#ifndef EUCLID_SEARCH_H
#define EUCLID_SEARCH_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <euclid/point.h>

extern int EGS_line_through_ellipse(double major_radius,
                                    double minor_radius,
                                    double slope,
                                    double intercept,
                                    double *xx1,
                                    double *yy1,
                                    double *xx2,
                                    double *yy2);
   
extern int EGS_point_in_ellipse(double ellipse_x, double ellipse_y,
                                double major_radius, double minor_radius,
                                double axis_rotation,
                                double search_x, double search_y,
                                double search_radius_ratio);

extern int EGS_point_in_polygon(Point_d pt, Point_d *vertex, int nvertices);
extern int EGS_point_in_polygon2(Point_d pt, Point_d *vertex, int nvertices);

#ifdef __cplusplus
}
#endif

#endif

