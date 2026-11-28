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
#include <ctetwws/smem.h>
#include <input.h>
#include <input_data.h>
#include <input_analyze.h>
#include <uf_util.h>

static input_data_state *S=NULL;
static int num_rec = 0;

static float *az_values = NULL; /* azimuth value */
static int *az_filled = NULL; /* azimuth value */
static int nazimuth = 0;
static float *elev_values = NULL; /* elevation value */
static int *elev_filled = NULL; /* elevation value */
static int nelevation = 0;

/*----------------------------------------------------------------*/
static char *flag_print(int index)
{
    if (index == 1)
        return "good";
    else
        return " bad";
}

/*----------------------------------------------------------------
 * 
 * determine azimuth 
 * return azimuth index value..note this assumes delta is constant.
 */
static int azimuth_index(float azimuth)
{
    int i;

    i = (azimuth - params.grid_params.az0)/params.grid_params.delta_az;
    if (i >= 0 && i < nazimuth)
    {
	if (params.debug_level >= 10)
	    printf("INDEX MATCHED AT %d\n", i);
	return i;
    }
    if (i == nazimuth &&
	(azimuth - az_values[nazimuth-1] <= params.grid_params.delta_az/2.0))
    {
	if (params.debug_level >= 10)
	    printf("INDEX MATCHED AT %d\n", i-1);
	return i-1;
    }
    if (i == -1 && (az_values[0] - azimuth <= params.grid_params.delta_az/2.0))
    {
	if (params.debug_level >= 10)
	    printf("INDEX MATCHED AT %d\n", 0);
	return 0;
    }
    
    if (params.debug_level >= 10)
	printf("INDEX DID NOT MATCH for azimuth=%f\n", azimuth); 
    return -1;
}

/*----------------------------------------------------------------*/
static int elevation_index(float elev)
{
    int i;

    for (i=0; i<nelevation; ++i)
	if (elev >= elev_values[i] - params.elevation_wobble_max &&
	    elev <= elev_values[i] + params.elevation_wobble_max)
	    return i;
    return -1;
}

/*----------------------------------------------------------------*/
static int pass0_is_wanted(int i)
{
    return (S[i].time > 0 && S[i].prf > 0 && S[i].elev_index >= 0 &&
	    S[i].az_index >= 0 && S[i].range > 0);
}

/*----------------------------------------------------------------*/
/* static int pass0_initial_sweep(int i) */
/* { */
/*     return (S[i].sweep); */
/* } */

/*----------------------------------------------------------------*/
static int first_wanted(void)
{
    int i;

    for (i=0; i<num_rec; ++i)
	if (S[i].wanted == 1)
	    return i;
    return -1;
}

/*----------------------------------------------------------------*/
static void clear_index_array(int *array, int max)
{
    int i;

    for (i=0; i<max; ++i)
	array[i] = 0;
}

/*----------------------------------------------------------------*/
static int num_beams_in_sweep(int sweep)
{
    int i;
    int count=0;

    for (i=0; i<num_rec; ++i)
	if (S[i].sweep == sweep)
	    ++count;
    return count;
}

/*----------------------------------------------------------------*/
static int first_beam_in_sweep(int sweep)
{
    int i;

    for (i=0; i<num_rec; ++i)
	if (S[i].sweep == sweep)
	    return i;
    return -1;
}

/*----------------------------------------------------------------*/
static int last_beam_in_sweep(int sweep)
{
    int i, i0=-1;

    for (i=0; i<num_rec; ++i)
	if (S[i].sweep == sweep)
	    i0 = i;
    return i0;
}

/*----------------------------------------------------------------*/
/*
 * Renumber from 0 on up the sweep values.
 */
static int re_number_sweeps(void)
{
    int i, d, i0, dnew;

    i0 = first_wanted();
    if (i0 < 0)
	return 0;
    dnew = 0;
    d = S[i0].sweep;
    for (i=i0; i<num_rec; ++i)
    {
	if (S[i].wanted == 0)
	    continue;
	if (S[i].sweep == d)
	    S[i].sweep = dnew;
	else
	{
	    ++dnew;
	    d = S[i].sweep;
	    S[i].sweep = dnew;
	}
    }
    return dnew;
}

/*----------------------------------------------------------------*/
/*
 * Set sweep values to flag for the input sweep number.
 * (NO longer wanted)
 */
static void flag_sweep(int sweep)
{
    int i;
    for (i=0; i<num_rec; ++i)
	if (S[i].sweep == sweep)
	{
	    S[i].wanted = 0;
	    S[i].sweep = -1;
	}
}

/*----------------------------------------------------------------*/
static void set_volume(int sweep, int volume)
{
    int i;

    for (i=0; i<num_rec; ++i)
	if (S[i].sweep == sweep)
	    S[i].volume = volume;
}

