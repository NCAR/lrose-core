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
#include <output.h>
#include <uf_mdv.h>
#include <uf_data_format.h>
#include <toolsa/pmu.h>
#include <sim/mdv_sim.h>
#include <ctetwws/smem.h>
#include <ctetwws/ut.h>

static MDV_master_header_t mmh;            	/* mdv master header   */
static MDV_field_header_t  *mfh=NULL; 		/* mdv field headers   */
static MDV_vlevel_header_t *mvh=NULL; 		/* mdv vlevel headers  */
static unsigned char **buf=NULL;
/* static int nbuf = 0; */
static int last_elev_index=-1;
static char **unwanted=NULL;
static int n_unwanted = 0; 


/*----------------------------------------------------------------*/
static void monitor_unwanted(char *field_name)
{
    int i;
    for (i=0; i<n_unwanted; ++i)
	if (strncmp(unwanted[i], field_name, 2) == 0)
	    return;
    
    if (n_unwanted == 0)
	unwanted = MEM_CALLOC(1, char *);
    else
	unwanted = MEM_GROW(unwanted, n_unwanted+1, char *);
    unwanted[n_unwanted] = strdup(field_name);
    printf("UNWANTED NAME???   %s\n", unwanted[n_unwanted]);
    ++n_unwanted;
}

/*----------------------------------------------------------------*/
static void init_vlevel_header(MDV_vlevel_header_t *h,
			       MDV_field_header_t *f)
{
    int vlevel;

    /* fill in fortran record size -- (record_len1=record_len2) */
    h->record_len1 = sizeof(MDV_vlevel_header_t)-2*sizeof(h->record_len2);

    h->struct_id = MDV_VLEVEL_HEAD_MAGIC_COOKIE;

    for (vlevel = 0; vlevel < f->nz; vlevel++)
    {
        h->vlevel_type[vlevel] = MDV_VERT_TYPE_ELEV;
	h->vlevel_params[vlevel] = params._vertical_levels[vlevel];
    }
 
    /* fill in fortran record size -- (record_len1=record_len2) */
    mvh->record_len2 = mvh->record_len1;
}  

/*----------------------------------------------------------------*/
static void init_field_header(MDV_field_header_t *h, int ifield,
			      Field_header *f,
			      Mandatory_header_block *m,
			      Data_header *d,
			      char *name,
			      double scale, double bias)
{

    /* fill in fortran record size -- (record_len1=record_len2) */
    h->record_len1 = sizeof(MDV_field_header_t)-2*sizeof(h->record_len2);

    /* fill in magic cookie value */
    h->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;

    /* FIELD_CODE */
    /* fill this in later h->field_code=field_code */

    /* user time1,2,3,4 */
#ifndef __linux
    h->user_time1 = 0;
    h->user_time2 = 0;
    h->user_time3 = 0;
    h->user_time4 = 0;

    /* for sim files, there is no forecast */
    h->forecast_delta    = 0;
    h->forecast_time     = mmh.time_centroid;
#endif
 
    /* Copy what's in master header */
    h->nx = mmh.max_nx;
    h->ny = mmh.max_ny;
    h->nz = mmh.max_nz;

    /* Projection type.*/
    h->proj_type = 9;


    /* encoding type.  MDV_INT8 is unsigned ints. All gint data
     * read in as MDV_INT8.  Output may either be MDV_PLANE_RLE8 or MDV_INT8. 
     * MDV_PLANE_RLE8 is MDV plane by plane compression format
     */
    h->encoding_type = MDV_INT8;

    /*
     * data element size in bytes --
     *  MDV_INT8 is the only one implemented right now
     */
    h->data_element_nbytes = MDV_data_element_size(MDV_INT8);

    /* volume size -- size of all the field data */
    if (h->encoding_type == MDV_INT8)
    {
	h->volume_size = (h->nx)*(h->ny)*(h->nz)*h->data_element_nbytes;
    }
    else
	fprintf(stdout,"\nDon't know what volume size is. Fill in later?");


    /* Field data offset, assume no chunk data, all field volumes same size.
     * If data written out encoded then this will need to be updated later.
     */
    h->field_data_offset = sizeof(MDV_master_header_t) + 
	mmh.n_fields * 
	(sizeof(MDV_field_header_t) + 
	 mmh.vlevel_included*sizeof(MDV_vlevel_header_t)) +
	ifield*h->volume_size +
	(2*ifield+1)*sizeof(h->record_len2);

    /* projection origin information....for the simulation let it be where the
     * "sensor" is.
     */
    h->proj_origin_lat = mmh.sensor_lat;
    h->proj_origin_lon = mmh.sensor_lon;

    /* projection parameters - not needed for simulation files (I hope) */
    /* h->proj_param[MDV_MAX_VLEVELS] = ; */

    /* Vertical reference point -- put in origin altitude */
    h->vert_reference = mmh.sensor_alt;

    /*
     * grid information in meters or degrees, meters converted to km.
     */
    h->grid_dx   = params.grid_params.delta_gt*METERS2KM;
    h->grid_dy   = params.grid_params.delta_az;
    h->grid_dz   = params.grid_params.delta_elev;
    h->grid_minx = params.grid_params.gt0*METERS2KM;
    h->grid_miny = params.grid_params.az0;
    h->grid_minz = params.grid_params.elev0;

    /* scale and bias..comes from the data..*/
    h->scale = scale;
    h->bias  = bias;
    h->field_code = 0;
    h->bad_data_value     = (float)MY_MDV_FLAG_VALUE;
    h->missing_data_value = (float)MY_MDV_MISSING_VALUE;
   
    /* no rotation */
    h->proj_rotation = 0.0;

    /* field name information */
    strncpy((char *)h->field_name_long, name, MDV_LONG_FIELD_LEN-1);
    strncpy((char *)h->field_name, name, MDV_SHORT_FIELD_LEN-1);
    strncpy((char *)h->units, "unknown", MDV_UNITS_LEN-1);

    /* NO transforamtion */
    h->transform[0] = '\0';

    /* fill in fortran record size -- (record_len1=record_len2) */
    h->record_len2 = h->record_len1;

    /* allocate buffer space for all planes of data */
    buf[ifield] = MEM_CALLOC((h->nx)*(h->ny)*(h->nz), unsigned char);
}  
 
