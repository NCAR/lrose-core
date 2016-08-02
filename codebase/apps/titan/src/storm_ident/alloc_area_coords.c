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
/*************************************************************************
 * alloc_area_coords.c
 *
 * memory allocation for the area_coords array
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * April 1995
 *
 *************************************************************************/

#include "storm_ident.h"

#define INIT_N_COORDS 2000
#define ALLOC_INCR 2

void alloc_area_coords(const si32 n_coords,
		       double ***coords_p,
		       si32 *n_alloc_p)
     
{


  si32 i;
  si32 n_new;
  si32 n_alloc;
  double **coords;
  double **new_coords;

  n_alloc = *n_alloc_p;
  coords = *coords_p;
  
  /*
   * initial alloc
   */

  if (coords == NULL) {
    n_alloc = INIT_N_COORDS;
    coords = (double **) umalloc2
      ((ui32) n_alloc, (ui32) 2, (ui32) sizeof(double));
  }

  /*
   * increase alloc
   */

  if (n_coords > n_alloc) {

    /*
     * new allocation
     */

    n_new = n_coords * ALLOC_INCR;
    new_coords = (double **) umalloc2
      ((ui32) n_new, (ui32) 2, (ui32) sizeof(double));

    /*
     * copy over
     */

    for (i = 0; i < n_alloc; i++) {
      new_coords[i][0] = coords[i][0];
      new_coords[i][1] = coords[i][1];
    }

    /*
     * delete old
     */

    ufree2((void **) coords);

    /*
     * set new vals
     */

    coords = new_coords;
    n_alloc = n_new;

  }

  *n_alloc_p = n_alloc;
  *coords_p = coords;
  return;

}
