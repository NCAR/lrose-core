// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * read_params.c: reads the parameters, loads up the globals
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "polar2gate.h"

void read_params(void)

{

  char work_buf[BUFSIZ];
  char *resource_str;
  char *elev_table_str;
  char *start_pt, *end_pt;
  char *token;
  si32 i;
  float elev;
  double device_read_wait;
  
  /*
   * set default print intervals
   */
  
  Glob->header_interval = uGetParamLong(Glob->prog_name,
					"header_interval",
					HEADER_INTERVAL);
  
  Glob->summary_interval = uGetParamLong(Glob->prog_name,
					 "summary_interval",
					 SUMMARY_INTERVAL);
  
  Glob->output_port = uGetParamLong(Glob->prog_name, "output_port",
				    OUTPUT_PORT);
  
  Glob->device_name = uGetParamString(Glob->prog_name,
				      "device_name", DEVICE_NAME);
  
  /*
   * buffer sizes
   */
  
  Glob->max_nbytes_beam_buffer = uGetParamLong(Glob->prog_name,
					       "max_nbytes_beam_buffer",
					       MAX_NBYTES_BEAM_BUFFER);
  
  /*
   * time correction
   */
  
  Glob->time_correction = uGetParamLong(Glob->prog_name,
					"time_correction",
					TIME_CORRECTION);
  
  /*
   * header type
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "header_type", HEADER_TYPE);

  if (!strcmp(resource_str, "lincoln")) {
    Glob->header_type = LINCOLN_HEADER;
  } else if (!strcmp(resource_str, "rp7")) {
    Glob->header_type = RP7_HEADER;
  } else if (!strcmp(resource_str, "chill")) {
    Glob->header_type = CHILL_HEADER;
  } else if (!strcmp(resource_str, "lass")) {
    Glob->header_type = LASS_HEADER;
  } else if (!strcmp(resource_str, "alenia")) {
    Glob->header_type = ALENIA_HEADER;
  } else {
    fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
    fprintf(stderr, "Param file %s\n", Glob->params_path_name);
    fprintf(stderr, "Invalid entry for 'header_type': %s\n",
	    resource_str);
    fprintf(stderr, "Valid entries are:\n");
    fprintf(stderr, "                  lincoln\n");
    fprintf(stderr, "                  rp7\n");
    fprintf(stderr, "                  chill\n");
    fprintf(stderr, "                  lass\n");
    fprintf(stderr, "                  alenia\n");
    tidy_and_exit(-1);
  }
  
  /*
   * input format
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "input_format", INPUT_FORMAT);
  
  if (!strcmp(resource_str, "ncar")) {
    Glob->input_format = NCAR_FORMAT;
  } else if (!strcmp(resource_str, "chill")) {
    Glob->input_format = CHILL_FORMAT;
  } else if (!strcmp(resource_str, "lass")) {
    Glob->input_format = LASS_FORMAT;
  } else if (!strcmp(resource_str, "alenia")) {
    Glob->input_format = ALENIA_FORMAT;
  } else {
    fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
    fprintf(stderr, "Param file %s\n", Glob->params_path_name);
    fprintf(stderr, "Invalid entry for 'input_format': %s\n",
	    resource_str);
    fprintf(stderr, "Valid entries are:\n");
    fprintf(stderr, "                  ncar\n");
    fprintf(stderr, "                  chill\n");
    fprintf(stderr, "                  lass\n");
    fprintf(stderr, "                  alenia\n");
    tidy_and_exit(-1);
  }
  
  /*
   * set set_time_to_current option
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "set_time_to_current",
				 SET_TIME_TO_CURRENT);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->set_time_to_current,
			    "set_time_to_current"))
    tidy_and_exit(-1);
  
  /*
   * scan_mode
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "scan_mode", SCAN_MODE);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->scan_mode,
			"sector", GATE_DATA_SECTOR_MODE,
			"rhi", GATE_DATA_RHI_MODE,
			"surveillance", GATE_DATA_SURVEILLANCE_MODE,
			"scan_mode"))
    tidy_and_exit(-1);
  
  /*
   * set check_gate_spacing option
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_gate_spacing",
				 CHECK_GATE_SPACING);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->check_gate_spacing, "check_gate_spacing"))
    tidy_and_exit(-1);
  
  
  Glob->target_gate_spacing =
    (si32) (uGetParamDouble(Glob->prog_name,
			    "target_gate_spacing",
			    TARGET_GATE_SPACING) * 1000.0 + 0.5);

  /*
   * wait interval
   */
  
  device_read_wait = uGetParamDouble(Glob->prog_name,
				     "device_read_wait",
				     DEVICE_READ_WAIT);
  
  if (device_read_wait > MAX_DEVICE_READ_WAIT) {
    device_read_wait = MAX_DEVICE_READ_WAIT;
  }

  Glob->device_read_wait = (si32) (device_read_wait * 1000000.0 + 0.5);
  
  /*
   * radar params
   */
  
  Glob->radar_id = uGetParamLong(Glob->prog_name, "radar_id", RADAR_ID);
  
  Glob->samples_per_beam = uGetParamLong(Glob->prog_name,
					 "samples_per_beam",
					 SAMPLES_PER_BEAM);
  
  Glob->altitude = uGetParamDouble(Glob->prog_name,
				   "altitude",
				   ALTITUDE);
  
  Glob->longitude = uGetParamDouble(Glob->prog_name,
				    "longitude",
				    LONGITUDE);
  
  Glob->latitude = uGetParamDouble(Glob->prog_name,
				   "latitude",
				   LATITUDE);
  
  Glob->gate_spacing = uGetParamDouble(Glob->prog_name,
				       "gate_spacing",
				       GATE_SPACING);
  
  Glob->start_range = uGetParamDouble(Glob->prog_name,
				      "start_range",
				      START_RANGE);
  
  Glob->beam_width = uGetParamDouble(Glob->prog_name,
				     "beam_width",
				     BEAM_WIDTH);
  
  Glob->pulse_width = uGetParamDouble(Glob->prog_name,
				      "pulse_width",
				      PULSE_WIDTH);
  
  Glob->prf_nominal = uGetParamDouble(Glob->prog_name,
				      "prf_nominal",
				      PRF_NOMINAL);
  
  Glob->wavelength = uGetParamDouble(Glob->prog_name,
				     "wavelength",
				     WAVELENGTH);
  
  /*
   * gates input and output
   */
  
  Glob->ngates_in = uGetParamLong(Glob->prog_name,
				  "ngates_in",
				  NGATES_IN);
  
  Glob->ngates_dropped = uGetParamLong(Glob->prog_name,
				       "ngates_dropped",
				       NGATES_DROPPED);
  
  Glob->ngates_out = uGetParamLong(Glob->prog_name,
				   "ngates_out",
				   NGATES_OUT);
  
  /*
   * number of input fields
   */
  
  Glob->nfields_in = uGetParamLong(Glob->prog_name, "nfields_in",
				   NFIELDS_IN);
  
  /*
   * output field positions
   */
  
  Glob->nfields_out = uGetParamLong(Glob->prog_name, "nfields_out",
                                    NFIELDS_OUT);
  
  resource_str = uGetParamString(Glob->prog_name, "out_field_pos",
				 OUT_FIELD_POS);
  
  Glob->out_field_pos =
    (si32 *) umalloc((ui32) (Glob->nfields_out * sizeof(double)));
  
  end_pt = resource_str;
  
  for (i = 0; i < Glob->nfields_out; i++) {
    
    start_pt = end_pt;
    
    errno = 0;
    Glob->out_field_pos[i] = strtod(start_pt, &end_pt);
    
    if (errno != 0) {
      fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
      fprintf(stderr, "Decoding field pos string '%s'\n", resource_str);
      fprintf(stderr, "Edit parameters file '%s'\n", Glob->params_path_name);
      perror(start_pt);
      tidy_and_exit(-1);
    }
    
  } /* i */

  if (Glob->input_format == CHILL_FORMAT) {

    if (Glob->header_type != CHILL_HEADER) {
      fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
      fprintf(stderr, "With CHILL_FORMAT you must use CHILL_HEADER\n");
      tidy_and_exit(-1);
      
    }

    /*
     * chill radar calibration data file
     */

    Glob->chill_calibration_path =
      uGetParamString(Glob->prog_name,
		      "chill_calibration_path",
		      "null" );
  
    /*
     * set chill_extended_hsk option
     */
  
    resource_str = uGetParamString(Glob->prog_name,
				   "chill_extended_hsk",
				   CHILL_EXTENDED_HSK);
  
    if (uset_true_false_param(Glob->prog_name,
			      "read_params",
			      Glob->params_path_name,
			      resource_str,
			      &Glob->chill_extended_hsk,
			      "chill_extended_hsk"))
      tidy_and_exit(-1);
    
    
    /*
     * chill fields out
     */

    resource_str = uGetParamString(Glob->prog_name,
				   "chill_fields_out",
				   CHILL_FIELDS_OUT);

    strncpy (work_buf, resource_str, BUFSIZ);

    Glob->chill_fields_out =
      (char **) ucalloc2((ui32) Glob->nfields_out,
			 (ui32) 4, sizeof(char));
  
    token = strtok(work_buf, " \t\n\r");
  
    for (i = 0; i < Glob->nfields_out; i++) {
  
      if (token == NULL || strlen(token) != 2) {
	fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
	fprintf(stderr, "Decoding chill_fields_out: '%s'\n", resource_str);
	fprintf(stderr, "Field codes are 2 chars long\n");
	fprintf(stderr, "Edit parameters file '%s'\n",
		Glob->params_path_name);
	tidy_and_exit(-1);
      }
      
      if (!strcmp(token, "VR"))
	strcpy(token, "VE");
      
      if (strcmp(token, "IP") &&
	  strcmp(token, "VE") &&
	  strcmp(token, "DR") &&
	  strcmp(token, "DP") &&
	  strcmp(token, "R1") &&
	  strcmp(token, "R2") &&
	  strcmp(token, "RH")) {
	
	fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
	fprintf(stderr, "Decoding chill_fields_out: '%s'\n", resource_str);
	fprintf(stderr, "Supported field types: IP VE/VR DR DP R1 R2 RH\n");
	fprintf(stderr, "Edit parameters file '%s'\n",
		Glob->params_path_name);
	tidy_and_exit(-1);
	
      }
      
      strcpy(Glob->chill_fields_out[i], token);

      /*
       * override the output field positions
       */

      Glob->out_field_pos[i] = i;
      
      token = strtok((char *) NULL, " \t\n\r");
      
    } /* i */

    /*
     * chill scan types
     */
    
    Glob->nchill_scan_types = uGetParamLong(Glob->prog_name,
					    "nchill_scan_types",
					    NCHILL_SCAN_TYPES);
    
    resource_str = uGetParamString(Glob->prog_name,
				   "chill_scan_types",
				   CHILL_SCAN_TYPES);
    
    strncpy (work_buf, resource_str, BUFSIZ);
    
    Glob->chill_scan_types =
      (char **) ucalloc2((ui32) Glob->nchill_scan_types,
			 (ui32) 10, sizeof(char));
    
    token = strtok(work_buf, " \t\n\r");
    
    for (i = 0; i < Glob->nchill_scan_types; i++) {
      
      if (token == NULL) {
	fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
	fprintf(stderr, "Decoding chill_scan_types: '%s'\n", resource_str);
	fprintf(stderr, "Edit parameters file '%s'\n",
		Glob->params_path_name);
	tidy_and_exit(-1);
      }
      
      strcpy(Glob->chill_scan_types[i], token);
      token = strtok((char *) NULL, " \t\n\r");
      
    } /* i */
    
    /* 
     * chill start of volume indication
     */
    
    Glob->chill_vol_start_scan_type =
      uGetParamString(Glob->prog_name,
		      "chill_vol_start_scan_type",
		      CHILL_VOL_START_SCAN_TYPE);
    
    Glob->chill_vol_start_tilt =
      uGetParamLong(Glob->prog_name,
		    "chill_vol_start_tilt",
		    CHILL_VOL_START_TILT);
    

  } /* if (Glob->input_format == CHILL_FORMAT) */
    
  /*
   * set data storage mode
   */
  
  if (Glob->header_type == CHILL_HEADER ||
      Glob->header_type == ALENIA_HEADER) {
    
    Glob->data_field_by_field = TRUE;
    
  } else {
    
    Glob->data_field_by_field = FALSE;
    
  }

  /*
   * load up elevation table as required
   */
  
  resource_str = uGetParamString(Glob->prog_name, "use_elev_table", "false");
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->use_elev_table,
			    "use_elev_table"))
    tidy_and_exit(-1);

  if (Glob->use_elev_table) {

    Glob->nelev_table =
      uGetParamLong(Glob->prog_name, "nelev_table", NELEV_TABLE);
    
    resource_str = uGetParamString(Glob->prog_name,
				   "elev_table", ELEV_TABLE);

    elev_table_str = (char *) umalloc(strlen(resource_str) + 1);
    strcpy(elev_table_str, resource_str);
    
    Glob->elev_table = (double *) umalloc
      (Glob->nelev_table * sizeof(double));
    
    for (i = 0; i < Glob->nelev_table; i++) {
      
      if (i == 0)
	token = strtok(elev_table_str, " \t");
      else
	token = strtok((char *) NULL, " \t");
      
      if (token == NULL) {
	fprintf(stderr, "ERROR - %s:read_params.\n", Glob->prog_name);
	fprintf(stderr, "Decoding elev_table\n");
	fprintf(stderr, "Edit params file '%s'\n", Glob->params_path_name);
	tidy_and_exit(-1);
      }
      
      fprintf(stderr, "%s\n", token);

      
      if (sscanf(token, "%g", &elev) != 1) {
	fprintf(stderr, "ERROR - %s:read_params.\n", Glob->prog_name);
	fprintf(stderr, "Decoding elev_table\n");
	fprintf(stderr, "Decoding string '%s'\n", resource_str);
	fprintf(stderr, "Edit params file '%s'\n", Glob->params_path_name);
	tidy_and_exit(-1);
      }
      
      Glob->elev_table[i] = elev;

    } /* i */

    ufree(elev_table_str);

  } /* if (Glob->use_elev_table) */
  
}

