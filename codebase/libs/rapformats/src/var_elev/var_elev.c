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

/************************************************************************

Module:	var_elev.c

Author:	Dave ALbo

Date:	Thu Jun 11 17:23:31 1998

Description: 
             Variable elevation data routines

************************************************************************/

/* System include files / Local include files */
#include <dataport/bigend.h>
#include <rapformats/var_elev.h>
#include <toolsa/mem.h>

/* Constant definitions / Macro definitions / Type definitions */

/*
 * var_elev is a representation that allows each azimuth in a scan 
 * to have its own elevation angle.  Hence the representation of
 * this information is as number of azimuths followed by an array
 * of said elevations, one for each azimuth.
 * It is written as a "chunk" (see mdv) as an ui32 followed by
 * nazimuth fl32 values.
 */
typedef struct
{
    int nazimuth;
    float *elevations;
} var_elev_t;


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
/*
 * Print the var_elev stuff passed in as an MDV chunk
 */
void VAR_ELEV_print(FILE *outfile, void *data, int size)
{
    fl32 *f;
    ui32 *naz;
    int len, i;

    fprintf(outfile, "\n");
    fprintf(outfile, "variable elevation chunk data\n");

    len = (size - sizeof(ui32))/sizeof(fl32);
    naz = (ui32 *)data;
    if (len*sizeof(fl32)+sizeof(ui32) != size)
	fprintf(stderr, "WARNING...size input to VAR_ELEV_print bad %d\n",
		size);
    if (*naz != len)
    {
	fprintf(stderr, "WARNING...size input to VAR_ELEV_print seems bad\n");
	fprintf(stderr, "          input length:%d   data length:%d\n",
		len, *naz);
    }

    f = (fl32 *) data + sizeof(fl32);
    for (i=0; i<*naz; ++i)
	fprintf(outfile, "elevation[%d] = %6.2f\n", i, f[i]);
    return;
}

/*----------------------------------------------------------------*/
/*
 * convert input var_elev data from BE into local format.
 */
void VAR_ELEV_variable_elev_from_BE(void *data, int size)
{
    BE_to_array_32(data, size);
}

/*----------------------------------------------------------------*/
/*
 * convert input var_elev data from local into BE.
 */
void VAR_ELEV_variable_elev_to_BE(void *data, int size)
{
    BE_from_array_32(data, size);
}

/*----------------------------------------------------------------*/
/*
 * Return var_elev record size in bytes associated with input nazimuth
 */
int VAR_ELEV_record_size(int nazimuth)
{
    if (nazimuth <= 0)
	return 0;
    return nazimuth*sizeof(fl32) + sizeof(ui32);
}

/*----------------------------------------------------------------*/
/*
 * Build and return the var_elev data associated with the inputs
 * Return the length of the data in len
 *
 * The user must free the space by calling VAR_ELEV_destroy
 */
void *VAR_ELEV_build(float *elevations, int nazimuth, int *len)
{
    ui08 *ve;
    fl32 *elev;
    ui32 *naz;
    int i;

    if (nazimuth <= 0 || elevations == NULL)
    {
	fprintf(stderr, "ERROR on input to VAR_ELEV_build\n");
	*len = 0;
	return NULL;
    }

    *len = VAR_ELEV_record_size(nazimuth);
    ve = (ui08 *)ucalloc(*len, sizeof(ui08));
    naz = (ui32 *)ve;
    elev = (fl32 *)(naz + 1);
    *naz = nazimuth;
    for (i=0; i<nazimuth; ++i)
	elev[i] = (fl32)elevations[i];
    return (void *)ve;
}

/*----------------------------------------------------------------*/
/*
 * Free chunk data allocated by call to VAR_ELEV_build
 */
void VAR_ELEV_destroy(void **var_elev)
{
    if (var_elev == NULL)
	return;
    if (*var_elev == NULL)
	return;
    ufree(*var_elev);
}

/*----------------------------------------------------------------*/
/*
 * Return the number of elevations in the input var_elev data
 */
int VAR_ELEV_num_elevations(void *var_elev)
{
    ui32 *i;
    int n;

    i = (ui32 *)var_elev;
    n = (int)*i;
    return n;
}

/*----------------------------------------------------------------*/
/*
 * Return the elevation associated with the input azimuth index
 * as found in the var_elev data passed in.
 */
float VAR_ELEV_get_elevation(void *var_elev, int index)
{
    fl32 *f;
    ui32 *i;

    i = (ui32 *)var_elev;
    if (index < 0 || index >= *i)
    {
	fprintf(stderr,
		"ERROR index to VAR_ELEV_get_elevation %d, range [0,%d]\n",
		index, *i-1);
	return VAR_ELEV_BAD_ELEV;
    }

    f = (fl32 *)(i+1);
    return (float)f[index];
}