/*----------------------------------------------------------------*/
static void uf2mdv_gate_compute(float uf0, float uf_delta, int nuf,
				float mdv0, float mdv_delta, int nmdv,
				int *u0, int *m0)
{
    if (uf0 > mdv0)
    {
	*m0 = (uf0 - mdv0)/mdv_delta;
	*u0 = 0;
    }
    else
    {
	*u0 = (mdv0 - uf0)/uf_delta;
	*m0 = 0;
    }
}
    
/*----------------------------------------------------------------*/
/*
 *  flag data values into buffer.
 */
static void flag_data(MDV_field_header_t *F, unsigned char *out)
{
    int m;
    
    for (m=0; m<F->nx; ++m)
	out[m] = F->missing_data_value;
}

/*----------------------------------------------------------------*/
/*
 *  fill in data values from uf
 */
static void fill_data(char *uf, Mandatory_header_block *M,
		      Field_header *f, MDV_field_header_t *F,
		      uf2mdv_input_filter filter,
		      unsigned char *out)
{
    signed short *spt0;        /* pointer to short */
    signed short *spt;        /* pointer to short */
    int u0, m0, u, m, ipv, y;
    float value, pv;
    float uf_min, uf_max;
    
    y = M->year + params.year_convert;
/*     if ((int)M->year < 50 ) */
/* 	y = (int)M->year + 2000; */
/*     else */
/* 	y = (int)M->year + 1900;	 */

    mmh.time_expire = UT_gmt_to_unix(y, (int)M->month,
				       (int)M->day, (int)M->hour,
				       (int)M->minute, (int)M->second);
    mmh.time_end = mmh.time_expire;


    uf_min = (float)( f->rnge_frst_gate*1000 + f->adjustment );
    uf_max = (float)( uf_min + (f->vol_space*(f->num_samples-1)) );

    pv = F->grid_dx*1000.0;
    ipv = (int)pv;
    if (f->vol_space != ipv)
    {
	printf("FATAL ERROR uf grid spacing=%d  MDV grid spacing=%d\n",
	       f->vol_space, ipv);
	exit(0);
    }
    uf2mdv_gate_compute(uf_min, (float)f->vol_space, f->num_samples,
			F->grid_minx*1000.0, F->grid_dx*1000.0, F->nx,
			&u0, &m0);
    spt0 = (signed short *)(POSITION(uf, f->position));
    for (m=0; m<m0; ++m)
	out[m] = F->missing_data_value;
    /*printf("Filling %d values\n", F->nx - m0);*/
    for (u=u0, m=m0; m<F->nx; ++m, ++u)
    {
	if (u >= f->num_samples)
	    out[m] = F->missing_data_value;
	else
	{
	    spt = spt0 + u;
	    if (*spt == M->missing)
		out[m] = F->missing_data_value;
	    else
	    {
		value = (float)(*spt)/(float)f->scale_factor;
		switch (filter)
		{
		case FILTER_NEGATIVE:
		    value = -1.0*value;
		case FILTER_NONE:
		default:
            break;
		}
		value = (value - F->bias)/F->scale;
		if (value < (double)MIN_MDV_DATA_VALUE)
		    out[m] = MIN_MDV_DATA_VALUE;
		else if (value > (double)MAX_MDV_DATA_VALUE)
		    out[m] = MAX_MDV_DATA_VALUE;
		else
		    out[m] = (unsigned char)value;
		/*printf("\tvalue[%d]=%d\n", m, (int)out[m]);*/
	    }
	}
    }
    return;
}

