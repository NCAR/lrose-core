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

#include <cstring>
#include <math.h>
#include "polar_ingest.h"

static int load_field_props(char *prop,
			    char *default_str,
			    char **prop_array);

void read_params(void)

{
  
  char *resource_str;
  char *token, *tok_str;
  char *end_pt;

  si32 i;
  si32 ntokens;
  si32 tilt_num;

  double read_wait;

  /*
   * program instance
   */
  
  Glob->instance = uGetParamString(Glob->prog_name,
				   "instance", "Test");
  
  /*
   * malloc_debug_level
   */
  
  Glob->malloc_debug_level =
    uGetParamLong(Glob->prog_name, "malloc_debug_level", 0);
  
  /*
   * command line for starting polar2mdv
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "start_polar2mdv",
				 START_POLAR2MDV);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->start_polar2mdv,
			    "start_polar2mdv"))
    tidy_and_exit(-1);
  
  Glob->polar2mdv_command_line =
    uGetParamString(Glob->prog_name,
		    "polar2mdv_command_line",
		    POLAR2MDV_COMMAND_LINE);
  
  /*
   * set default print intervals
   */
  
  Glob->header_interval = uGetParamLong(Glob->prog_name,
					"header_interval",
					HEADER_INTERVAL);
  
  Glob->summary_interval = uGetParamLong(Glob->prog_name,
					 "summary_interval",
					 SUMMARY_INTERVAL);
  
  /*
   * shared memory key
   */
  
  Glob->shmem_key = (key_t) uGetParamLong(Glob->prog_name,
					  "shmem_key",
					  SHMEM_KEY);
  
  /*
   * shared rdi message queue options
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "write_rdi_mmq",
				 WRITE_RDI_MMQ);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->write_rdi_mmq, "write_rdi_mmq"))
    tidy_and_exit(-1);
  
  Glob->rdi_mmq_key = (key_t) uGetParamLong(Glob->prog_name,
					    "rdi_mmq_key",
					    RDI_MMQ_KEY);
  
  /*
   * number of beams in rotating buffer
   */
  
  Glob->nbeams_buffer = uGetParamLong(Glob->prog_name,
				      "nbeams_buffer", NBEAMS_BUFFER);
  
  /*
   * device params
   */
  
  Glob->tape_name = uGetParamString(Glob->prog_name,
				    "tape_name", TAPE_NAME);
  
  Glob->tcpip_host = uGetParamString(Glob->prog_name,
				     "tcpip_host", TCPIP_HOST);
  
  if (!strcmp(Glob->tcpip_host, "local")) {
    Glob->tcpip_host = PORThostname();
  }

  Glob->tcpip_port = uGetParamLong(Glob->prog_name,
				   "tcpip_port", TCPIP_PORT);
  
  Glob->udp_port = uGetParamLong(Glob->prog_name,
				 "udp_port", UDP_PORT);
  
  Glob->fmq_path = uGetParamString(Glob->prog_name,
				   "fmq_path", FMQ_PATH);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "fmq_seek_to_end",
				 FMQ_SEEK_TO_END);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->fmq_seek_to_end, "FMQ_SEEK_TO_END"))
    tidy_and_exit(-1);
  
  Glob->rdi_mmq_key = (key_t) uGetParamLong(Glob->prog_name,
					    "rdi_mmq_key",
					    RDI_MMQ_KEY);
  
  /*
   * miscellaneous
   */

  Glob->noise_dbz_at_100km = uGetParamDouble(Glob->prog_name,
                                             "noise_dbz_at_100km",
                                             NOISE_DBZ_AT_100KM);
  
  /*
   * end_of_volume decision
   */
  
  resource_str = uGetParamString(Glob->prog_name,
					    "end_of_vol_decision",
					    END_OF_VOL_DECISION);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->end_of_vol_decision,
			"end_of_given_tilt", END_OF_GIVEN_TILT,
			"end_of_vol_flag", END_OF_VOL_FLAG,
			"end_of_vol_decision"))
    tidy_and_exit(-1);
  
  /*
   * last tilt in volume - used to detect end-of-volume
   */
  
  Glob->last_tilt_in_vol = uGetParamLong(Glob->prog_name,
					 "last_tilt_in_vol",
					 LAST_TILT_IN_VOL);
  
  /*
   * header type
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				    "header_type", HEADER_TYPE);
  
  if (uset_quad_param(Glob->prog_name,
		      "read_params",
		      Glob->params_path_name,
		      resource_str, &Glob->header_type,
		      "lincoln", LINCOLN_HEADER,
		      "rp7", RP7_HEADER,
		      "bprp", BPRP_HEADER,
		      "gate_data", GATE_DATA_HEADER,
		      "header_type"))
    tidy_and_exit(-1);
  
  /*
   * set surveillance mode
   */
  
  resource_str = uGetParamString(Glob->prog_name,
					  "surveillance_mode",
					  SURVEILLANCE_MODE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->surveillance_mode, "surveillance_mode"))
    tidy_and_exit(-1);

  /*
   * set rhi mode
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "rhi_mode",
				 RHI_MODE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->rhi_mode, "rhi_mode"))
    tidy_and_exit(-1);
  
  /*
   * set sector mode
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				    "sector_mode",
				    SECTOR_MODE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->sector_mode, "sector_mode"))
    tidy_and_exit(-1);
  
  /*
   * tape wait interval
   */

  read_wait = uGetParamDouble(Glob->prog_name,
			      "tape_read_wait",
			      TAPE_READ_WAIT);

  if (read_wait > MAX_TAPE_READ_WAIT) {
    read_wait = MAX_TAPE_READ_WAIT;
  }

  Glob->tape_read_wait = (si32) (read_wait * 1000000.0 + 0.5);
  
  /*
   * fmq wait interval
   */

  read_wait = uGetParamDouble(Glob->prog_name,
			      "fmq_read_wait",
			      FMQ_READ_WAIT);

  Glob->fmq_read_wait = (si32) (read_wait * 1000000.0 + 0.5);
  
  /*
   * radar params
   */
  
  Glob->note = uGetParamString(Glob->prog_name, "note", NOTE);
  
  Glob->radar_id = uGetParamLong(Glob->prog_name, "radar_id", RADAR_ID);
  
  Glob->radar_name = uGetParamString(Glob->prog_name, "radar_name",
                                     RADAR_NAME);
  
  /*
   * hemisphere descriptions
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				      "ns_hemisphere", NS_HEMISPHERE);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->ns_factor,
			"north", 1,
			"south", -1,
			"ns_hemisphere"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				      "ew_hemisphere", EW_HEMISPHERE);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->ew_factor,
			"east", 1,
			"west", -1,
			"we_hemisphere"))
    tidy_and_exit(-1);
  
  Glob->samples_per_beam = uGetParamLong(Glob->prog_name,
					 "samples_per_beam",
					 SAMPLES_PER_BEAM);
  
  Glob->ngates_out = uGetParamLong(Glob->prog_name,
				   "ngates_out",
				   NGATES_OUT);
  
  Glob->ngates_dropped = uGetParamLong(Glob->prog_name,
				       "ngates_dropped",
				       NGATES_DROPPED);
  
  /*
   * radar fields
   */
  
  Glob->nfields_in = uGetParamLong(Glob->prog_name, "nfields_in",
				   NFIELDS_IN);
  
  /*
   * field names
   */

  Glob->field_name = (char **)
    umalloc((ui32) (Glob->nfields_in * sizeof(char *)));
  
  if (load_field_props("field_names",
		       FIELD_NAMES,
		       Glob->field_name))
    tidy_and_exit(-1);

  /*
   * field units
   */
  
  Glob->field_units = (char **)
    umalloc((ui32) (Glob->nfields_in * sizeof(char *)));
  
  if (load_field_props("field_units",
		       FIELD_UNITS,
		       Glob->field_units))
    tidy_and_exit(-1);

  /*
   * field transforms
   */
  
  Glob->field_transform = (char **)
    umalloc((ui32) (Glob->nfields_in * sizeof(char *)));
  
  if (load_field_props("field_transform",
		       FIELD_TRANSFORM,
		       Glob->field_transform))
    tidy_and_exit(-1);

  /*
   * position of dbz field in data stream
   */
  
  Glob->dbz_field_pos = uGetParamLong(Glob->prog_name,
                                      "dbz_field_pos",
				      DBZ_FIELD_POS);
  
  /*
   * position of flag byte field in data stream
   * flag byte has clutter and second-trip bits set
   */
  
  Glob->flag_field_pos = uGetParamLong(Glob->prog_name,
				       "flag_field_pos",
				       FLAG_FIELD_POS);

  /*
   * set modes to remove clutter or second trip based on 
   * flags set in the data
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "apply_flags", APPLY_FLAGS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->apply_flags,
			    "apply_flags"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "use_bit_mask", USE_BIT_MASK);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->use_bit_mask,
			    "use_bit_mask"))
    tidy_and_exit(-1);
  
  Glob->flag_check_val = uGetParamLong(Glob->prog_name,
				       "flag_check_val", FLAG_CHECK_VAL);
    
  Glob->flag_value_min = uGetParamLong(Glob->prog_name,
				       "flag_value_min", FLAG_VALUE_MIN);
    
  Glob->flag_value_max = uGetParamLong(Glob->prog_name,
				       "flag_value_max", FLAG_VALUE_MAX);
    
  /*
   * set check_gate_spacing mode
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
   * time correction (secs)
   */

  Glob->time_correction =
    uGetParamLong(Glob->prog_name, "time_correction", 0);
	    
  /*
   * time override?
   */
  
  resource_str = uGetParamString(Glob->prog_name, "time_override", "false");
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->time_override, "time_override"))
    tidy_and_exit(-1);
  
  /*
   * set check_elev_limits mode
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_elev_limits",
				 CHECK_ELEV_LIMITS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->check_elev_limits, "check_elev_limits"))
    tidy_and_exit(-1);
  
  Glob->min_elevation = uGetParamDouble(Glob->prog_name,
					"min_elevation",
					MIN_ELEVATION);
	    
  Glob->max_elevation = uGetParamDouble(Glob->prog_name,
					"max_elevation",
					MAX_ELEVATION);
	    
  /*
   * set mode for checking tilt numbers
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_tilt_number",
				 CHECK_TILT_NUMBER);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->check_tilt_number, "check_tilt_number"))
    tidy_and_exit(-1);

  resource_str = uGetParamString(Glob->prog_name,
				 "tilt_numbers",
				 TILT_NUMBERS);
  
  /*
   * get number of tilts specified, and the max tilt number
   */
  
  tok_str = (char *) umalloc ((ui32) (strlen(resource_str) + 1));
  strcpy(tok_str, resource_str);
  
  /*
   * get number of tilts
   */

  Glob->max_tilt_number = 0;
  ntokens = 0;
  token = strtok(tok_str, " ,\n\t");

  while (token != NULL) {
    tilt_num = strtol(token, &end_pt, 10);
    if (Glob->max_tilt_number < tilt_num)
      Glob->max_tilt_number = tilt_num;
    ntokens++;
    token = strtok((char *) NULL, " ,\n\t");
  } /* while */

  Glob->n_tilts = ntokens;

  Glob->tilt_numbers = (si32 *) umalloc
    ((ui32) (Glob->n_tilts * sizeof(si32)));
  
  Glob->tilt_flags = (si32 *) ucalloc
    ((ui32) (Glob->max_tilt_number + 1), (ui32) sizeof(si32));

  /*
   * set tilt_numbers and tilt_flags array
   */

  strcpy(tok_str, resource_str);
  token = strtok(tok_str, " ,\n\t");
  
  for (i = 0; i < Glob->n_tilts; i++) {
    tilt_num = strtol(token, &end_pt, 10);
    Glob->tilt_numbers[i] = tilt_num;
    Glob->tilt_flags[tilt_num] = TRUE;
    token = strtok((char *) NULL, " ,\n\t");
  } /* i */

  ufree((char *) tok_str);
  
}

/***************************************************************
 * load_field_props()
 *
 * returns 0 on success, -1 on failure
 */

static int load_field_props(char *prop,
			    char *default_str,
			    char **prop_array)

{

  char *field_prop_str;
  char *tmp_str;
  char *token;
  si32 i;

  /*
   * field names
   */
  
  tmp_str = uGetParamString(Glob->prog_name, prop, default_str);

  field_prop_str = (char *) umalloc
    ((ui32) (strlen(tmp_str) + 1));
  
  strcpy(field_prop_str, tmp_str);
  
  for (i = 0; i < Glob->nfields_in; i++) {
    
    if (i == 0)
      token = strtok(field_prop_str, ",");
    else
      token = strtok((char *) NULL, ",");
    
    if (token == NULL) {
      fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
      fprintf(stderr, "Decoding %s string '%s'\n", prop, tmp_str);
      fprintf(stderr, "Edit parameters file '%s'\n",
              Glob->params_path_name);
      return (-1);
    }
    
    prop_array[i] = token;
    
  } /* i */

  return (0);
  
}
