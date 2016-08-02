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
 * read_disk_uf.c
 *
 * Reads the universal format Nexrad data from the disk file(s) and converts
 * the information to the format described in <titan/gate_data.h>.
 * Returns a flag indicating if data was read.  At the end of each volume,
 * returns FALSE.  The next call will open new input files and read the
 * first beam from these files.  This was done to simplify the code in this
 * routine, which was getting pretty complicated.
 *
 * This routine makes several assumptions about the input files:
 *   - The input files contain all of the input fields in a single file.
 *   - If the latitude or longitude degree value in an input file is
 *     negative, the minute and second values will also be negative.
 *
 * Nancy Rehak, RAP NCAR March 1995
 *
 **************************************************************************/

#include <dirent.h>
#include <time.h>
#include <toolsa/umisc.h>
#include <rapformats/uf_headers.h>
#include <toolsa/reutil.h>

#include "uf2gate.h"


/*
 * Prototypes for static routines.
 */

static FILE *open_input_file(void);


/*
 * static variables
 */

static int  FileNum = 0;
static int  NumFiles;
static char **FileList;

static FILE  *InputFP = NULL;
static si16 *DiskBuffer;

static int  VolumeNumber = 0;
static int  LastFileVolume = -1;
static int  NewVolumeNum = TRUE;
static int  NewFile = TRUE;
static int  Prev_tilt_num = 100;

uf2gate_tilt_table *TiltInfo = NULL;
  