/*----------------------------------------------------------------*/
static void flag_azimuth(int az_index)
{
    MDV_field_header_t *F;
    int ifield;

    if (last_elev_index < 0)
    {
	printf("ERROR flagging azimuth %d. no elevation information\n",
	       az_index);
	return;
    }

    for (ifield = 0; ifield < mmh.n_fields; ++ifield)
    {
	F = &mfh[ifield];
	flag_data(F, &(buf[ifield][last_elev_index*F->nx*F->ny +
				   az_index*F->nx]));
    }
}

/*----------------------------------------------------------------*/
static void flag_elevation(int el_index)
{
    MDV_field_header_t *F;
    int ifield, y;

    for (y=0; y<mmh.max_ny; ++y)
	for (ifield = 0; ifield < mmh.n_fields; ++ifield)
	{
	    F = &mfh[ifield];
	    flag_data(F, &(buf[ifield][el_index*F->nx*F->ny +
				       y*F->nx]));
	}
}

/*----------------------------------------------------------------*/
static void output_fill_data(char *uf, Mandatory_header_block *m,
			     Field_header *f, int az_index, int el_index,
			     int ifield, uf2mdv_input_filter filter)
{
    unsigned char *writeptr;
    MDV_field_header_t *F;

    /*
     * Point to the right field header
     */
    F = &mfh[ifield];

    /*
     * Point buffer to where to write
     */
    /*printf("Field:%d (el=%d,az=%d) Starting write pointer=%d\n",ifield,
	   el_index, az_index, el_index*F->nx*F->ny + az_index*F->nx);*/
    writeptr = &(buf[ifield][el_index*F->nx*F->ny + az_index*F->nx]);

    /*
     * Write 
     */
    fill_data(uf, m, f, F, filter, writeptr);
}

