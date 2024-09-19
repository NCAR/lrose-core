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
 * filetr.c
 *
 * perform the filtering
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "bearing2latlon.hh"

void filter(void)

{

  char line[BUFSIZ];
  double bearing, range;
  double lat, lon;
  FILE *ifp;

  if (!strcmp(Glob->params.input_file, "stdin")) {
    ifp = stdin;
  } else {
    if ((ifp = fopen(Glob->params.input_file, "r")) == NULL) {
      fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
      fprintf(stderr, "Cannot open input file for reading\n");
      perror(Glob->params.input_file);
    tidy_and_exit(-1);
    }
  }
  
  while (fgets(line, BUFSIZ, ifp) != NULL) {
    
    if (line[0] == '#') {
      
      fputs(line, stdout);
      
    } else {

      if (sscanf(line, "%lg%lg", &bearing, &range) != 2) {

	fputs(line, stdout);

      } else {
	
	PJGLatLonPlusRTheta(Glob->params.origin.latitude,
			    Glob->params.origin.longitude,
			    range, bearing,
			    &lat, &lon);
	
	fprintf(stdout, "%g %g\n", lat, lon);

      } /* if (sscanf(line, ... */

    } /* if (line[0] == '#') */

    fflush(stdout);

  } /* while */
  
  fflush(stdout);

  if (ifp != stdin) {
    fclose(ifp);
  }

}

