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
/***************************************************************************
 * write_ced_file.c
 *
 * write out an interpolated boundary layer array into CEDRIC file format
 *
 * Jason Helland
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#include "trec.h"
#include "cedric.h"

static void dec_deg_to_min_sec(double dec_deg,
			       int *ideg_p, int *imin_p, int *isec_p);

static void
filldata(float ***cart, int nx, int ny, int sf, int k, short *array);


/*#define WRITE_DEBUG*/

void write_ced_file(date_time_t *sample_time,
		    ll_params_t *beam,
		    dimension_data_t *dim_data,
		    float ***ucart,
		    float ***vcart,
		    float ***wcart,
		    float ***conv,
		    float ***dcart)

{

  cedric_header_t ced_hdr;

  FILE *ced_file;
  short int num_header_recs, num_data_recs, tot_bytes;
  short h_pad_size, d_pad_size, *h_pad, *d_pad;
  long a_num_bytes, h_num_bytes;
  int k,ierr;
  int ideg, imin, isec;

  char tmp_file_path[MAX_PATH_LEN];
  char file_name[MAX_PATH_LEN];
  char remote_file[MAX_PATH_LEN];
  char local_file_path[MAX_PATH_LEN];
  char remote_file_path[MAX_PATH_LEN];
  char local_dir_path[MAX_PATH_LEN];
  char remote_dir_path[MAX_PATH_LEN];
  char call_str[MAX_PATH_LEN];
  char tmp_file[MAX_PATH_LEN];
  char tmp_file1[MAX_PATH_LEN];

  static int first_call = TRUE;
  static char prev_remote_dir_path[MAX_PATH_LEN];
  static char prev_local_dir_path[MAX_PATH_LEN];
  static short int *array;

  if (Glob->params.debug) {
    fprintf(stderr, "*** write_ced_file ***\n");
  }

  PMU_auto_register("writing cedric file");

  /*
   * if first call, initialize
   */
  
  if (first_call) {
    
    strcpy(prev_local_dir_path, " ");
    strcpy(prev_remote_dir_path, " ");
    array = (short *)
      umalloc (dim_data->nx * dim_data->ny * sizeof(short));

    first_call = FALSE;
     
  }
  
  /*
   * compute output file name from the date and time of the input file
   */
    
  sprintf(local_dir_path, "%s%s%.4d%.2d%.2d",
	  Glob->params.local_dir, PATH_DELIM,
	  sample_time->year,
	  sample_time->month,
	  sample_time->day);
    
  sprintf(remote_dir_path, "%s%s%.4d%.2d%.2d",
	  Glob->params.remote_dir, PATH_DELIM,
	  sample_time->year,
	  sample_time->month,
	  sample_time->day);
    
  sprintf(file_name, "%.2d%.2d%.2d%s",
	  sample_time->hour,
	  sample_time->min,
	  sample_time->sec,
	  Glob->params.file_suffix);

  sprintf(remote_file,"trecmud.%.2d%.2d",
          sample_time->hour,
          sample_time->min);

  sprintf(local_file_path, "%s%s%s",
	  local_dir_path, PATH_DELIM,
	  file_name);

  sprintf(remote_file_path, "%s%s%s",
	  remote_dir_path, PATH_DELIM,
	  file_name);


  if (Glob->params.debug) {
    fprintf(stderr,"file_name= %s\n",file_name);
    fprintf(stderr,"remote_file= %s\n",remote_file);
    fprintf(stderr,"local_file_path= %s\n",local_file_path);
    fprintf(stderr,"remote_file_path= %s\n",remote_file_path);
  }

  sprintf(tmp_file,"%s%strectmp.out",remote_dir_path,PATH_DELIM);
  sprintf(tmp_file1,"%s%strectmp.out",local_dir_path,PATH_DELIM);
      
  /*
   * if current directory paths are not same as previous ones,
   * create directories
   */
    
  
  if (Glob->params.local_files) {

    if (Glob->params.debug) {
      fprintf(stderr, "local_dir_path= %s\n", local_dir_path);
    }
    
    if (strcmp(local_dir_path, prev_local_dir_path)) {
	
      sprintf(call_str, "mkdir %s", local_dir_path);
      if (Glob->params.debug) {
	fprintf(stderr, "call_str=%s \n\n", call_str);
      }
      system(call_str);
      strcpy(prev_local_dir_path, local_dir_path);
	
    }
      
  } /* if (Glob->params.local_files) */
    
  if (Glob->params.remote_files) {
    
    if (strcmp(remote_dir_path, prev_remote_dir_path)) {
	
      sprintf(call_str, "rsh -l %s -n %s 'mkdir %s' &",
	      Glob->params.target_machine_login,
	      Glob->params.target_machine_name, remote_dir_path);
      system(call_str);
      if (Glob->params.debug) {
	fprintf(stderr, "%s\n", call_str);
      }
      strcpy(prev_remote_dir_path, remote_dir_path);

    }

  } /* if(Glob->params.remote_files) */   

  
  /*************
   * open up file for writing
   */

  sprintf(tmp_file_path, "%s%s%s", Glob->params.local_dir, PATH_DELIM, 
	  "trectmp.out");
  if((ced_file = fopen(tmp_file_path, "w")) == NULL) {
    fprintf(stderr,"Couldn't open %s\n",tmp_file_path);
    exit(0);
  }

  /*****************
   * initialize header structure to zero
   */

  memset ((void *) &ced_hdr, 0, sizeof(ced_hdr));

  /****************
   * fill in relevant parts of ced_hdr
   */

  strcpy(ced_hdr.id, "ZTOP    ");
  strcpy(ced_hdr.program, "TREC  ");
  strcpy(ced_hdr.project, beam->proj_name);
  strcpy(ced_hdr.scientist, getenv("USER"));
  strcpy(ced_hdr.submitter, getenv("USER"));
  strcpy(ced_hdr.radar, beam->radar_name);
  strcpy(ced_hdr.scan_mode, "CRT ");
  strcpy(ced_hdr.tape, "UNKNWN");
  strcpy(ced_hdr.field[0].field_name, "U       ");
  strcpy(ced_hdr.field[1].field_name, "V       ");
  strcpy(ced_hdr.field[2].field_name, "W       ");
  strcpy(ced_hdr.field[3].field_name, "CONV    ");
  strcpy(ced_hdr.field[4].field_name, "DOPPLER ");
  strcpy(ced_hdr.vol_name, "TREC    ");

  ced_hdr.begin_year = sample_time->year;
  ced_hdr.begin_month = sample_time->month;
  ced_hdr.begin_day = sample_time->day;
  ced_hdr.begin_hour = sample_time->hour;
  ced_hdr.begin_min = sample_time->min;
  ced_hdr.begin_second = sample_time->sec;
  ced_hdr.end_year = sample_time->year;
  ced_hdr.end_month = sample_time->month;
  ced_hdr.end_day = sample_time->day;
  ced_hdr.end_hour = sample_time->hour;
  ced_hdr.end_min = sample_time->min;
  ced_hdr.end_second = sample_time->sec;

  ced_hdr.bad_data = CED_MISSING_DATA;
  ced_hdr.scale_factor = 100;
  ced_hdr.cf = 64;
  ced_hdr.block_size = dim_data->nx*dim_data->ny;
  ced_hdr.bits_datum = 16;

  ced_hdr.min_x = (int) (dim_data->min_x * 100.0 + 0.5);
  ced_hdr.min_y = (int) (dim_data->min_y * 100.0 + 0.5);
  ced_hdr.min_z = (int) (dim_data->min_z * 1000.0 + 0.5);
  ced_hdr.max_x = (int) (dim_data->max_x * 100.0 + 0.5);
  ced_hdr.max_y = (int) (dim_data->max_y * 100.0 + 0.5);
  ced_hdr.max_z = (int) (dim_data->max_z * 1000.0 + 0.5);
  ced_hdr.dx = (int) (dim_data->del_x * 1000.0 + 0.5);
  ced_hdr.dy = (int) (dim_data->del_y * 1000.0 + 0.5);
  ced_hdr.dz = (int) (dim_data->del_z * 1000.0 + 0.5);
  ced_hdr.nx = dim_data->nx;
  ced_hdr.ny = dim_data->ny;
  ced_hdr.nz = dim_data->nz;

  ced_hdr.angle1 =
    (short) ((90.0 + Glob->params.grid_rotation) * ced_hdr.cf + 0.5);

  ced_hdr.fast_axis = 1;
  ced_hdr.mid_axis = 2;
  ced_hdr.slow_axis = 3;

  ced_hdr.field[0].field_sf = 100;
  ced_hdr.field[1].field_sf = 100;
  ced_hdr.field[2].field_sf = 1;
  ced_hdr.field[3].field_sf = 1;
  ced_hdr.field[4].field_sf = 100;

  ced_hdr.num_gates_beam = beam->range_seg[0].gates_per_beam;
  ced_hdr.gate_spacing = beam->range_seg[0].gate_spacing;
  ced_hdr.min_azmith = 0;
  ced_hdr.max_azmith = 359;

  dec_deg_to_min_sec(Glob->params.radar_lat,
		     &ideg, &imin, &isec);

  ced_hdr.lat_deg = ideg;
  ced_hdr.lat_min = imin;
  ced_hdr.lat_sec = isec;

  dec_deg_to_min_sec(Glob->params.radar_lon,
		     &ideg, &imin, &isec);

  ced_hdr.lon_deg = ideg;
  ced_hdr.lon_min = imin;
  ced_hdr.lon_sec = isec;

  ced_hdr.origin_height = beam->altitude;

  strcpy(ced_hdr.time_zone, "UCT ");
  strcpy(ced_hdr.computer, "UX");
  strcpy(ced_hdr.plane_type, "CO");

  ced_hdr.header_record_length = 510;
  ced_hdr.num_planes = 1;
  ced_hdr.num_radars = 1;
  ced_hdr.num_fields = 5;

  sprintf(ced_hdr.time_run, "%d:%d:%d",
	  sample_time->hour,
	  sample_time->min,
	  sample_time->sec);

  sprintf(ced_hdr.date, "%d/%d/%d",
	  sample_time->month,
	  sample_time->day,
	  sample_time->year);



  /********************
   * compute needed pad for writing out header
   */

  h_num_bytes = ced_hdr.header_record_length;
  num_header_recs = (h_num_bytes / ced_hdr.block_size) + 1;
  tot_bytes = num_header_recs * ced_hdr.block_size;
  h_pad_size = (tot_bytes - h_num_bytes);

#ifdef WRITE_DEBUG
  fprintf(stderr, "Header pad_size: %d\n", h_pad_size);
  fprintf(stderr, "Should be 2690!\n");
#endif

  /*********************
   * malloc pad array for writing after header and initialize to zero
   */

  h_pad = (short int *) umalloc(h_pad_size*sizeof(short int));
  memset((void *)h_pad, 0, h_pad_size);

#ifdef WRITE_DEBUG
  fprintf(stderr, "Size of malloc'd h_pad: %d\n", sizeof(h_pad));
  fprintf(stderr, "Dimensions:  minx: %d  miny: %d  minz: %d\n", 
	  ced_hdr.min_x, ced_hdr.min_y, ced_hdr.min_z);
  fprintf(stderr, "Dimensions:  maxx: %d  maxy: %d  maxz: %d\n", 
	  ced_hdr.max_x, ced_hdr.max_y, ced_hdr.max_z);
  fprintf(stderr, "Dimensions:  delx: %d  dely: %d  delz: %d\n", ced_hdr.dx, 
	  ced_hdr.dy, ced_hdr.dz);
  fprintf(stderr, "Dimensions:  nx: %d  ny: %d  nz: %d\n", ced_hdr.nx, 
	  ced_hdr.ny, ced_hdr.nz);
#endif

  /********************
   * compute needed pad for writing out array
   */

  a_num_bytes = (ced_hdr.nx * ced_hdr.ny);
  num_data_recs = (short)((float)(a_num_bytes / ced_hdr.block_size) + 0.99);

  ced_hdr.records_plane = num_data_recs;
  ced_hdr.records_field = num_data_recs*ced_hdr.num_fields;
  ced_hdr.records_volume = ced_hdr.records_field*ced_hdr.nz;
  ced_hdr.total_records=ced_hdr.records_volume+1;
  ced_hdr.points_plane = (u_short)a_num_bytes;

  tot_bytes = num_data_recs * ced_hdr.block_size;
  d_pad_size = (tot_bytes - a_num_bytes);
  
  if (Glob->params.debug) {
    fprintf(stderr, "nx,ny,nz= %5d %5d %5d\n",
	    ced_hdr.nx,ced_hdr.ny,ced_hdr.nz);
    fprintf(stderr, "block_size,records_plane= %5d %5d\n",
	    ced_hdr.block_size,
	    ced_hdr.records_plane);
  }

  /*******************
   * malloc pad array for writing after data and initialize to zero
   */

  d_pad = (short int *) umalloc(d_pad_size*sizeof(short int));
  memset((void *)d_pad, 0, d_pad_size);

#ifdef WRITE_DEBUG
  fprintf(stderr, "File  size: %d\n", (num_data_recs + 1)*3200*2);
  fprintf(stderr, "Recs_plane: %d\n", ced_hdr.records_plane);
  fprintf(stderr, "Recs_field: %d\n", ced_hdr.records_field);
  fprintf(stderr, "Recs_volume: %d\n", ced_hdr.records_volume);
#endif


  /*******************
   * write out header and pad & data and pad
   */
                                                 
  fwrite((char *)&ced_hdr, sizeof(cedric_header_t),1 ,ced_file);
  fwrite((char *)h_pad, sizeof(short int),h_pad_size ,ced_file);
  for(k=0; k<ced_hdr.nz; k++)
    {     
      filldata(ucart,ced_hdr.nx,ced_hdr.ny,ced_hdr.field[0].field_sf,k,array);
      fwrite((char *)array, sizeof(short int),(dim_data->nx * dim_data->ny),
	     ced_file);
    }
  for(k=0; k<ced_hdr.nz; k++)
    {     
      filldata(vcart,ced_hdr.nx,ced_hdr.ny,ced_hdr.field[1].field_sf,k,array);
      fwrite((char *)array, sizeof(short int),(dim_data->nx * dim_data->ny),
	     ced_file);
    }
  for(k=0; k<ced_hdr.nz; k++)
    {     
      filldata(wcart,ced_hdr.nx,ced_hdr.ny,ced_hdr.field[2].field_sf,k,array);
      fwrite((char *)array, sizeof(short int),(dim_data->nx * dim_data->ny),
	     ced_file);
    }
  for(k=0; k<ced_hdr.nz; k++)
    {     
      filldata(conv,ced_hdr.nx,ced_hdr.ny,ced_hdr.field[3].field_sf,k,array);
      fwrite((char *)array, sizeof(short int),(dim_data->nx * dim_data->ny),
	     ced_file);
    }
  for(k=0; k<ced_hdr.nz; k++)
    {     
      filldata(dcart,ced_hdr.nx,ced_hdr.ny,ced_hdr.field[4].field_sf,k,array);
      fwrite((char *)array, sizeof(short int),(dim_data->nx * dim_data->ny),
	     ced_file);
    }

  /*
   * close file and release mem
   */
  
  fclose (ced_file);
  ufree ((char *) h_pad);
  ufree ((char *) d_pad);

#ifdef WRITE_DEBUG  
  fprintf(stderr, "Wrote pad of length %d after data.\n", d_pad_size);
#endif

  /*
   * if remote files, rcp file to remote host
   */

  if (Glob->params.remote_files) {
      
    sprintf(call_str, "rcp %s %s@%s:%s",
	    tmp_file_path, 
	    Glob->params.target_machine_login,
	    Glob->params.target_machine_name,
	    tmp_file);
    if (Glob->params.debug) {
      fprintf(stderr,"%s \n\n",call_str);
    }

    ierr=system(call_str);
    if (Glob->params.debug) {
      fprintf(stderr,"ierr= %8d\n",ierr);
    }

    sprintf(call_str,"rsh -l %s -n %s 'mv %s %s'",
	    Glob->params.target_machine_login, 
	    Glob->params.target_machine_name, 
	    tmp_file, remote_file_path);
    if (Glob->params.debug) {
      fprintf(stderr,"\n %s \n",call_str);
    }

    ierr=system(call_str);
    if (Glob->params.debug) {
      fprintf(stderr,"ierr= %8d\n",ierr);
    }
  }    

  /*
   * if local files are to be stored, change the name from the
   * tmp name to the correct name, else remove tmp file
   */
  
  if (Glob->params.local_files) {
    sprintf(call_str,"cp %s %s",tmp_file_path,local_file_path);
    ierr=system(call_str);
    fprintf(stderr, "Wrote file to %s\n",local_file_path);
  }

}

/***********************
 * dec_deg_to_min_sec()
 */

static void dec_deg_to_min_sec(double dec_deg,
			       int *ideg_p, int *imin_p, int *isec_p)

{

  int ideg, imin, isec;
  double fmin, fsec;

  ideg = (int) dec_deg;
  fmin = fabs(dec_deg - ideg) * 60.0;
  imin = (int) fmin;
  fsec = (fmin - imin) * 60.0;
  isec = (int) (fsec + 0.5);

  *ideg_p = ideg;
  *imin_p = imin;
  *isec_p = isec;

  return;

}

/***********
 * filldata()
 ***********/

static void
filldata(float ***cart, int nx, int ny, int sf, int k, short *array)

{
  int i, j, index;

  index=0;
  for(j=0; j<ny; j++) {
    for(i=0; i<nx; i++) {
      array[index]=cart[i][j][k]*sf;
      if (cart[i][j][k] == Glob->params.bad) {
	array[index] = (short)Glob->params.bad;
      }
      index++;
    }
  }

}

