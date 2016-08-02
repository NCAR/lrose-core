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
 *
 * uniform.c
 *
 * Uniform distribution functions.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Feb 1998
 *
 **************************************************************************/

#include <rapmath/stats.h>

/*********************
 * default seed values
 */

static int xx = 3911;
static int yy = 11383;
static int zz = 22189;

/**************************************************
 * STATS_uniform_seed()
 *
 * Seed for uniform random number generation.
 *
 * Given the seed, compute suitable starting values
 * for xx, yy and zz.
 *
 * Optional before using uniform_gen().
 *
 * Written by Mike Dixon.
 */

void STATS_uniform_seed(int seed)

{

  /*
   * set starting values for xx, yy and zz
   */

  xx = seed % 30011;

  if (xx < 14983) {
    xx += 14983;
  }

  yy = xx % 4909 + xx / 89;

  zz = xx % 9973 + xx / 97;

}

/**************************************************
 * STATS_uniform_gen()
 *
 * Generate a random number between 0 and 1.
 *
 * Optionally call uniform_seed() before using this function.
 *
 * Reference: Algorithm AS 183, B.A.Wichmann abd I.D.Hill.
 *            Applied Statistics Algorithms, Griffiths & Hill.
 *            Royal Stats Society, p238.
 *            Pub: Ellis Horwood Limited, Chichester.
 */

double STATS_uniform_gen(void)

{
  
  double sum;
  double unif;

  xx = 171 * (xx % 177) - 2  * (xx / 177);
  yy = 172 * (yy % 176) - 35 * (yy / 176);
  zz = 170 * (zz % 178) - 63 * (zz / 178);

  if (xx < 0) xx += 30269;
  if (yy < 0) yy += 30307;
  if (zz < 0) zz += 30323;

  /* 
   * on machines with integer arithmetic up to 5212632,
   * previous lines could be replaced with the following.
   *
   * xx = (xx * 171) % 30269;
   * yy = (yy * 172) % 30307;
   * zz = (zz * 170) % 30323;
   */

  sum = ((double) xx / 30269.0 +
	 (double) yy / 30307.0 +
	 (double) zz / 30323.0);

  unif = fmod(sum, 1.0);

  return (unif);

}


