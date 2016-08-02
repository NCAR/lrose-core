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
 * rtiming.c
 *
 * computes timing of radar volume-scan cycle
 */

#include "rtiming.h"

int main(argc, argv)
     int argc;
     char **argv;

{

  char line[256];
  int elev_used;
  long i, j, k, m;
  long nsweeps;
  double sweep_time;
  double cycle;
  double age, mean_age;
  sweep_t *sweep;
  double *elev;
  FILE *input;
  
  /*
   * check number of args
   */

  if (argc != 2) {

    printf("usage - rtiming input-file\n");
    exit (-1);

  }

  /*
   * open input file
   */

  
  if ((input = fopen(argv[1], "r")) == NULL) {

    printf("ERROR - opening input file\n");
    perror(argv[1]);
    printf("usage - rtiming input-file\n");

  }

  /*
   * read input file
   */

  if (fgets(line, 256, input) == NULL) {
    printf("ERROR reading nsweeps, cycle\n");
    perror(line);
    exit (-1);
  }
  
  if (sscanf(line, "%ld %lg", &nsweeps, &cycle) != 2) {

    printf("ERROR reading nsweeps, cycle\n");
    perror(line);
    exit (-1);

  }

  printf("nsweeps : %ld\n", nsweeps);
  printf("cycle : %g\n\n", cycle);

  sweep = (sweep_t *) umalloc
    (nsweeps * sizeof(sweep_t));

  elev = (double *) umalloc
    (nsweeps * sizeof(double));

  for (i = 0; i < nsweeps; i++) {

    if (fgets(line, 256, input) == NULL) {
      break;
    }
  
    if (sscanf(line, "%ld %lg %lg %lg %lg",
	       &sweep[i].num,
	       &sweep[i].elev,
	       &sweep[i].time,
	       &sweep[i].duration,
	       &sweep[i].vol_frac) != 5) {

      printf("ERROR reading sweep %ld\n", i);
      perror(line);
      exit (-1);
      
    }

    printf("num, elev, time, vol_frac : "
	   "%3ld %10.1f %10.2f %10.2f %10.3f\n",
	   sweep[i].num,
	   sweep[i].elev,
	   sweep[i].time,
	   sweep[i].duration,
	   sweep[i].vol_frac);

  } /* i */

  printf("\n");

  /*
   * compute the mean age of data at each sweep
   */

  for (i = 0; i < nsweeps; i++) {

    mean_age = 0.0;
    sweep_time = sweep[i].time;

    for (j = 0; j < nsweeps; j++)
      elev[j] = -200.0;

    for (j = i; j > i - nsweeps; j--) {

      if (j >= 0) {
	k = j;
	age = sweep_time - sweep[k].time;
      } else {
	k = j + nsweeps;
	age = sweep_time - sweep[k].time + cycle;
      }
      
      /*
       * check if elevation has already been used
       */

      elev_used = FALSE;

      for (m = 0; m < nsweeps; m++) {

	if (elev[m] < -180.0) {
	  elev[m] = sweep[k].elev;
	  break;
	}
	
	if (elev[m] == sweep[k].elev) {
	  elev_used = TRUE;
	  break;
	}

      } /* m */

      if (!elev_used) {

	mean_age += age * sweep[k].vol_frac;

      } /* if (!elev_used) */

    } /* j */

    mean_age += sweep[i].duration / 2.0;

    printf("sweep, time, mean_age : %3ld %10.2f %10.2f\n",
	   sweep[i].num, sweep[i].time, mean_age);

  } /* i */

  printf("\n");

  return (0);

}




