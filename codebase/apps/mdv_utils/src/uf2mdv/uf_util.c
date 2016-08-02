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
#include <input.h>
#include <ctetwws/ut.h>
#include <uf_util.h>
#include <uf_data_format.h>

static time_t min_time, max_time;
static float min_gate, max_gate;

/*----------------------------------------------------------------*/
static
void field_header_dump(Field_header *f, char *field_name, int big)
{
    if (big == 1)
	printf("%s prf:%.3f gates:%.3f to %.3f spacing:%d num:%d scale:%d",
	       field_name, PRF(f->pulse_rep),
	       RNG0(f), RNG1(f), f->vol_space, f->num_samples,
	       f->scale_factor);
    else
	printf("%s gates:[%.3f,%.3f] spc:%d scl:%d",
	       field_name, RNG0(f), RNG1(f), f->vol_space,
	       f->scale_factor);
}

/*----------------------------------------------------------------*/
static
void mandatory_header_dump(Mandatory_header_block *m, int one_line)
{
    printf("vol:%d ray:%d rayr:%d sweep:%d t:%d,%d,%d,%d,%d,%d az:%.3f el:%.3f fxd:%.3f  mode:%d",
	   m->volume_scan,
	   m->ray_number,
	   m->ray_record,
	   m->sweep_number,
	   m->year,
	   m->month,
	   m->day,
	   m->hour,
	   m->minute,
	   m->second,
	   m->azimuth/64.0,
	   m->elevation/64.0,
	   m->fixed_angle/64.0,
	   m->sweep_mode);
}

/*----------------------------------------------------------------*/
static void ufdump_all_data(char *uf,
			    Mandatory_header_block *m,
			    Field_header *f, char *name)
{
    signed short *spt;        /* pointer to short */
    int i; /*, j, j0, j1;*/
    float range, value, min, delta;

    spt = (signed short *)(POSITION(uf, f->position));
    min = (float)(f->rnge_frst_gate*1000 + f->adjustment);
    delta = (float)(f->vol_space);
    printf("%s:\n", name);
    for (i=0; i<f->num_samples; ++i, ++spt)
    {
	range = min + (float)i*delta;
	if (*spt == m->missing)
	{
	    value = F_FLAG;
	}
	else
	    value = (float)(*spt)/(float)(f->scale_factor);
	printf("%f...%f\n", range, value);
    }
    return;
}


/*----------------------------------------------------------------*/
static int mandatory_block_test(Mandatory_header_block *m,
				int *time)
{
    time_t header_time;
    int y;

    //if (m->sweep_mode != PPI_MODE && m->sweep_mode != 8)
    //printf("DONT ABORT..bad mode bad mode %d\n", m->sweep_mode);

    /*
     * copy the header block time into time6 
     */

    /* A Y2K kludge for the year. It assumes this program will
       be dead in 50 years.*/
       
#ifdef NOTNOW
    if (m->year < 1900)
	{
	   // Y2K kludge
	   if( m->year < 50)
	       y = m->year + 2000;
	   else 
	        y = m->year + 1900;
	}
    else
#endif
	y = (int)m->year;
    header_time = UT_gmt_to_unix(y, (int)m->month, (int)m->day,
				 (int)m->hour, (int)m->minute,
				 (int)m->second);

    /*
     * check if time is in range
     */
    if (header_time < min_time || header_time > max_time)
	*time = 0; 
    else
	*time = 1;
    return 1;
}

/*----------------------------------------------------------------*/
static void field_block_test(char *c, char *databuf, int *prf, int *range)
{
    signed short *spt;
    Field_header *f;
    float prf_value, uf_min, uf_max;

    spt = (signed short *)(c + 2);
    f = (Field_header *)(POSITION(databuf, *spt));
    prf_value = PRF(f->pulse_rep);
    if (prf_value > params.max_prf)
	*prf = 0;
    if (prf_value < params.min_prf)
        *prf = 0;
   
    /*
     * the total range of gates in incoming uf data
     */
    uf_min = (float)( f->rnge_frst_gate*1000 + f->adjustment );
    uf_max = (float)( uf_min + (f->vol_space*(f->num_samples-1)) );

    if (uf_max < min_gate || uf_min >= max_gate)
	*range = 0;
}