/*----------------------------------------------------------------*/
/*
 * Build the volumes by parsing what we have, in order.
 */
static void build_volumes(int num_derived_sweeps)
{
    int i, i0, e0, e1;
    int volume;

    volume = 0;
    e0 = -1;
    for (i=0; i<=num_derived_sweeps; ++i)
    {
	i0 = first_beam_in_sweep(i);
	if (i0 < 0)
	{
	    printf("LOGIC ERROR\n");
	    continue;
	}
	e1 = S[i0].elev_index;
	if (e0 != -1 && e1 <= e0)
	    ++volume;
	set_volume(i, volume);
	e0 = e1;
    }
}

/*----------------------------------------------------------------*/
/*
 * Two sweeps adjacent with same elevation...keep only one of them
 */
static void elev_arbitrate(int sweep0, int sweep1,
			       float elev0, float elev1)
{
    int i0, i1;

    /*
     * Look at the 2 elevations, maybe they are actually different
     * NYA
     */

    /*
     * Choose a particular sweep number for this elevation
     * NYA
     */

    /*
     * Choose the one thats bigger
     */
    i0 = num_beams_in_sweep(sweep0);
    i1 = num_beams_in_sweep(sweep1);
    if (i0 < i1)
	flag_sweep(sweep0);
    else if (i0 > i1)
	flag_sweep(sweep1);
    
    /*
     * Choose the one thats earlier or later
     * NYA
     */
}

/*----------------------------------------------------------------*/
/*
 * Filter based on redundant elevations
 */
static int filter_redundant_elev(int num)
{
    int i1, i2, i;

    if (nelevation <= 1)
	return num;

    /*
     * Note indexing includes num'th below..as wanted
     */
    for (i=0; i<num; ++i)
    {	
	i1 = last_beam_in_sweep(i);
	i2 = first_beam_in_sweep(i+1);
	if (S[i1].elev_index == S[i2].elev_index &&
	    S[i1].file == S[i2].file )
	    /*
	     * Two elevs, in a row the same, in same file
	     * ONe must go.
	     */
	    elev_arbitrate(S[i1].sweep, S[i2].sweep,
			       S[i1].elevation, S[i2].elevation);
    }
    return (re_number_sweeps());
}

/*----------------------------------------------------------------*/
/*
 *Filter if too small
 */
static int filter_small_sweeps(int num)
{
    int i;
    for (i=0; i<=num; ++i)
	if (num_beams_in_sweep(i) < params.min_beams_per_scan)
	    flag_sweep(i);

    return (re_number_sweeps());
}

/*----------------------------------------------------------------*/
static int check_for_new_sweep(input_data_state *state,
			       input_data_state *this,
			       int last_filled, int this_index)
{
    int ia;

    this->sticky_azimuth = 0;

    /*
     * Check for changes in elevation or sweep number
     */
    if (state->sweep != this->sweep)
    {
	if (params.debug_level > 0)
	    printf("Transistion: sweep:%d to %d", state->sweep, this->sweep);
	return 1;
    }
    if (state->elev_index != this->elev_index)
    {
	if (params.debug_level > 0)
	    printf("Transistion: elev:%d to %d", state->elev_index,
		   this->elev_index);
	return 1;
    }

    if (state->file != this->file)
    {
	if (params.debug_level > 0)
	    printf("Transition file:%d to %d", state->file, this->file);
	return 1;
    }

    /*
     * Check out the azimuth changes
     */
    ia = this->az_index;
    if (az_filled[ia] == 0)
	/*
	 * Not a new sweep, this azimuth not yet filled in current sweep.
	 */
	return 0;

    if (last_filled == ia)
    {
	this->sticky_azimuth = 1;
	if (params.sticky_azimuths)
	    /*
	     * Filled but just filled twice with same value.
	     * and..we are allowing that..Not a new sweep.
	     */
	    return 0;
	else
	    return 1;
    }

    /*
     * It is indeed filled..return true assuming
     * wraparound has happened.
     */
    if (params.debug_level > 0)
	printf("Transition: azimuth wraparound at %d (last=%d)", ia,
	       last_filled);
    return 1;
}

/*----------------------------------------------------------------*/
static int pass0_init(int *i0, int sweep, 
		      input_data_state *state, int *last_filled)
{
    int i;

    /*
     * Go through and initialize wanted or not, and init derived sweep#
     */
    for (i=0; i<num_rec; ++i)
    {
	S[i].wanted = (pass0_is_wanted(i)?1:0);
	//S[i].sweep = -1; // don't do this, I hope!  it was set from the data.
	S[i].volume = -1;
    }

    /*
     * Find the first wanted record:
     */
    if (((*i0) = first_wanted()) < 0)
	return 0;

    /*
     * Set state based on values from this record.
     * Reset sweep number to input value in global store.
     */
    *state = S[*i0];
    S[*i0].sweep = sweep;
    az_filled[S[*i0].az_index] = 1;

    /*
     * No previous state
     */
    *last_filled = -1;
    return 1;
}