/*----------------------------------------------------------------*/
void UF_MDV_init_master_header(input_data_t *uf)
{
    int after_fields,y;
/*     double uf_min, uf_max; */
    Mandatory_header_block *m;

    m = (Mandatory_header_block *)uf->data;
    
    /* fill in fortran record size -- assume length of record_len1=length of record_len2)*/
    mmh.record_len1 = sizeof(MDV_master_header_t) - 2*sizeof(mmh.record_len1);

    /* fill in magic cookie */
    mmh.struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE; 

    /* fill in revision code */
    mmh.revision_number = MDV_REVISION_NUMBER;

    /* time fields */

    y = m->year + params.year_convert;
    mmh.time_begin = UT_gmt_to_unix(y, (int)m->month,
				    (int)m->day, m->hour, m->minute,
				    m->second);
    mmh.time_gen = mmh.time_begin;
    mmh.time_end      =  mmh.time_begin;
    mmh.time_centroid = mmh.time_begin;
    mmh.time_expire   =  mmh.time_begin;

    /* number data times associated (ie number of forecasts produced),
     * 1 for sim */
    mmh.num_data_times = 1;

    /* which data time this file represents, 1 for sim */
    mmh.index_number = 1;

    /* dimensionallity of data, 3 for sim */
    mmh.data_dimension = 3;

    /* Collection method. */
    mmh.data_collection_type = 0;

    /* Original vertical level type.  */
    mmh.native_vlevel_type = MDV_VERT_TYPE_ELEV;

    /* This data's vertical level type -- 
     * set to original vertical level type */
    mmh.vlevel_type = MDV_VERT_TYPE_ELEV;

    /* Vlevel included -- no for sim  */
    if (params.use_vertical_levels)
        mmh.vlevel_included = TRUE;
    else
        mmh.vlevel_included = FALSE;

    /* Grid orientation */
    mmh.grid_order_direction = MDV_ORIENT_SN_WE;

    /* Order of indicies. 
     * For basic mdv datasets, write out XYZ (X moves fastest,then Y,then Z) */
    mmh.grid_order_indices =  MDV_ORDER_XYZ;

    /* number fields */
    mmh.n_fields = params.data_types.len;

    /* grid dimensions */
    mmh.max_nx = params.grid_params.ngt;
    mmh.max_ny = params.grid_params.naz;
    mmh.max_nz = params.grid_params.nelev;

    /* make sure number of gint vertical levels less than max allowed in mdv */
    if (mmh.max_nz > MDV_MAX_VLEVELS) {
	fprintf(stderr,"\nToo many vertical levels in file.");
	fprintf(stderr,"\nNlevels = %d",(int)mmh.max_nz);
	fprintf(stderr,"\nMAX ALLOWED in MDV file: %d.",(int)MDV_MAX_VLEVELS);
	exit(0);
    }

    /* number of chunks in file*/
    mmh.n_chunks = 0;

    /* offsets, in bytes from beginning of dataset 
     * Must be in order Master Header, Field Headers, Vlevel Headers,
     * Chunk Headers (chunk headers optional)
     */
    mmh.field_hdr_offset  = sizeof(MDV_master_header_t);
    after_fields = mmh.field_hdr_offset + mmh.n_fields*sizeof(MDV_field_header_t);

    if (params.use_vertical_levels)
        mmh.vlevel_hdr_offset = after_fields;
    else
        mmh.vlevel_hdr_offset = 0;

    if (mmh.n_chunks != 0)
    {
	if (params.use_vertical_levels)
	    mmh.chunk_hdr_offset  = mmh.vlevel_hdr_offset +
		mmh.n_fields*sizeof(MDV_vlevel_header_t);
	else
	    mmh.chunk_hdr_offset = after_fields;
    }
    else
	mmh.chunk_hdr_offset  = 0;

#ifndef __linux
    mmh.field_grids_differ = FALSE;
#endif

    /* sensor origin information.. */
    if (params.override_sensor_location)
    {
	mmh.sensor_lat = params.sensor_latitude;
	mmh.sensor_lon = params.sensor_longitude;
	mmh.sensor_alt = params.sensor_altitude;
    }
    else
    {
	mmh.sensor_lat = (float)m->degrees_lat +
	    (float)m->minutes_lat*1.0/60.0 +
	    (float)m->seconds_lat*1.0/(3600.0)/64.0;
	mmh.sensor_lon = (float)m->degrees_lon +
	    (float)m->minutes_lon*1.0/60.0 +
	    (float)m->seconds_lon*1.0/(3600.0)/64.0;
	mmh.sensor_alt = m->height_antenna;	/* above sea level (meters)*/
    }

    /* fill in fortran record size -- record_len1=record_len2) */
    mmh.record_len2 = mmh.record_len1;
    if (params.debug_level > 0)
	MDV_print_master_header(&mmh, stdout);
}
 