si32 read_disk_uf (int n_input_files, char **input_file_list,
		   ui08 *beam_buffer, ui08 *param_buffer)
{

  static int first_call = TRUE;
  static int first_beam = TRUE;
  
  /* Variables used in manipulating input files */
  int           field_num;
  float         input_start_range;
  
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
  char *reg_exp[1];
 
  /* intermediate calculations */
  float bias;
  float scale;
  
  int   use_tilt_table;
  int   search_by_tilt_num;
  int   before_beg_of_input_beam;
  int   past_end_of_input_beam;
  
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
     * Allocate space for the buffer containing the information
     * read from disk.
     */

    DiskBuffer = (si16 *) umalloc(MAX_DISK_REC_SIZE);

    /*
     * Get the list of input files to process.
     */

    if (n_input_files == 0) {

      /*
       * get from directory
       */
    
      reg_exp_string = REUTIL_shell_to_regexp(Glob->params.input_file_spec);
      
      reg_exp[0] = reg_exp_string;
      
      FileList = REUTIL_get_file_list(Glob->params.input_file_path,
				      reg_exp, 1, TRUE,
				      &NumFiles);
      
    } else {

      /*
       * use list from command line
       */

      FileList = input_file_list;
      NumFiles = n_input_files;

    }
      
    FileNum = 0;
    
    if (Glob->params.debug)
    {
      fprintf(stdout, "Processing the following input files:\n");
      for (i = 0; i < NumFiles; i++)
	fprintf(stdout, "  %s\n", FileList[i]);
    }
    
    first_call = FALSE;

  } /* endif - first_call */

  /*
   * Open the input file.
   */

  if (InputFP == (FILE *)NULL)
    if ((InputFP = open_input_file()) == (FILE *)NULL)
      return(UF_READ_END_OF_DATA);
  
  /*
   * Read the beam information from the file.
   */

  nread = fread(&nbytes, sizeof(int), 1, InputFP);

  if (nread <= 0)
  {
    /*
     * read error or end of file, close input files and return
     */

    fclose(InputFP);
    if ((InputFP = open_input_file()) == (FILE *)NULL)
      return(UF_READ_END_OF_DATA);

    if ((nread = fread(&nbytes, sizeof(int), 1, InputFP)) <= 0)
      return(UF_READ_BEAM_INVALID);
  } 

  fread(DiskBuffer, sizeof(char), nbytes, InputFP);
  fread(&nbytes, sizeof(int), 1, InputFP);
    
  /*
   * Calculate pointers into the buffers.
   */

  man_header = (uf_mandatory_header_t *)DiskBuffer;
   
  data_header = (uf_data_hdr_info_t *)
    (DiskBuffer + man_header->data_header_pos - 1);
  
  beam_header = (gate_data_beam_header_t *)beam_buffer;
      
  radar_params = (gate_data_radar_params_t *)param_buffer;

  field_params = (gate_data_field_params_t *)
    (param_buffer + sizeof(gate_data_radar_params_t));
    
  /*
   * Determine if we should use the tilt table.
   */

  if (man_header->sweep_mode == UF_SWEEP_PPI)
  {
    switch(Glob->params.tilt_table_flag)
    {
    case GET_TARGET_FROM_TABLE :
      use_tilt_table = TRUE;
      search_by_tilt_num = TRUE;
      break;
      
    case COMPUTE_TILT_NUM_FROM_TABLE :
      use_tilt_table = TRUE;
      search_by_tilt_num = FALSE;
      break;
      
    case DONT_USE_TABLE :
      use_tilt_table = FALSE;
      break;
    } /* endswitch */
  }
  else
    use_tilt_table = FALSE;
  
  if (use_tilt_table)
  {
    if (search_by_tilt_num &&
	(TiltInfo == NULL ||
	 man_header->sweep_num != TiltInfo->tilt_num))
    {
      /*
       * Find the tilt information.
       */

      TiltInfo = (uf2gate_tilt_table *)NULL;
      
      for (i = 0; i < Glob->params.tilt_table.len; i++)
      {
	if (Glob->params.tilt_table.val[i].tilt_num ==
	    man_header->sweep_num)
	{
	  TiltInfo = &(Glob->params.tilt_table.val[i]);
	  break;
	}
      } /* endfor - i */

      if (TiltInfo == (uf2gate_tilt_table *)NULL)
      {
	fprintf(stderr, "WARNING - %s:read_disk_uf\n",
		Glob->prog_name);
	fprintf(stderr, "Tilt %d in primary file but not in tilt table\n",
		man_header->sweep_num);
	fprintf(stderr, "Skipping beam\n");
	
	return(UF_READ_BEAM_INVALID);
      } /* endif - tilt number not in tilt table */

    } /* endif - search_by_tilt_num */
    else
    {
      /*
       * Find the tilt with the closest target elevation.
       */

      double tilt_elev = man_header->elevation / 64.0;
      double elev_diff, temp_elev_diff;
      
      TiltInfo = &(Glob->params.tilt_table.val[0]);
      elev_diff = fabs(TiltInfo->target_elevation - tilt_elev);
      
      for (i = 1; i < Glob->params.tilt_table.len; i++)
      {
	temp_elev_diff = fabs(tilt_elev -
			      Glob->params.tilt_table.val[i].target_elevation);
	
	if (temp_elev_diff < elev_diff)
	{
	  elev_diff = temp_elev_diff;
	  TiltInfo = &(Glob->params.tilt_table.val[i]);
	} /* endif - closer to target elevation */
      }/* endfor - i */
      
    } /* endelse - don't search_by_tilt_num */
  } /* endif - use_tilt_table */
  
  
  /*
   * Update the radar parameter information.
   */

  radar_params->radar_id = Glob->params.radar_params.radar_id;

  if (Glob->params.radar_params.altitude ==
      Glob->params.radar_params.use_file_value)
    radar_params->altitude = man_header->antenna_height;
  else
    radar_params->altitude = (int)Glob->params.radar_params.altitude;

  if (Glob->params.radar_params.latitude ==
      Glob->params.radar_params.use_file_value)
    radar_params->latitude =
      (int)(((float)man_header->lat_degrees +
	     ((float)man_header->lat_minutes / 60.0) +
	     ((float)man_header->lat_seconds / 64.0 / 3600.0)) * 1000000.0
	    + 0.5);
  else
    radar_params->latitude =
      (int)(Glob->params.radar_params.latitude * 1000000.0);

  if (Glob->params.radar_params.longitude ==
      Glob->params.radar_params.use_file_value)
    radar_params->longitude =
      (int)(((float)man_header->lon_degrees +
	     ((float)man_header->lon_minutes / 60.0) +
	     ((float)man_header->lon_seconds / 64.0 / 3600.0)) * 1000000.0
	    + 0.5);
  else
    radar_params->longitude =
      (int)(Glob->params.radar_params.longitude * 1000000.0);

  radar_params->ngates = Glob->params.ngates_out;
  radar_params->gate_spacing = Glob->params.target_gate_spacing * 1000;
  radar_params->start_range = 
    (int)(Glob->params.target_start_range * 1000.0 + 0.05);

  if (Glob->params.radar_params.beam_width ==
      Glob->params.radar_params.use_file_value)
    radar_params->beam_width = 
      (int)((float)field_header->horiz_beam_width / 64.0 * 1000000.0 + 0.5);
  else
    radar_params->beam_width = 
      (int)(Glob->params.radar_params.beam_width * 1000000.0);

  if (Glob->params.radar_params.samples_per_beam ==
      Glob->params.radar_params.use_file_value)
  {
    field_header = (uf_field_header_t *)
      (DiskBuffer + data_header->field_info_array[0].field_pos - 1);
    radar_params->samples_per_beam = field_header->num_samples;
  }
  else
    radar_params->samples_per_beam =
      (int)Glob->params.radar_params.samples_per_beam;

  radar_params->pulse_width = Glob->params.radar_params.pulse_width;

  if (Glob->params.radar_params.prf ==
      Glob->params.radar_params.use_file_value)
    radar_params->prf = 
      (int)(1.0 /
	    ((float)field_header->pulse_rep_time / 1000000.0) *
	    1000.0 + 0.5);
  else
    radar_params->prf = 
      (int)(Glob->params.radar_params.prf * 1000.0);

  if (Glob->params.radar_params.wavelength ==
      Glob->params.radar_params.use_file_value)
  {
    field_header = (uf_field_header_t *)
      (DiskBuffer + data_header->field_info_array[0].field_pos - 1);
    radar_params->wavelength = 
      (int)((float)field_header->wavelength / 64.0 * 10000.0 + 0.5);
  }
  else
    radar_params->wavelength =
      (int)(Glob->params.radar_params.wavelength * 10000.0);
    
  radar_params->nfields = Glob->num_fields_out;
  radar_params->scan_type = 0;
  radar_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
  radar_params->data_field_by_field = Glob->params.data_field_by_field;
  radar_params->nfields_current = Glob->num_fields_out;
  radar_params->field_flag = (1 << Glob->num_fields_out) - 1;

  /*
   * Update the information in the beam header.
   */

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
  if (use_tilt_table)
  {
    beam_header->target_elev =
      (int)(TiltInfo->target_elevation * 1000000.0);
    beam_header->tilt_num = TiltInfo->tilt_num;
  }
  else
  {
    beam_header->target_elev = beam_header->elevation;
    beam_header->tilt_num = man_header->sweep_num;
  }
  
  if ((Glob->params.force_volume_change &&
       beam_header->tilt_num < Prev_tilt_num) ||
      LastFileVolume != man_header->volume_scan_num)
  {
    NewVolumeNum = TRUE;
    LastFileVolume = man_header->volume_scan_num;
  }
  Prev_tilt_num = beam_header->tilt_num;
  
  if (NewVolumeNum || NewFile)
  {
    NewVolumeNum = FALSE;
    NewFile = FALSE;
    VolumeNumber++;
  }
  
  beam_header->vol_num = VolumeNumber;

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

  /*
   * Update the field specific information.
   */

  for (field_num = 0, i = 0; i < Glob->params.field_info.len; i++)
  {
    if (!Glob->params.field_info.val[i].output_flag)
      continue;
    
    field_header = (uf_field_header_t *)
      (DiskBuffer + data_header->field_info_array[i].field_pos - 1);
    
    /*
     * Update the field parameter information
     */

    field_params[field_num].factor =
      Glob->params.field_info.val[field_num].field_factor;    
    field_params[field_num].scale =
      Glob->params.field_info.val[field_num].field_scale;
    field_params[field_num].bias =
      Glob->params.field_info.val[field_num].field_bias;

    bias = (float)field_params[field_num].bias /
      (float)field_params[field_num].factor;
    scale = (float)field_params[field_num].scale /
      (float)field_params[field_num].factor;
    
    /*
     * Copy the data into the buffer.
     */

    input_start_range = field_header->start_range * 1000 +
      field_header->start_center;
    if (Glob->params.radar_params.adj_start_range)
      input_start_range += ((float)field_header->volume_spacing / 2.0);
    
#ifdef DEBUG_ON
    fprintf(stderr, "header start_range = %d km\n",
 	    field_header->start_range);
    fprintf(stderr, "header start_center = %d m\n",
 	    field_header->start_center);
    fprintf(stderr, "input_start_range = %f\n", input_start_range);
    fprintf(stderr, "target_start_range = %f\n", 
	    Glob->params.target_start_range);
    fprintf(stderr, "ngates_out = %d\n", Glob->params.ngates_out);
#endif
    
    for (j = 0; j < Glob->params.ngates_out; j++)
    {
      int disk_data_offset;
      
      /*
       * Find the buffer positions of the data we are interested in.
       */

      if (Glob->params.data_field_by_field)
	beam_data_pos = sizeof(gate_data_beam_header_t) + 
	  (field_num * Glob->params.ngates_out + j);
      else
	beam_data_pos = sizeof(gate_data_beam_header_t) + 
	  Glob->num_fields_out * j + field_num;
      
      disk_data_offset = 
	(int)((((float)Glob->params.target_gate_spacing * (float)j) +
	       Glob->params.target_start_range - input_start_range) /
	      (float)field_header->volume_spacing);

      before_beg_of_input_beam = FALSE;
      if (disk_data_offset < 0)
	before_beg_of_input_beam = TRUE;
      
      disk_data_pos = field_header->data_pos - 1 + disk_data_offset;

      past_end_of_input_beam = FALSE;
      if (disk_data_pos >
	  (field_header->data_pos - 1 + field_header->num_volumes))
      {
	if (Glob->params.debug)
	{
	  fprintf(stdout, "Need sample past end of beam:\n");
	  fprintf(stdout, "   Looking for gate %d, using last gate %d\n",
		 disk_data_pos - field_header->data_pos + 1,
		 field_header->num_volumes);
	}
	
	past_end_of_input_beam = TRUE;
      }
      
      /*
       * Calculate the value to be placed in the beam buffer.
       */

      if (before_beg_of_input_beam)
	beam_buffer[beam_data_pos] = 0;
      else if (past_end_of_input_beam)
	beam_buffer[beam_data_pos] = 0;
      else if (DiskBuffer[disk_data_pos] == man_header->missing_data_val)
	beam_buffer[beam_data_pos] = 0;
      else
      {
	beam_data_val = (int)(((((float)DiskBuffer[disk_data_pos] /
				 (float)field_header->scale_factor) - 
				bias) / scale) + 0.5);

	if (beam_data_val > 255)
	  beam_buffer[beam_data_pos] = 255;
	else if (beam_data_val < 0)
	  beam_buffer[beam_data_pos] = 0;
	else
	  beam_buffer[beam_data_pos] = beam_data_val;
      }
		    
    } /* endfor - j (process each gate) */
    field_num++;
  } /* endfor - i (process each field) */
  

  /*
   * wait a given time
   */

  if (Glob->params.device_read_wait > 0)
    uusleep((ui32) Glob->params.device_read_wait);

  /*
   * return beam_valid flag
   */

  return(UF_READ_BEAM_VALID);
}


static FILE *open_input_file(void)
{
  FILE *fp = NULL;
  
  if (FileNum >= NumFiles)
    return(NULL);
    
  if (Glob->params.debug)
    fprintf(stdout, "Opening input file %s\n", FileList[FileNum]);
      
  if ((fp = fopen(FileList[FileNum], "r"))
      == (FILE *)NULL)
  {
    fprintf(stderr, "ERROR - %s:read_disk_uf.\n",
	    Glob->prog_name);
    fprintf(stderr, "Could not open file: %s\n",
	    FileList[FileNum]);
    perror(FileList[FileNum]);
    tidy_and_exit(-1);
  }

  NewFile = TRUE;
  FileNum++;

  return(fp);
}
