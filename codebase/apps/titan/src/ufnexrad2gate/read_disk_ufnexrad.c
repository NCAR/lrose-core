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
 * read_disk_ufnexrad.c
 *
 * Reads the universal format Nexrad data from the disk file(s) and converts
 * the information to the format described in <titan/gate_data.h>.
 * Returns a flag indicating if data was read.  At the end of each volume,
 * returns FALSE.  The next call will open new input files and read the
 * first beam from these files.  This was done to simplify the code in this
 * routine, which was getting pretty complicated.
 *
 * This routine makes several assumptions about the input files:
 *   - The input files are assumed to contain a single field in each
 *     file.
 *   - The field information for each file is taken from the parameter
 *     file.  Mainly, the field name in the UF file is ignored.
 *   - The minute and second values for the latitude and longitude in
 *     the input files are assumed to always be positive.  This assumption
 *     is debatable based on the wording of some of the file descriptions,
 *     but matches the sample input files received.
 *   - The wavelength for the radar is taken from the parameter file.  The
 *     value in the input file is ignored.
 *   - The missing value indicator for the radar altitude field is -999.
 *   - Sweeps are assumed to always "match up" in the input files.  i.e.,
 *     the sweeps in the first input file specified in the parameter file
 *     (considered the "primary" file) are given in the same order as the
 *     sweeps in the rest of the data files.  If beams for the wrong sweep
 *     are encountered, they are ignored.
 *   - Sweeps appear in the data files in numerical order.  This is only
 *     important if a beam is encountered whose sweep number does not match
 *     that of the primary beam.  In this case, if the sweep number is
 *     larger than expected, the beam will be reread until the matching
 *     sweep is encountered in the primary file.
 *
 * Nancy Rehak, RAP NCAR March 1995
 *
 **************************************************************************/

#include <dirent.h>
#include <time.h>
#include <toolsa/umisc.h>
#include <rapformats/uf_headers.h>
#include <toolsa/reutil.h>

#include "ufnexrad2gate.h"
#include "replace_substring.h"

#define  TILT_ALLOC_INCR      400


/*
 * local typedefs
 */

typedef struct tilt_read_table_t
{
  int    current_tilt;         /* tilt number for beams read */
  int    num_beams;            /* number of beams in the following arrays */
  int    num_alloc;            /* space allocated for the following arrays */
  int    *azimuth;             /* array of azimuths for beams */
  ui08 **beam_info;          /* array of beams read from file */
} tilt_read_table_t;


/*
 * static variables
 */

int VolumeNumber = 0;
tilt_read_table_t *TiltReadTable = NULL;