/*----------------------------------------------------------------*/
void UF_MDV_init_field_headers(input_data_t *uf)
{
    int i, ifield, ok;
    Data_header *d;
    char *c, field_name[3];
    Mandatory_header_block *m;
/*     int rvalue; */
    signed short *spt;
    Field_header *f;
/*     int wrote; */
    int *got;


    /* NOTE assume buf is freed at end of each sweep */
    buf = MEM_CALLOC(mmh.n_fields, unsigned char *);

    /* allocate memory for all mdv field headers */
    if (mfh == NULL)
	mfh = MEM_CALLOC(mmh.n_fields, MDV_field_header_t);

    /* allocate mem for all vlevel headers if appropriate */
    if (params.use_vertical_levels)
        mvh = MEM_CALLOC(mmh.n_fields, MDV_vlevel_header_t);
    else
        mvh = NULL;

    got = MEM_CALLOC(mmh.n_fields, int);
    for (i=0; i<mmh.n_fields; ++i)
	got[i] = 0;

    m = (Mandatory_header_block *)uf->data;
    d = (Data_header *)(POSITION(uf->data, m->data_header));
    c = (char *)d + sizeof(Data_header);
    for (i=0; i<d->num_flds_rec; ++i, c+=4) 
    {
	memcpy(field_name, c, 2);
	field_name[2] = 0;
	spt = (signed short *)(c + 2);
	f = (Field_header *)(POSITION(uf->data, *spt));
	for (ifield = 0; ifield < mmh.n_fields; ++ifield)
	{
	    if (strncmp(field_name,
			params.data_types.val[ifield].input_name, 2) == 0)
	    {
		init_field_header(&mfh[ifield], ifield, f, m, d,
				  params.data_types.val[ifield].output_name,
				  params.data_types.val[ifield].scale,
				  params.data_types.val[ifield].bias);
		if (params.use_vertical_levels)
		    init_vlevel_header(&mvh[ifield], &mfh[ifield]);
		got[ifield] = 1;
		break;
	    }	    
	}
    }
    for (ok=1,i=0; i<mmh.n_fields; ++i)
    {
	if (got[i] == 0)
	{
	    ok=0;
	    printf("ERROR in filling field %d name %s none in data\n",
		   i, params.data_types.val[i].input_name);
	}
	else
	    if (params.debug_level > 0)
	    {
		MDV_print_field_header(&(mfh[i]), stdout);
		if (params.use_vertical_levels)
		    MDV_print_vlevel_header(&(mvh[i]), mfh[i].nz,
					    (char *)mfh[i].field_name_long,
					    stdout);
	    }
    }
    if (ok == 0)
	exit(0);
    else
	MEM_FREE(got);
}

/*----------------------------------------------------------------*/
int UF_MDV_process_one_buffer(input_data_t *uf)
{
    Mandatory_header_block *m;
    Data_header *d;
    char *c, field_name[3];
    int i, wrote, ifield, ok, index;
    signed short *spt;
    Field_header *f;
    MDV_field_header_t *F;
    int *got;



    got = MEM_CALLOC(mmh.n_fields, int);
    for (i=0; i<mmh.n_fields; ++i)
	got[i] = 0;

    wrote = 0;
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
	 * we only want what we want
	 */
	for (ok=0,ifield = 0; ifield < mmh.n_fields; ++ifield)
	{
	    if (strncmp(field_name,
			params.data_types.val[ifield].input_name, 2) == 0)
	    {
		output_fill_data(uf->data, m, f, uf->analyzed_state.az_index,
				 uf->analyzed_state.elev_index, ifield,
				 params.data_types.val[ifield].filter);
		last_elev_index = uf->analyzed_state.elev_index;
		got[ifield] = 1;
		ok = 1;
		wrote = 1;
		break;
	    }
	}
	if (ok == 0)
	    monitor_unwanted(field_name);
    }

    for (ok=1,i=0; i<mmh.n_fields; ++i)
    {
	if (got[i] == 0)
	{
	    ok=0;
	    printf("ERROR in filling field %d name %s none in data\n",
		   i, params.data_types.val[i].input_name);
	    F = &mfh[i];
	    index = uf->analyzed_state.elev_index*F->nx*F->ny +
		uf->analyzed_state.az_index*F->nx;
	    flag_data(F, &(buf[i][index]));
	    wrote = 1;
	}
    }
    MEM_FREE(got);
    return wrote;
}


