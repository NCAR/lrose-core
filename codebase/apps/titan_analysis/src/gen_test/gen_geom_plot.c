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
  gen_geom_plot.c : generates plot file of radar geometry
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <toolsa/umisc.h>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453293
#endif

#define NELEVATIONS 21
#define ELEVATION_STR "0.2 0.5 0.8 1.2 1.6 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.2 7.0 7.7 8.5 9.2 10.0 13.0 17.0"
#define NGATES 660
#define GATE_SPACING 225.0
#define START_RANGE 112.5
#define MAX_RANGE 150000
#define START_ALTITUDE 1500.0
#define DELTA_ALTITUDE 1000.0
#define MAX_ALTITUDE 10000.0
#define RADAR_ALTITUDE 1604.0
#define PSEUDO_RADIUS 8533.0

int main(int argc, char **argv)
{

  si32 i, j;
  FILE *plot_file;

  char *elevation_str;
  char *start_pt, *end_pt;

  si32 nelevations = NELEVATIONS;
  si32 ngates = NGATES;

  double elevations[NELEVATIONS];
  double start_range = START_RANGE;
  double gate_spacing = GATE_SPACING;
  double max_range = MAX_RANGE;
  double start_altitude = START_ALTITUDE;
  double delta_altitude = DELTA_ALTITUDE;
  double max_altitude = MAX_ALTITUDE;
  double radar_altitude = RADAR_ALTITUDE;
  double cosphi, sinphi;
  double slant_range, gnd_range, beam_ht;
  double twice_radius = 2000.0 * PSEUDO_RADIUS;
  double altitude;

  /*
   * display ucopyright message
   */

  ucopyright("gen_geom_plot");

  /*
   * check no of args
   */

  if (argc != 2) {
    fprintf(stderr, "Usage: %s file-name\n", argv[0]);
    exit(1);
  }

  /*
   * open plot file
   */

  if((plot_file = fopen(argv[1], "w")) == NULL) {
    fprintf(stderr, "ERROR -  creating test file\n");
    perror(argv[1]);
    exit(1);
  }

  /*
   * write header to file
   */

  fprintf(plot_file, "Device: Postscript\n");
  fprintf(plot_file, "Disposition: To Device\n");
  fprintf(plot_file, "Geometry: 900x500+0+0\n");
  fprintf(plot_file, "BoundBox: True\n");
  fprintf(plot_file, "Ticks: True\n");
  fprintf(plot_file, "TitleFont: Helvetica12\n");
  fprintf(plot_file, "StyleMarkers: False\n");
  fprintf(plot_file, "XUnitText: Range (m)\n");
  fprintf(plot_file, "YUnitText: Altitude (m)\n");
  fprintf(plot_file, "FileorDev: lp1\n");
  fprintf(plot_file, "TitleText: Mile High Radar Geometry\n");

  /*
   * fill in elevations and field addresses
   */

  elevation_str = ELEVATION_STR;

  end_pt = elevation_str;

  for (i = 0; i < nelevations; i++) {
    start_pt = end_pt;
    elevations[i] = strtod(start_pt, &end_pt);
  }

  /*
   * plot the beam line for each elevation
   */

  for (i = 0; i < nelevations; i++) {

    fprintf(plot_file, "\n");
    fprintf(plot_file, "\"%g deg\"\n", elevations[i]);

    sinphi = sin(elevations[i] * DEG_TO_RAD);
    cosphi = cos(elevations[i] * DEG_TO_RAD);

    slant_range = start_range;

    for (j = 0; j < ngates; j++) {

      gnd_range = slant_range * cosphi;

      beam_ht = (radar_altitude + slant_range * sinphi +
		 slant_range * slant_range / twice_radius);

      slant_range += gate_spacing;

      if (beam_ht <= max_altitude && gnd_range < max_range) {
	fprintf(plot_file, "%g %g\n", gnd_range, beam_ht);
      } else {
	break;
      }

    } /* j */
    
  } /* i */

  /*
   * plot in cappi height lines
   */

  fprintf(plot_file, "\n");
  fprintf(plot_file, "\"Cappi planes\"\n");

  altitude = start_altitude;

  while (altitude <= max_altitude) {

    fprintf(plot_file, "move %g %g\n", 0.0, altitude);
    fprintf(plot_file, "draw %g %g\n", max_range, altitude);

    altitude += delta_altitude;

  }
  
  /*
   * close file
   */

  fclose(plot_file);

  return (0);

}
