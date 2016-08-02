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
/****************************************************************************
 * vil.c
 *
 * Computes VIL from dbz and ht layer information
 *
 * Step 1: call vil_init()
 * Step 2: call vil_add() for each layer
 * Step 3: call vil_compute() to get result.
 * 
 *********************************************************************/

#include <physics/vil.h>
#include <math.h>

static double sum_vil;

/*****************************
 * vil_init()
 *
 * Initialize VIL computation.
 */

void vil_init(void)

{
  sum_vil = 0.0;
}

void vil_init_local( double* sum_vil_local )
{
  *sum_vil_local = 0.0;
}

/*************************************************
 * vil_add()
 *
 * Add vil for a layer, given dbz and dheight (km)
 */

void vil_add(double dbz, double dheight)

{

  double dvil;
  
  if (dbz > VIL_THRESHOLD) {
    dvil = pow(10.0, dbz * FOURBYSEVENTY);
    sum_vil += dvil * dheight * 1000.0;
  }
  
}

void vil_add_local(double* sum_vil_local, double dbz, double dheight)

{

  double dvil;

  if (dbz > VIL_THRESHOLD) {
    dvil = pow(10.0, dbz * FOURBYSEVENTY);
    *sum_vil_local += dvil * dheight * 1000.0;
  }

}


/********************************
 * vil_compute()
 *
 * return the computed VIL value.
 */

double vil_compute(void)

{
  return (sum_vil * VILCONST);
}

double vil_compute_local(double* sum_vil_local)

{
  return (*sum_vil_local * VILCONST);
}