/*----------------------------------------------------------------*/
void UF_MDV_finish_1_sweep(int *az_status, int nazimuth)
{
    int i;

    /*
     * Finalize the time values in the sweep
     */
    mmh.time_centroid = (mmh.time_begin + mmh.time_end)/2;

    /*
     * Flag out all azimuths 
     */
    if (nazimuth != mmh.max_ny)
    {
	printf("FATAL ERROR logic in number of azimuths inconsistant %d %d\n",
	       nazimuth, mmh.max_ny);
	exit(-1);
    }
    for (i=0; i<nazimuth; ++i)
	if (az_status[i] == 0)
	    flag_azimuth(i);
}

/*----------------------------------------------------------------*/
/*
 * Write the entire contents out
 */
void UF_MDV_finish_1_volume(int *elev_status, int nelev)
{
    char *output_file_name=NULL;	/* ascii name of output (mdv) file */
    FILE *outfile;
    int ifield, i;

    /*
     * Flag out remaining elevations
     */
    for (i=0; i<nelev; ++i)
	if (elev_status[i] == 0)
	    flag_elevation(i);

    /*
     * Create mdv file name
     */
    output_file_name =
	MDV_SIM_create_file_name(&mmh, params.output_dir,
				 params.output_file_suffix);
    /*
     * open output (mdv) file
     */
    if ((outfile = fopen(output_file_name,"w")) == NULL)
    {
	printf("Error opening output file: %s\n",output_file_name);
	return;
    }
    if (params.debug_level > 0)
	printf("Opened output file %s\n", output_file_name);


    /*
     * write master header into file
     */
    if (params.debug_level > 0)
	printf("writing master header\n");
    if ((MDV_write_master_header(outfile, &mmh)) == MDV_FAILURE)
    {
	printf("Error occurred during MDV_write_master_header\n");
	return;
    }

    /* write out field headers to file*/
    for (ifield=0; ifield < mmh.n_fields; ++ifield)
    {
	if ((MDV_write_field_header(outfile, &(mfh[ifield]), ifield))
	    == MDV_FAILURE)
	{
	    printf("Error occurred during MDV_write_field_header\n");
	    return;
	}
	if (params.debug_level > 0)
	    printf("wrote field header\n");
    }


    /* write out vlevel headers to file*/
    if (params.use_vertical_levels)
    {
        for (ifield=0; ifield < mmh.n_fields; ++ifield)
	{
	    if ((MDV_write_vlevel_header(outfile, &(mvh[ifield]),
					 &mmh, ifield))	== MDV_FAILURE)
	    {
		printf("Error occurred during MDV_write_vlevel_header\n");
		return;
	    }
	    if (params.debug_level > 0)
	      printf("wrote vlevel header\n");
	}
    }
    
    /*
     * write out data
     */
    if (params.debug_level > 0)
	printf("Writing out mdv file: %s \n", output_file_name);
    for (ifield = 0; ifield < mmh.n_fields ; ifield++)
    {
	/* write unencoded data */
	if (params.output_encoding_type == 0)
	{ 
	    if ((MDV_write_field_data(&(mfh[ifield]), ifield,
				      mfh[ifield].field_data_offset,
				      buf[ifield], MDV_INT8, outfile)) <= 0)
	    {
		printf("failure in MDV_write_field_data (unencoded), %d\n",
		       ifield);
		return;
	    }

	}
	else if (params.output_encoding_type == 1)
	{ 
	    /*  write out in MDV compressed (encoded) format */
	    printf("writing encoded data\n");
	    if ((MDV_write_field_data(&(mfh[ifield]), ifield,
				      mfh[ifield].field_data_offset,
				      buf[ifield], MDV_PLANE_RLE8, 
				      outfile)) <= 0)
	    {
		printf("failure in MDV_write_field_data (encoded), %d\n",
		       ifield);
		return;
	    }

	    /*
	     * update location of next field_data_offset
	     * to reflect encoding
	     */
	    if (ifield + 1 < mmh.n_fields)
		mfh[ifield+1].field_data_offset =
		    mfh[ifield].field_data_offset + 
		    mfh[ifield].volume_size+2*sizeof(int);
	}
            
	/*
	 * Done with this buffer
	 */
	MEM_FREE(buf[ifield]);
    }
    MEM_FREE(buf);
    MEM_FREE(mfh);
    if (params.use_vertical_levels)
        MEM_FREE(mvh);
    fclose(outfile);
}