/*----------------------------------------------------------------*/
static void
pass0_process_1_record(input_data_state *this, 
		       input_data_state *state, 
		       int index, int *last_filled,
                       int *derived_sweep)
{
    if (this->wanted == 0)
	return;

    if (check_for_new_sweep(state, this, *last_filled, index) == 1)
    {
	clear_index_array(az_filled, nazimuth);
	++(*derived_sweep);
	if (params.debug_level > 0)
	    printf("  new sweep number: %d\n", *derived_sweep);
	*state = *this;   /* sweep # from this (pass0 values)*/
    }
    az_filled[this->az_index] = 1;
    this->sweep = *derived_sweep; /* set derived sweep# into this*/
    *last_filled = this->az_index;
}

/*----------------------------------------------------------------*/
/*
 * First pass analysis
 */
static int pass0(void)
{
    int last_filled, i, i0, der_sweep;
    input_data_state state;

    if (num_rec <= 0)
	return 0;

    der_sweep = 0;
    if (pass0_init(&i0, der_sweep, &state, &last_filled) == 0)
	return 0;

    /*
     * Peruse remaining records.
     */
    for (i=i0+1; i<num_rec; ++i)
	pass0_process_1_record(&S[i], &state, i, &last_filled,
			       &der_sweep);
    return der_sweep;
}

/*----------------------------------------------------------------*/
static void tests(input_data_t *uf, input_data_state *s, int index)
{

    if (index != uf->init_state.record)
	printf("WARNING...possible synchronizatiln problem\n");

    /*
     * Start with the state from the data
     */
    *s = uf->init_state;
    
    /*
     * Do uf analysis
     */
    uf_analyze_tests(uf, s);
    
    /*
     * Turn az/elev into indicies
     */
    s->az_index = azimuth_index(s->azimuth);
    s->elev_index = elevation_index(s->elevation);

}

/*----------------------------------------------------------------*/
/*
 * print the list of information from uf pass 0
 */