si32 read_disk_ufnexrad (ui08 *beam_buffer, ui08 *param_buffer)
{

  static int first_call = TRUE;
  static int first_beam = TRUE;
  static si16 *disk_buffer;

  static FILE **input_fp;
  static int  file_num;
  static int  num_files;
  static char **file_list;
  
  static ufnexrad2gate_tilt_table *tilt_info = NULL;
  
  /* Variables used in manipulating input files */
  int           field_num;
  char          input_filename[MAX_PATH_LEN];
  float         input_start_range;
  ui08        *input_buffer;
  int           desired_azimuth;
  int           desired_beam;
  int           beam_diff;
  int           test_beam_diff;
  
  /* Pointers into the disk buffer */
  uf_mandatory_header_t *man_header;
  uf_data_hdr_info_t      *data_header;
  uf_field_header_t     *field_header;
  int                   disk_data_pos;
  
  /* Pointers into the beam buffer */
  gate_data_beam_header_t  *beam_header;
  int                      beam_data_pos;
  int                      beam_data_val;
  
  /* Pointers into the radar parameters buffer */
  gate_data_radar_params_t  *radar_params;
  gate_data_field_params_t  *field_params;
  
  /* regular expression processing */
  char *reg_exp_string;
  char *reg_exp[2];
 
  si32  beam_valid = TRUE;

  int  i, j;
  
  si32 nread;
  int  nbytes;
  
  date_time_t  dt;
  
  /*
   * initialize buffer
   */

  if (first_call)
  {
    /*
     * Make sure there are fields to process.
     */

    if (Glob->params.file_info.len <= 0)
    {
      fprintf(stderr, "ERROR - %s:read_disk_ufnexrad\n", Glob->prog_name);
      fprintf(stderr, "Invalid number of fields to process: %d\n",
	      Glob->params.file_info.len);
      tidy_and_exit(-1);
    }
    
    /*
     * Allocate space for the buffer containing the information
     * read from disk.
     */

    disk_buffer = (si16 *) umalloc(MAX_DISK_REC_SIZE);

    /*
     * Allocate space for the input file pointers.
     */

    input_fp = (FILE **)umalloc((ui32)(Glob->params.file_info.len *
					sizeof(FILE *)));
    
    for (i = 0; i < Glob->params.file_info.len; i++)
      input_fp[i] = (FILE *)NULL;
    
    /*
     * Allocate space for the tilts pre-read from the files.
     */

    TiltReadTable =
      (tilt_read_table_t *)umalloc((ui32)(Glob->params.file_info.len *
					   sizeof(tilt_read_table_t)));
    
    for (i = 0; i < Glob->params.file_info.len; i++)
    {
      TiltReadTable[i].current_tilt = -1;
      TiltReadTable[i].num_beams = 0;
      TiltReadTable[i].num_alloc = 0;
      TiltReadTable[i].azimuth = (int *)NULL;
      TiltReadTable[i].beam_info = (ui08 **)NULL;
    }
    
    /*
     * Get the list of input files to process.
     */

    reg_exp_string = REUTIL_shell_to_regexp(Glob->params.input_file_spec);
    
    reg_exp[0] = reg_exp_string;
    reg_exp[1] = Glob->params.file_info.val[0].file_id_string;
    
    file_list = REUTIL_get_file_list(Glob->params.input_file_path,
				     reg_exp, 2, TRUE,
				     &num_files);
    
    file_num = 0;
    
    if (Glob->params.debug)
    {
      printf("Processing the following input files:\n");
      for (i = 0; i < num_files; i++)
	printf("  %s\n", file_list[i]);
    }
    
    first_call = FALSE;
  } /* if (first_call) */

  /*
   * Open the input files.
   */

  if (input_fp[0] == (FILE *)NULL)
  {
    if (file_num >= num_files)
      return(FALSE);
    
/*    if (Glob->params.debug) */
      printf("Opening input file %s\n", file_list[file_num]);
      
    if ((input_fp[0] = fopen(file_list[file_num], "r"))
	== (FILE *)NULL)
    {
      fprintf(stderr, "ERROR - %s:read_disk_ufnexrad.\n",
	      Glob->prog_name);
      fprintf(stderr, "Could not open file: %s\n",
	      file_list[file_num]);
      perror(file_list[file_num]);
      tidy_and_exit(-1);
    }

    for (i = 1; i < Glob->params.file_info.len; i++)
    {
      replace_substring(file_list[file_num],
			Glob->params.file_info.val[0].file_id_string,
			Glob->params.file_info.val[i].file_id_string,
			input_filename);
      
/*      if (Glob->params.debug) */
	printf("Opening input file %s\n", input_filename);
      
      if ((input_fp[i] = fopen(input_filename, "r"))
	  == (FILE *)NULL)
      {
	fprintf(stderr, "ERROR - %s:read_disk_ufnexrad.\n",
		Glob->prog_name);
	fprintf(stderr, "Could not open file: %s\n",
		file_list[file_num]);
	perror(file_list[file_num]);
	tidy_and_exit(-1);
      }

    }
    
    VolumeNumber++;
    file_num++;
  }
  
  /*
   * Read the beam information from the files.
   */

  for (field_num = 0; field_num < Glob->params.file_info.len; field_num++)
  {
    if (field_num == 0 ||
	tilt_info == (ufnexrad2gate_tilt_table *)NULL ||
	Glob->params.file_info.val[field_num].primary_flag ||
	tilt_info->primary_tilt == tilt_info->secondary_tilt)
    {
      nread = fread(&nbytes, sizeof(int), 1, input_fp[field_num]);

      if (nread <= 0)
      {
	/*
	 * read error or end of file, close input files and return
	 */

	for (i = 0; i < Glob->params.file_info.len; i++)
	{
	  fclose(input_fp[i]);
	  input_fp[i] = (FILE *)NULL;
	}
      
	return(FALSE);
      } 

      fread(disk_buffer, sizeof(char), nbytes, input_fp[field_num]);
      fread(&nbytes, sizeof(int), 1, input_fp[field_num]);
    
    } /* endif - read a single beam from the input file */
    else
    {
      /*
       * Read an entire tilt of information so the azimuths can be
       * matched up.
       */

      if (TiltReadTable[field_num].current_tilt != tilt_info->secondary_tilt ||
	  TiltReadTable[field_num].num_beams <= 0)
      {
	if (TiltReadTable[field_num].num_beams > 0)
	{
	  for (i = 0; i < TiltReadTable[field_num].num_beams; i++)
	    free(TiltReadTable[field_num].beam_info);
	  TiltReadTable[field_num].num_beams = 0;
	}
	
	do
	{
	  nread = fread(&nbytes, sizeof(int), 1, input_fp[field_num]);

	  if (nread <= 0)
	  {
	    /*
	     * read error or end of file, close input files and return
	     */

	    for (i = 0; i < Glob->params.file_info.len; i++)
	    {
	      fclose(input_fp[i]);
	      input_fp[i] = (FILE *)NULL;
	    }
      
	    return(FALSE);
	  } 

	  input_buffer = (ui08 *)umalloc(MAX_DISK_REC_SIZE);
	
	  fread(input_buffer, sizeof(char), nbytes, input_fp[field_num]);
	  fread(&nbytes, sizeof(int), 1, input_fp[field_num]);
    
	  man_header = (uf_mandatory_header_t *)input_buffer;
	
	  if (man_header->sweep_num == tilt_info->secondary_tilt)
	  {
	    if (TiltReadTable[field_num].num_alloc <= 0)
	    {
	      TiltReadTable[field_num].num_alloc = TILT_ALLOC_INCR;
	    
	      TiltReadTable[field_num].azimuth =
		(int *)umalloc((ui32)(TILT_ALLOC_INCR * sizeof(int)));
	      TiltReadTable[field_num].beam_info =
		(ui08 **)umalloc((ui32)(TILT_ALLOC_INCR * sizeof(ui08*)));
	    }
	    else if (TiltReadTable[field_num].num_beams >=
		     TiltReadTable[field_num].num_alloc)
	    {
	      TiltReadTable[field_num].num_alloc += TILT_ALLOC_INCR;
	    
	      TiltReadTable[field_num].azimuth =
		(int *)realloc((char *)TiltReadTable[field_num].azimuth,
			       TiltReadTable[field_num].num_alloc *
			       sizeof(int));
	      TiltReadTable[field_num].beam_info =
		(ui08 **)realloc((char *)TiltReadTable[field_num].beam_info,
				   TiltReadTable[field_num].num_alloc *
				   sizeof(ui08 *));
	    }
	  
	    TiltReadTable[field_num].current_tilt = man_header->sweep_num;
	    TiltReadTable[field_num].
	      azimuth[TiltReadTable[field_num].num_beams] =
		man_header->azimuth;
	    TiltReadTable[field_num].
	      beam_info[TiltReadTable[field_num].num_beams] =
		input_buffer;

	    TiltReadTable[field_num].num_beams++;
	  }
	  else
	  {
	    /*
	     * "Unread" the last beam read.
	     */

	    fseek(input_fp[field_num], -(nbytes + (2 * sizeof(int))), 1);
	    free(input_buffer);
	  }
	
	} while (man_header->sweep_num == tilt_info->secondary_tilt);
      } /* endif - read full tilt */
      
      /*
       * Find the beam with the azimuth nearest the desired one.
       */

      beam_diff = abs(desired_azimuth - TiltReadTable[field_num].azimuth[0]);
      desired_beam = 0;
      for (i = 1; i < TiltReadTable[field_num].num_beams; i++)
      {
	test_beam_diff = abs(desired_azimuth -
			     TiltReadTable[field_num].azimuth[i]);
	if (test_beam_diff < beam_diff)
	{
	  beam_diff = test_beam_diff;
	  desired_beam = i;
	}
      }
      
      memcpy((char *)disk_buffer,
	     (char *)TiltReadTable[field_num].beam_info[desired_beam],
	     MAX_DISK_REC_SIZE);
      
    } /* endelse - read an entire tilt from the file and match azimuths */
    
    /*
     * Calculate pointers into the buffers.
     */

    man_header = (uf_mandatory_header_t *)disk_buffer;
   
    data_header = (uf_data_hdr_info_t *)
      (disk_buffer + man_header->data_header_pos - 1);
  
    field_header = (uf_field_header_t *)
      (disk_buffer + data_header->field_info_array[0].field_pos - 1);
    
    beam_header = (gate_data_beam_header_t *)beam_buffer;
      
    radar_params = (gate_data_radar_params_t *)param_buffer;

    field_params = (gate_data_field_params_t *)
      (param_buffer + sizeof(gate_data_radar_params_t));
    
    if (field_num == 0 &&
	(tilt_info == NULL ||
	 man_header->sweep_num != tilt_info->primary_tilt))
    {
      /*
       * Clear the old tilt read table information.
       */

      for (i = 0; i < Glob->params.file_info.len; i++)
      {
	for (j = 0; j < TiltReadTable[i].num_beams; j++)
	  free(TiltReadTable[i].beam_info[j]);

	TiltReadTable[i].num_beams = 0;
      }
      
      /*
       * Find the tilt information.
       */

      for (i = 0; i < Glob->params.tilt_table.len; i++)
      {
	if (Glob->params.tilt_table.val[i].primary_tilt ==
	    man_header->sweep_num)
	{
	  tilt_info = &(Glob->params.tilt_table.val[i]);
	  break;
	}
      }
      
      if (tilt_info == (ufnexrad2gate_tilt_table *)NULL)
      {
	fprintf(stderr, "WARNING - %s:read_disk_ufnexrad\n",
		Glob->prog_name);
	fprintf(stderr, "Tilt %d in primary file but not in tilt table\n",
		man_header->sweep_num);
	fprintf(stderr, "Skipping beam\n");
	
	return(FALSE);
      }

      /*
       * Update the radar parameter information.
       */

      radar_params->radar_id = Glob->params.radar_params.radar_id;

      if (man_header->antenna_height == -999)
	radar_params->altitude = 0;
      else
	radar_params->altitude = man_header->antenna_height;

      if (man_header->lat_degrees >= 0)
	radar_params->latitude =
	  (int)(((float)man_header->lat_degrees +
		 ((float)man_header->lat_minutes / 60.0) +
		 ((float)man_header->lat_seconds / 64.0 / 3600.0)) * 1000000.0
		+ 0.5);
      else
	radar_params->latitude =
	  (int)(((float)man_header->lat_degrees -
		 ((float)man_header->lat_minutes / 60.0) -
		 ((float)man_header->lat_seconds / 64.0 / 3600.0)) * 1000000.0
		+ 0.5);

      if (man_header->lon_degrees >= 0)
	radar_params->longitude =
	  (int)(((float)man_header->lon_degrees +
		 ((float)man_header->lon_minutes / 60.0) +
		 ((float)man_header->lon_seconds / 64.0 / 3600.0)) * 1000000.0
		+ 0.5);
      else
	radar_params->longitude =
	  (int)(((float)man_header->lon_degrees -
		 ((float)man_header->lon_minutes / 60.0) -
		 ((float)man_header->lon_seconds / 64.0 / 3600.0)) * 1000000.0
		+ 0.5);

      radar_params->ngates = Glob->params.ngates_out;
      radar_params->gate_spacing = Glob->params.target_gate_spacing * 1000;
      radar_params->start_range = Glob->params.target_start_range * 1000;
      radar_params->beam_width = 
	(int)((float)field_header->horiz_beam_width / 64.0 * 1000000.0 + 0.5);
      radar_params->samples_per_beam =
	Glob->params.radar_params.samples_per_beam;
      radar_params->pulse_width = Glob->params.radar_params.pulse_width;
      radar_params->prf = 
	(int)(1.0 /
	      ((float)field_header->pulse_rep_time / 1000000.0) *
	      1000.0 + 0.5);
      radar_params->wavelength =
	(int)(Glob->params.radar_params.wavelength * 10000.0);
      radar_params->nfields = Glob->params.file_info.len;
      radar_params->scan_type = 0;
      radar_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
      radar_params->data_field_by_field = Glob->params.data_field_by_field;
      radar_params->nfields_current = Glob->params.file_info.len;
      radar_params->field_flag = (1 << Glob->params.file_info.len) - 1;
    }

    /*
     * Make sure the tilt numbers match up on the beams.
     */

    if (Glob->params.file_info.val[field_num].primary_flag &&
	man_header->sweep_num != tilt_info->primary_tilt)
    {
      fprintf(stderr, "WARNING - %s:read_disk_ufnexrad\n",
	      Glob->prog_name);
      fprintf(stderr, "Expecting tilt %ld for field %d, "
	      "got tilt %d instead.\n",
	      tilt_info->primary_tilt, field_num, man_header->sweep_num);
      fprintf(stderr, "Skipping beam\n");

      if (man_header->sweep_num > tilt_info->primary_tilt)
	fseek(input_fp[field_num], -(nbytes + (2 * sizeof(int))), 1);
      
      return(FALSE);
    }
    else if (!Glob->params.file_info.val[field_num].primary_flag &&
	man_header->sweep_num != tilt_info->secondary_tilt)
    {
      fprintf(stderr, "WARNING - %s:read_disk_ufnexrad\n",
	      Glob->prog_name);
      fprintf(stderr, "Expecting tilt %ld for field %d, "
	      "got tilt %d instead.\n",
	      tilt_info->secondary_tilt, field_num, man_header->sweep_num);
      fprintf(stderr, "Skipping beam\n");
      
      if (man_header->sweep_num > tilt_info->secondary_tilt)
	fseek(input_fp[field_num], -(nbytes + (2 * sizeof(int))), 1);
      
      return(FALSE);
    }
    
    /*
     * Update the field parameter information.
     */

    field_params[field_num].factor =
      Glob->params.file_info.val[field_num].field_factor;
    field_params[field_num].scale = 
      Glob->params.file_info.val[field_num].field_scale;
    field_params[field_num].bias = 
      Glob->params.file_info.val[field_num].field_bias;
      
    /*
     * Update the information in the beam header.
     */

    if (field_num == 0)
    {
      if (Glob->params.set_time_to_current)
	beam_header->time = time(NULL);
      else
      {
	if (man_header->year > 70)
	  dt.year = 1900 + man_header->year;
	else
	  dt.year = 2000 + man_header->year;
	dt.month = man_header->month;
	dt.day = man_header->day;
	dt.hour = man_header->hour;
	dt.min = man_header->minute;
	dt.sec = man_header->second;
      
	uconvert_to_utime(&dt);
    
	beam_header->time = dt.unix_time + Glob->params.time_correction;
      }

      beam_header->azimuth = (int)((float)man_header->azimuth /
				   64.0 * 1000000.0 + 0.5);
      beam_header->elevation = (int)((float)man_header->elevation /
				     64.0 * 1000000.0 + 0.5);
      beam_header->target_elev =
	(int)(tilt_info->target_elevation * 1000000.0);
      beam_header->vol_num = VolumeNumber;
      beam_header->tilt_num = man_header->sweep_num;

      if (first_beam)
      {
	beam_header->new_scan_limits = TRUE;
	first_beam = FALSE;
      }
      else
	beam_header->new_scan_limits = FALSE;

      /*
       * end_of_tilt and end_of_volume are always returned as FALSE.
       * These values are determined elsewhere (in set_beam_flags).
       */

      beam_header->end_of_tilt = FALSE;
      beam_header->end_of_volume = FALSE;

      desired_azimuth = man_header->azimuth;
    }

    /*
     * Copy the data into the buffer.
     */

    input_start_range = (float)field_header->volume_spacing / 2.0;
      
    for (i = 0; i < Glob->params.ngates_out; i++)
    {
      /*
       * Find the buffer positions of the data we are interested in.
       */

      if (Glob->params.data_field_by_field)
	beam_data_pos = sizeof(gate_data_beam_header_t) + 
	  (field_num * Glob->params.ngates_out + i);
      else
	beam_data_pos = sizeof(gate_data_beam_header_t) + 
	  Glob->params.file_info.len * i + field_num;
      
      disk_data_pos = field_header->data_pos - 1 +
	(int)((((float)Glob->params.target_gate_spacing * i) +
	       (float)Glob->params.target_start_range - input_start_range) /
	      (float)field_header->volume_spacing + 0.5);
      if (disk_data_pos >
	  (field_header->data_pos - 1 + field_header->num_volumes))
      {
	if (Glob->params.debug)
	{
	  printf("Need sample past end of beam:\n");
	  printf("   Looking for gate %d, using last gate %d\n",
		 disk_data_pos - field_header->data_pos + 1,
		 field_header->num_volumes);
	}
	
	disk_data_pos = field_header->data_pos - 1 + field_header->num_volumes;
      }
      
      /*
       * Calculate the value to be placed in the beam buffer.
       */

      beam_data_val = (int)(((((float)disk_buffer[disk_data_pos] /
			      (float)field_header->scale_factor) - 
			     ((float)field_params[field_num].bias /
			      (float)field_params[field_num].factor)) / 
			    ((float)field_params[field_num].scale /
			     (float)field_params[field_num].factor)) + 0.5);

      if (disk_buffer[disk_data_pos] == man_header->missing_data_val)
	beam_buffer[beam_data_pos] = 0;
      else if (beam_data_val > 255)
	beam_buffer[beam_data_pos] = 255;
      else if (beam_data_val < 0)
	beam_buffer[beam_data_pos] = 0;
      else
	beam_buffer[beam_data_pos] = beam_data_val;
    } /* endfor - i */
  } /* endfor - field_num */

  /*
   * wait a given time
   */

  if (Glob->params.device_read_wait > 0)
    uusleep((ui32) Glob->params.device_read_wait);

  /*
   * return beam_valid flag
   */

  return(beam_valid);
}

