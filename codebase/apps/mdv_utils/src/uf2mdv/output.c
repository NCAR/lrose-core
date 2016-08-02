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
 * output.c
 *
 * Write output files.
 * Defaults contained in paramdef.output
 *
 * Dave Albo, RAP, NCAR, Boulder CO, January, 1998
 *
 *********************************************************************/

#include <signal.h>
#include <output.h>
#include <output_store.h>
#include <uf_mdv.h>
#include <toolsa/umisc.h>

static int is_first = 1;
static int num_volumes = 0;

/*----------------------------------------------------------------*/
void OUTPUT_init(int nazimuth, int nelevation)
{
    /* Initialize the objects */
    output_store_init(nazimuth, nelevation);

    /*
     * Set appropriate values into the internal state to "nothing 
     * yet".
     */
    output_store_set_sweep_empty(1);
    is_first = 1;
    num_volumes = 0;
}

/*----------------------------------------------------------------*/
/*
 * Finished with one sweep/sweep/ppi
 */
void OUTPUT_finish_1_sweep(void)
{
    int nbeam, naz, *az;

    /*
     * See how many beams in the sweep
     */
    nbeam = output_store_num_beams(); 
    if (params.debug_level >= 1)
	printf("\tfinish...%d rays in buffer\n", nbeam);
    if (nbeam <= 0)
    {
	if (params.debug_level >= 4)
	    printf("EMPTY sweep NO WRITE\n");
	return;
    }

    /*
     * Finish the sweep
     */
    az = output_store_get_az_indicies(&naz);
    UF_MDV_finish_1_sweep(az, naz);

    /*
     * Empty out the sweep stuff in the store
     */
    output_store_set_sweep_empty(0);
}

/*----------------------------------------------------------------*/
/*
 * Finished with one volume
 */
void OUTPUT_finish_1_volume(void)
{
    int *elev, nelev;

    elev = output_store_get_elev_indicies(&nelev);
    UF_MDV_finish_1_volume(elev, nelev);
    ++num_volumes;

    /*
     * Empty out the volume in the store
     */
    output_store_set_sweep_empty(1);

    is_first = 1;
}

/*----------------------------------------------------------------*/
void OUTPUT_process(input_data_t *uf)
{
    if (is_first == 1)
    {
	/*
	 * Init the mdv data with this input
	 */
	UF_MDV_init_master_header(uf);
	UF_MDV_init_field_headers(uf);
    }
  
    /*
     * Now we have some data to process...do so.
     */
    if (UF_MDV_process_one_buffer(uf) == 1)
    {
        /*
 	 * Indicate indicies now filled
	 */
        output_store_set_indicies(uf);
	is_first = 0;
    }
}

/*----------------------------------------------------------------*/
void OUTPUT_finish(void)
{
    /*
     * Finish outstanding sweeps in final volume
     */
    if (is_first != 1)
    {
	OUTPUT_finish_1_sweep();
	OUTPUT_finish_1_volume();
    }
    if (params.debug_level >= 1)
	fprintf(stdout,"output finished\n");
}

/*----------------------------------------------------------------*/
int OUTPUT_num_records(void)
{
    return (output_store_total_beams());
}

/*----------------------------------------------------------------*/
int OUTPUT_num_volumes(void)
{
    return (num_volumes);
}
