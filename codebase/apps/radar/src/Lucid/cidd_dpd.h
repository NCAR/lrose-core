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
 * CCIDD_DPD.H : Definitions and control struictures/variables for "extra"
 *    features for the CIDD display
 *
 */

#ifndef CIDD_DPD_H
#define CIDD_DPD_H

typedef struct {
    MetRecord *u_wind;
    MetRecord *v_wind;
    MetRecord *w_wind;
    MetRecord *temper;
    MetRecord *turbulence;
    MetRecord *icing;
} route_t;

typedef struct {
    MetRecord *u_wind;
    MetRecord *v_wind;
    MetRecord *w_wind;
} winds_t;


typedef struct {
    int num_layer_grids;
    int num_contour_grids;
    int num_wind_grids;

    MetRecord *key_grid;
    MetRecord *terrain;
    MetRecord **layer_grid;
    MetRecord **contour_grid;

    winds_t **wind_grid;

    route_t *route;

    // Place Holder for TDRP Display Page Description  Params 
} dpd_t;

#endif
