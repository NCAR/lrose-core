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
 * read_params.c: reads the parameters, loads up the globals
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/
  
#include "polar2mdv.h"
  
void read_params(void)

{
  
  char *resource_str;
  char *start_pt, *end_pt;
  int i;
  
  /*
   * get globals from the parameters
   */
  
  /*
   * directories and hosts
   */
  
  Glob->rdata_dir =
    uGetParamString(Glob->prog_name, "rdata_dir", RDATA_DIR);
  
  Glob->local_tmp_dir =
    uGetParamString(Glob->prog_name, "local_tmp_dir", ".");
  
  Glob->hostname =
    uGetParamString(Glob->prog_name, "hostname", HOSTNAME);
  
  Glob->output_file_ext =
    uGetParamString(Glob->prog_name, "output_file_ext", OUTPUT_FILE_EXT);
  
  /*
   * server mapper
   */
  
  Glob->servmap_host1 = uGetParamString(Glob->prog_name,
					"servmap_host1",
					SERVMAP_HOST1_STR);
  
  Glob->servmap_host2 = uGetParamString(Glob->prog_name,
					"servmap_host2",
					SERVMAP_HOST2_STR);
  
  Glob->info = uGetParamString(Glob->prog_name,
			       "info", INFO);
  
  Glob->instance = uGetParamString(Glob->prog_name,
				   "instance", INSTANCE);
  
  /*
   * file names
   */
  
  Glob->clutter_table_path =
    uGetParamString(Glob->prog_name, "clutter_table_path", CLUTTER_TABLE_PATH);
  
  Glob->rc_table_path =
    uGetParamString(Glob->prog_name, "rc_table_path", RC_TABLE_PATH);
  
  /*
   * shared memory
   */
  
  Glob->shmem_key = (key_t) uGetParamLong(Glob->prog_name,
					  "shmem_key",
					  SHMEM_KEY);
  
  Glob->shmem_read_wait =
    (long) (uGetParamDouble(Glob->prog_name,
			    "shmem_read_wait",
			    SHMEM_READ_WAIT) * 1000000.0 + 0.5);

  /*
   * offset to be added to azimuth values before truncating, so that
   * they round to the nearest integer. Normally set to 0.5, but
   * sometimes a different number is needed because of the azimuth
   * values the radar is sending
   */

  Glob->azimuth_offset = uGetParamDouble(Glob->prog_name, "azimuth_offset",
                                        AZIMUTH_OFFSET);
  
  Glob->sn_threshold = uGetParamDouble(Glob->prog_name,
				       "sn_threshold",
				       SN_THRESHOLD);
  
  Glob->min_valid_run = uGetParamLong(Glob->prog_name,
				      "min_valid_run",
				      MIN_VALID_RUN);
  
  /*
   * fields
   */
  
  Glob->nfields_processed = uGetParamLong(Glob->prog_name,
					  "nfields_processed",
					  NFIELDS_PROCESSED);
  
  Glob->field_positions = (si32 *)
    umalloc((ui32) (Glob->nfields_processed * sizeof(long)));
  
  resource_str =
    uGetParamString(Glob->prog_name, "field_positions", FIELD_POSITIONS);
  
  end_pt = resource_str;
  
  for (i = 0; i < Glob->nfields_processed; i++) {
    
    start_pt = end_pt;
    errno = 0;
    Glob->field_positions[i] = strtol(start_pt, &end_pt, 10);
    
    if (errno != 0) {
      fprintf(stderr, "ERROR - %s:read_params.\n", Glob->prog_name);
      fprintf(stderr, "Decoding field position string '%s'\n",
	      resource_str);
      fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
      perror(start_pt);
      tidy_and_exit(-1);
    }
    
  } /* i */
  
  Glob->max_packet_length = uGetParamLong(Glob->prog_name,
					  "max_packet_length",
					  MAX_PACKET_LENGTH);
  
  Glob->check_clients_interval = (double)
    uGetParamDouble(Glob->prog_name,
		    "check_clients_interval",
		    CHECK_CLIENTS_INTERVAL);
  
  Glob->port = (int)
    uGetParamLong(Glob->prog_name, "port", PORT);
  
  /*
   * age at end of volume
   */
  
  Glob->age_at_end_of_volume =
    uGetParamLong(Glob->prog_name,
		  "age_at_end_of_volume",
		  AGE_AT_END_OF_VOLUME);
  
  /*
   * tilt and initial timestamp to use for timestamping output file
   */
  Glob->timestamp_at_start_of_tilt = uGetParamLong(Glob->prog_name,
                                     "timestamp_at_start_of_tilt",
                                     TIMESTAMP_AT_START_OF_TILT );
  Glob->volume_timestamp = -1;

  /*
   * set options
   */
  
  resource_str = uGetParamString(Glob->prog_name, "debug", DEBUG_STR);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->debug,
			    "debug"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				      "auto_mid_time",
				      AUTO_MID_TIME);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->auto_mid_time,
			    "auto_mid_time"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				      "override_table_latlon",
				      OVERRIDE_TABLE_LATLON);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->override_table_latlon,
			    "override_table_latlon"))
    tidy_and_exit(-1);
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "use_repeated_elevations",
		    USE_REPEATED_ELEVATIONS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->use_repeated_elevations,
			    "use_repeated_elevations"))
    tidy_and_exit(-1);
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "use_max_on_repeat",
		    USE_MAX_ON_REPEAT);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->use_max_on_repeat,
			    "use_max_on_repeat"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "run_length_encode",
				 RUN_LENGTH_ENCODE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->run_length_encode,
			    "output encoding"))
    tidy_and_exit(-1);
  
  resource_str =
    uGetParamString(Glob->prog_name, "remove_clutter", REMOVE_CLUTTER);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->remove_clutter,
			    "clutter removal"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "transmit_cart_data",
				 TRANSMIT_CART_DATA);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->transmit_cart_data,
			    "transmit cartesian packets"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_missing_beams",
				 CHECK_MISSING_BEAMS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->check_missing_beams,
			    "check missing beams"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "report_missing_beams",
				 REPORT_MISSING_BEAMS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->report_missing_beams,
			    "report missing beams"))
    tidy_and_exit(-1);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "swap_times_if_incorrect",
				 SWAP_TIMES_IF_INCORRECT);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->swap_times_if_incorrect,
			    "swap_times_if_incorrect"))
    tidy_and_exit(-1);
  
  /*
   * set max allowable number of missing beams
   */
  
  Glob->max_missing_beams = uGetParamLong(Glob->prog_name,
					  "max_missing_beams",
					  MAX_MISSING_BEAMS);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_sn",
				 CHECK_SN);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->check_sn,
			    "check_sn"))
    tidy_and_exit(-1);

  /*
   * set max vol interval
   */
  
  Glob->max_vol_duration = uGetParamLong(Glob->prog_name,
					 "max_vol_duration",
					 900);
  
  resource_str = uGetParamString(Glob->prog_name,
				 "check_sn",
				 CHECK_SN);
  
}

