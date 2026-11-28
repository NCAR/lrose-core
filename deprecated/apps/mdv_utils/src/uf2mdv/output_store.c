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
#include <stdio.h>
#include <output_store.h>
#include <ctetwws/smem.h>

static int total_beams = 0;
static int num_beams = 0;    /* number of beams in output buffer */
static int *az_status = NULL; /* 1 if azimuth filled */
static int nazimuth = 0;
static int *elev_status = NULL; /* 1 if elevation filled */
static int nelevation = 0;

/*----------------------------------------------------------------*/
/*
 * Initialize OUTPUT
 */
void output_store_init(int naz, int nelev)
{
    int i;

    /* Make list of azimuth, elevation status values */
    nazimuth = naz;
    if (naz > 0)
    {
	az_status = MEM_CALLOC(nazimuth, int);
	for (i=0; i<nazimuth; ++i)
	    az_status[i] = 0;
    }
    
    nelevation = nelev;
    if (nelev > 0)
    {
	elev_status = MEM_CALLOC(nelevation, int);
	for (i=0; i<nelevation; ++i)
	    elev_status[i] = 0;
    }

    num_beams = 0;
    total_beams = 0;
}

/*----------------------------------------------------------------*/
/*
 * Called whenever a sweep (ppi) has just finished, and need to reinit
 * output internal state.
 */
void output_store_set_sweep_empty(int clear_elev)
{
    int i;

    num_beams = 0;  /* no beams in output sweep */

    /* All azimuths have status 0 */
    for (i=0; i< nazimuth; ++i)
	az_status[i] = 0;

    if (clear_elev == 1)
    {
	/* All elevations have status 0 */
	for (i=0; i< nelevation; ++i)
	    elev_status[i] = 0;
    }
}

/*----------------------------------------------------------------*/
/*
 * A beam just filled in output given by indicies.
 */
void output_store_set_indicies(input_data_t *uf)
{
    az_status[uf->analyzed_state.az_index] = 1;
    elev_status[uf->analyzed_state.elev_index] = 1;
    ++num_beams;
    ++total_beams;
}

/*----------------------------------------------------------------*/
int output_store_num_beams(void) 
{
    return num_beams;
}

/*----------------------------------------------------------------*/
int *output_store_get_az_indicies(int *naz)
{
    *naz = nazimuth;
    return az_status;
}

/*----------------------------------------------------------------*/
int *output_store_get_elev_indicies(int *nel)
{
    *nel = nelevation;
    return elev_status;
}

/*----------------------------------------------------------------*/
int output_store_total_beams(void)
{
    return total_beams;
}