static void print_0(void)
{
    int i, ii;
    float delta_az, delta_elev;
    int daz, delev;

    for (ii=0,i=0; i<num_rec; ++i)
    {
	if (i < num_rec-1)
	{
	    delta_az = S[i+1].azimuth - S[i].azimuth;
	    daz = S[i+1].az_index - S[i].az_index;
	    delta_elev = S[i+1].elevation - S[i].elevation;
	    delev = S[i+1].elev_index - S[i].elev_index;
	}
	else
	{
	    delta_az = 0.0;
	    daz = 0.0;
	    delta_elev = 0.0;
	    delev = 0.0;
	}
	if (ii == 0)
	{
            printf(" rec  range   time    prf    sweep\taz/daz\tel/del\taz/daz  \telev/del\tfile\n");
	    printf(" ---  -----   ----    ---    -----\t------\t------\t--------\t--------\t----\n");
	}
	if (++ii > 30)
	    ii = 0;

	printf("%4d  %s   %s   %s     %3d\t%3d/%d\t%3d/%d\t%6.2f/%.2f\t%6.2f/%.2f\t%d\n",
	       i, flag_print(S[i].range), flag_print(S[i].time),
	       flag_print(S[i].prf), S[i].sweep,
	       S[i].az_index, daz, S[i].elev_index, delev,
	       S[i].azimuth, delta_az, S[i].elevation, delta_elev,
	       S[i].file);
    }
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
void input_analyze_init(void)
{
    int i;

    S=NULL;
    num_rec = 0;

    nazimuth = params.grid_params.naz;
    if (nazimuth > 0)
    {
	az_values = MEM_CALLOC(nazimuth, float);
	az_filled = MEM_CALLOC(nazimuth, int);
    }
    for (i=0; i<nazimuth; ++i)
    {
	az_values[i] = params.grid_params.az0 +
	    i*params.grid_params.delta_az;
	az_filled[i] = 0;
    }
    nelevation = params.grid_params.nelev;
    if (nelevation > 0)
    {
	elev_values = MEM_CALLOC(nelevation, float);
	elev_filled = MEM_CALLOC(nelevation, int);
    }
    
    if (params.use_vertical_levels)
    {
        for (i=0; i<nelevation; ++i)
	{
	    elev_values[i] = params._vertical_levels[i];
	    elev_filled[i] = 0;
	}
    }
    else
    {
	for (i=0; i<nelevation; ++i)
	{
	    elev_values[i] = params.grid_params.elev0 +
	      i*params.grid_params.delta_elev;
	    elev_filled[i] = 0;
	}
    }
    uf_analyze_init();
}

/*----------------------------------------------------------------*/
void input_analyze_load_next(input_data_t *uf)
{
/*     input_data_state *s; */

    /*
     * Create new record to append results of the tests
     */
    if (S == NULL)
    {
	S = MEM_CALLOC(1, input_data_state);
	num_rec = 0;
    }
    else
	S = MEM_GROW(S, num_rec+1, input_data_state);

    /*
     * Perform tests on the data
     */
    tests(uf, &S[num_rec], num_rec);
    ++num_rec;
}

/*----------------------------------------------------------------*/
/*
 * Build the tables internally
 */
void input_analyze_build_tables(void)
{
    int num_sweeps;

    /*
     * Derive the sweeps from the current initial data
     */
    num_sweeps = pass0();
    printf("AFTER PASS0, %d sweeps\n", num_sweeps);
    if (params.analyze_feedback == ANALYZE_FEEDBACK_MEGA ||
	params.analyze_feedback == ANALYZE_FEEDBACK_INIT_FINAL)
	print_0();

    if (params.filter_min_beams)
    {
	/*
	 * Remove sweeps that have too few elements
	 */
	num_sweeps = filter_small_sweeps(num_sweeps);
	printf("AFTER FILTERING SMALL SWEEPS, %d sweeps min_size=%d\n",
	       num_sweeps, params.min_beams_per_scan);
	if (params.analyze_feedback == ANALYZE_FEEDBACK_MEGA)
	    input_analyze_print("SMALL_SWEEPS_GONE");
    }

    if (params.filter_redundant_elev_based_on_size)
    {
	/*
	 * If there is more than one elevation, and 2 elevations
	 * are adjacent sweeps that are the same elevation, pick
	 * the right one using simple rules.
	 */
	num_sweeps = filter_redundant_elev(num_sweeps);
	printf("AFTER FILTERING REDUNDANT ELEVATIONS, %d sweeps\n",
	       num_sweeps);
	if (params.analyze_feedback == ANALYZE_FEEDBACK_MEGA)
	    input_analyze_print("REDUNDANT ELEVATIONS_GONE");
    }

    /*
     * Finally determine the volumes
     */
    build_volumes(num_sweeps);
}

/*----------------------------------------------------------------*/
/*
 * Print out the results
 */
void input_analyze_print(char *title)
{
    int i;
/*     float delta_az, delta_elev; */
/*     int daz, delev; */

    if (title != NULL)
	printf("\n%s\n", title);
    printf("beam   file   az    elev   derived  derived\n");
    printf("index  index  index index  sweep    vol\n"); 
    printf("------  ----  ----  -----  -------  ----\n");
    for (i=0; i<num_rec; ++i)
    {
	if (S[i].wanted == 0)
	    continue;
	if (S[i].sticky_azimuth == 0)
	    printf("%5d   %4d  %4d  %4d   %4d     %4d\n",
		   i, S[i].file, S[i].az_index, S[i].elev_index,
		   S[i].sweep, S[i].volume);
	else
	    printf("%5d   %4d  %4d  %4d   %4d     %4d Sticky\n",
		   i, S[i].file, S[i].az_index, S[i].elev_index,
		   S[i].sweep, S[i].volume);
    }
}
/*----------------------------------------------------------------*/
/*
 * return next wanted data, NULL for none.
 */
input_data_t *input_analyze_next_wanted(void)
{
    input_data_t *u;
    int i;

    for (;;)
    {
	/* 
	 * Get some data
	 */
	u = input_data_get_next();
	if (u == NULL)
	    return u;

	/*
	 * Use its record number to see if its wanted
	 */
	i = u->init_state.record;
	if (S[i].wanted == 1)
	{
	    /*
	     * Yes...Fill in the derived values and return.
	     */
	    u->analyzed_state = S[i];
	    return u;
	}
	else
	    /*
	     *NO try again
	     */
	    input_data_free(u);
    }
    return NULL;
}

/*----------------------------------------------------------------*/
int input_analyze_az_index(input_data_t *uf)
{
    return uf->analyzed_state.az_index;
}

/*----------------------------------------------------------------*/
int input_analyze_elev_index(input_data_t *uf)
{
    return uf->analyzed_state.elev_index;
}

/*----------------------------------------------------------------*/
int input_analyze_num_records(void)
{
    int i, i0, count;

    i0 = first_wanted();
    if (i0 < 0)
	return 0;

    for (count=0, i=i0; i<num_rec; ++i)
    {
	if (S[i].wanted == 1)
	    ++count;
    }
    return count;
}
