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
 * print_comments.c
 *
 * Prints comments lines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Sept 1997
 *
 ****************************************************************************/

#include "storms_to_tifs.h"

void print_comments(FILE *out)

{

  fprintf(out, "# ");
  fprintf(out, "year, month, day, hour, min, sec, x (km), y (km), ");
  fprintf(out, "lat (deg), lon (deg), ");
  fprintf(out, "precip area (km2), precip rate (mm/hr), ");
  fprintf(out, "major radius (km), minor radius (km), ");
  fprintf(out, "orientation from (deg TN), ");
  fprintf(out, "volume (km3), mass (ktons), top (km msl), ");
  fprintf(out, "max dBZ, mean dBZ, ");
  fprintf(out, "%% vol > 40 dBZ, %% vol > 50 dBZ, ");
  fprintf(out, "%% vol > 60 dBZ, %% vol > 70 dBZ, ");
  fprintf(out, "speed (km/hr), dirn (deg T), ");
  fprintf(out, "dvol_dt (km3/hr), precip_area_dt (km2/h)\n");

}