/*----------------------------------------------------------------*/
void ufdump(input_data_t *uf, int one_line)
{
    Data_header *d;
    char *c, field_name[3];
    int i; /*, rvalue;*/
    signed short *spt;
    Field_header *f;
    Mandatory_header_block *m;

    m = (Mandatory_header_block *)uf->data;
    printf("file:%3d fswp:%3d ", uf->init_state.file,
	   uf->init_state.sweep);
    mandatory_header_dump(m, one_line);
    d = (Data_header *)(POSITION(uf->data, m->data_header));
    c = (char *)d + sizeof(Data_header);
    for (i=0; i<d->num_flds_rec; ++i, c+=4)
    {
	memcpy(field_name, c, 2);
	field_name[2] = 0;
	spt = (signed short *)(c + 2);
	f = (Field_header *)(POSITION(uf->data, *spt));
	if (one_line == 1)
	{
	    printf("\t");
	    field_header_dump(f, field_name, 0);
	}
	else
	{
	    printf("\n");
	    field_header_dump(f, field_name, 1);
	}
    }
    printf("\n");
}

/*----------------------------------------------------------------*/
void ufdump_all(input_data_t *uf)
{
    Data_header *d;
    char *c, field_name[3];
    int i; /*rvalue, index;*/
    signed short *spt;
    Field_header *f;
    Mandatory_header_block *m;


    ufdump(uf, 0);
    
    m = (Mandatory_header_block *)uf->data;
    d = (Data_header *)(POSITION(uf->data, m->data_header));
    c = (char *)d + sizeof(Data_header);
    for (i=0; i<d->num_flds_rec; ++i, c+=4)
    {
	memcpy(field_name, c, 2);
	field_name[2] = 0;
	spt = (signed short *)(c + 2);
	f = (Field_header *)(POSITION(uf->data, *spt));

	/*
	 * we only want fields that are known
	 */
	/*if (FIELD_is_uf_name(field_name, &index))*/
	ufdump_all_data(uf->data, m, f, field_name);
    }
}

/*----------------------------------------------------------------*/
void uf_get_header_values(char *uf, float *az, float *elev,
			  int *sweep)
{
    Mandatory_header_block *m;
    
    m = (Mandatory_header_block *)uf;
    *az = m->azimuth/64.0;
    while ((*az) >= 360.0)
	(*az) -= 360.0;
    while ((*az) < 0.0)
	(*az) += 360.0;
    *elev = m->elevation/64.0;
    *sweep = m->sweep_number;
}

/*----------------------------------------------------------------*/
void uf_analyze_tests(input_data_t *uf, input_data_state *s)
{
    Mandatory_header_block *m;
    Data_header *d;
    char *c;
    int i;

    /*
     * check for accordance with parameters in Mandatory header block
     */
    m = (Mandatory_header_block *)uf->data;
    mandatory_block_test(m, &s->time);
   
    /*
     * Check the field header blocks too
     */
    d = (Data_header *)(POSITION(uf->data, m->data_header));
    c = (char *)d + sizeof(Data_header);
    for (s->prf=1, s->range=1, i=0; i<d->num_flds_rec; ++i, c+=4) 
	field_block_test(c, uf->data, &s->prf, &s->range);

    /*
     * PUll out values from data
     */
    uf_get_header_values(uf->data, &s->azimuth, &s->elevation,
			 &s->sweep);
}

/*----------------------------------------------------------------*/
void uf_analyze_init(void)
{
    if (params.start_time.len == 6)
	min_time = UT_gmt_to_unix(params.start_time.val[0], 
				    params.start_time.val[1],
				    params.start_time.val[2],
				    params.start_time.val[3],
				    params.start_time.val[4],
				    params.start_time.val[5]);
    else
    {
	printf("WARNING..start time param doesn't have 6 values..ignore\n");
	min_time = 0;
    }

    if (params.stop_time.len == 6)
	max_time = UT_gmt_to_unix(params.stop_time.val[0], 
				    params.stop_time.val[1],
				    params.stop_time.val[2],
				    params.stop_time.val[3],
				    params.stop_time.val[4],
				    params.stop_time.val[5]);
    else
    {
	printf("WARNING..stop time param doesn't have 6 values..ignore\n");
	min_time = 0;
    }
    min_gate = params.grid_params.gt0;
    max_gate = params.grid_params.gt0 +
	params.grid_params.ngt*params.grid_params.delta_gt;

}
