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
#ifdef __cplusplus
 extern "C" {
#endif

/************************************************************************
 * vil.h : Header file for the vil module of the physics library
 *
 ************************************************************************/

#ifndef VIL_WAS_INCLUDED
#define VIL_WAS_INCLUDED

/*
 * The vil computation used here is equivalent to using the
 * following z-m relationship, where m is the vapor density
 * in g/m3:
 *
 *   Z = 20465 * m ^ 1.75
 */

#define VILCONST 3.44e-6
#define FOURBYSEVENTY  0.057142857
#define VIL_THRESHOLD 20.0

/**********************
 * MACRO implementation
 */
   
/*****************************
 * VIL_INIT
 *
 * Initialize VIL computation.
 */
   
#define VIL_INIT(vil) (vil = 0.0)
   
/*************************************************
 * VIL_ADD()
 *
 * Add to vil for a layer, given dbz and dheight (km)
 */
   
#define VIL_ADD(vil, dbz, dheight) if ((dbz) > VIL_THRESHOLD) vil += (pow(10.0, (dbz) * FOURBYSEVENTY) * (dheight) * 1000.0)

/********************************
 * VIL_COMPUTE()
 *
 * Compute the final value
 */

#define VIL_COMPUTE(vil) (vil *= VILCONST)

/****************************************************************************
 *
 * Computes VIL from dbz and ht layer information
 *
 * Step 1: call vil_init()
 * Step 2: call vil_add() for each layer
 * Step 3: call vil_compute() to get result.
 * 
 *********************************************************************/

/*****************************
 * vil_init()
 *
 * Initialize VIL computation.
 */

extern void vil_init(void);
extern void vil_init_local(double* sum_vil_local);

/*************************************************
 * vil_add()
 *
 * Add vil for a layer, given dbz and dheight (km)
 */

extern void vil_add(double dbz, double dheight);
extern void vil_add_local(double* sum_vil_local, double dbz, double dheight);

/********************************
 * vil_compute()
 *
 * return the computed VIL value.
 */

extern double vil_compute(void);
extern double vil_compute_local(double* sum_vil_local);

#endif
#ifdef __cplusplus
}
#endif
