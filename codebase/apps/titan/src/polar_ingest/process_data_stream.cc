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
/***************************************************************************
 * process_data_stream.c
 *
 * reads in the beam data, processes it and places is in shared memory
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar_ingest.h"
#include <dataport/bigend.h>

#define QUIT_CHECK_INTERVAL 50

/*
 * file scope
 */

static rdata_shmem_header_t *Shdr;

/*
 * prototypes
 */

static void check_buffer_overflow(si32 incoming_beam);
static int check_sem(void);
static int check_valid(int *scan_mode);
static void clear_shmem_flag(void);
static void initialize(si32 incoming_beam);
static void print_beam(void);
static int read_beam(void);

/*
 * main routine
 */

void process_data_stream(void)

{

  int forever = TRUE;
  int no_valid_beam_yet = TRUE;
  int beam_valid, prev_beam_valid;
  int shmem_flag_cleared = FALSE;
  int end_of_vol;
  int scan_mode;
  si32 incoming_beam, outgoing_beam;

  /*
   * enter loop
   */

  while (forever) {
    
    /*
     * check the polar2mdv quit semaphore,
     * and quit if it is set
     */

    if (check_sem())
      tidy_and_exit(0);
  
    /*
     * read in a beam
     */

    if (read_beam())
      return;

    /*
     * check for validity of beam
     */

    beam_valid = check_valid(&scan_mode);

    /*
     * for valid beams, print out as required
     */

    if (beam_valid)
      print_beam();

    if (no_valid_beam_yet) {

      /*
       * looking for the first valid beam to initialize
       */

      if (beam_valid) {

	incoming_beam = 0;
	initialize(incoming_beam);
	prev_beam_valid = TRUE;
	no_valid_beam_yet = FALSE;

      } /* if (beam_valid) */

    } else {

      /*
       * previous valid beam, so initialization has been done
       */

      if (beam_valid) {

	/*
	 * beam valid
	 */

	if (prev_beam_valid) {

	  /*
	   * prev beam valid, so read in the new beam and
	   * set the flags on the previous one and write it out
	   */

	  outgoing_beam = incoming_beam;
	  
	  incoming_beam =
	    (outgoing_beam + 1) % Glob->nbeams_buffer;
	  
	  /*
	   * check for buffer overflow
	   */

	  check_buffer_overflow(incoming_beam);

	  load_beam(incoming_beam, TRUE);
	  
	  set_beam_flags(outgoing_beam, incoming_beam,
			 beam_valid, prev_beam_valid, scan_mode,
			 &end_of_vol);
	  
	  Shdr->last_beam_written = outgoing_beam;

	} else {

	  /*
	   * prev beam not valid, so the outgoing beam will
	   * already have been written out. Therefore, just
	   * load in incoming beam
	   */

	  load_beam(incoming_beam, TRUE);
	  
	  set_beam_flags(outgoing_beam, incoming_beam,
			 beam_valid, prev_beam_valid, scan_mode,
			 &end_of_vol);

	  /*
	   * if an end_of_vol was found, set the late_end_of_vol flag
	   * to indicate this
	   */

	  if (end_of_vol) {
	    Shdr->late_end_of_vol = TRUE;
	  }
	  
	} /* if (prev_beam_valid) */

	/*
	 * clear the not-ready semaphore so that the client may begin
	 * reading the shared memory buffer
	 */

	if (!shmem_flag_cleared) {
	  clear_shmem_flag();
	  shmem_flag_cleared = TRUE;
	}
	
	prev_beam_valid = TRUE;

      } else {

	/*
	 * beam not valid
	 */

	if (prev_beam_valid) {

	  /*
	   * Load in the invalid beam, and set the
	   * flags on the previous beam, which was valid.
	   * Then write out prev beam
	   */

	  outgoing_beam = incoming_beam;

	  incoming_beam =
	    (outgoing_beam + 1) % Glob->nbeams_buffer;
	  
	  /*
	   * check for buffer overflow
	   */

	  check_buffer_overflow(incoming_beam);
	  
	  load_beam(incoming_beam, FALSE);
	  
	  set_beam_flags(outgoing_beam, incoming_beam,
			 beam_valid, prev_beam_valid, scan_mode,
			 &end_of_vol);
	  
	  Shdr->last_beam_written = outgoing_beam;

	} /* if (prev_beam_valid) */

	prev_beam_valid = FALSE;

      } /* if (beam_valid) */

    } /* if (no_valid_beam_yet) */
    
  } /* while (forever) */

}

/***********************************************************************
 * check_buffer_overflow()
 *
 * checks for buffer overflow
 *
 */

static void check_buffer_overflow(si32 incoming_beam)

{

  if (!Glob->monitor) {

    switch (Glob->device) {

    case LL_UDP_DEVICE:
    case NCAR_UDP_DEVICE:

      /*
       * cannot wait - message
       */

      if (incoming_beam == Shdr->last_beam_used) {

	fprintf(stderr, 
		"WARNING - %s:process_data_stream:check_buffer_overflow\n",
		Glob->prog_name);
	fprintf(stderr, "Beam buffer overflow.\n");
	fprintf(stderr, "%s%s\n",
		"Client process (e.g. polar2mdv) ",
		"down or not keeping pace with data rate.");
	
      } /* if (incoming_beam == Shdr->last_beam_used) */
      
      break;

    case TAPE_DEVICE:
    case TCPIP_DEVICE: 

      /*
       * can wait
       */

      if (usem_check(Glob->sem_id, POLAR2MDV_ACTIVE_SEM)) {

	while (incoming_beam == Shdr->last_beam_used) {
	  
	  if (Glob->debug == TRUE) {
	    fprintf(stderr,
		    "MESSAGE - %s:process_data_stream\n", Glob->prog_name);
	    fprintf(stderr,
		    "Tape operation - waiting for client to catch up.\n");
	  }
	  
	  uusleep(1000000);
	  
	  if (Glob->semaphores_attached)
	    if (usem_check(Glob->sem_id, POLAR2MDV_QUIT_SEM))
	      tidy_and_exit(0);
	  
	}

      } /* if (usem_check(Glob->sem_id ... */

      break;

    } /* switch */

  } /* if (!Glob->monitor) */
      
}

/*************************************************************
 * check_sem()
 *
 * once in a while, check the polar2mdv quit semaphore.
 * Return 1 if set, 0 if not.
 */

static int check_sem(void)

{

  static int loop_count = 0;

  if (loop_count > QUIT_CHECK_INTERVAL) {
    
    if (Glob->semaphores_attached)
      if (usem_check(Glob->sem_id, POLAR2MDV_QUIT_SEM))
	return (1);
    
    loop_count = 0;
    
  }

  loop_count++;
  return (0);

}

/*************************************************************
 * check_valid()
 *
 * checks beam validity, and sets flag
 */

static int check_valid(int *scan_mode_p)

{

  ui08 *data;
  int beam_valid;
  si32 tilt_number;
  double gate_spacing;
  double elevation;
  static double cf = 360.0 / 65536.0;
  rp7_params_t *rp7_params;
  ll_params_t *ll_params;
  bprp_params_t *bprp_params;
  bprp_data_t *bprp_data;
  gate_data_radar_params_t *gate_rparams;
  gate_data_field_params_t *gate_fparams;
  gate_data_beam_header_t *gate_header;

  /*
   * get the gate_spacing and scan mode
   */

  beam_valid = TRUE;
  switch (Glob->header_type) {

  case LINCOLN_HEADER:

    get_lincoln_ptrs(&ll_params, &data);
    
    tilt_number = ll_params->tilt_num;
    gate_spacing = (double) ll_params->range_seg[0].gate_spacing;
    elevation = (double) ll_params->elevation / 100.0;
      
    if (ll_params->scan_mode == LL_SURVEILLANCE_MODE) {
      *scan_mode_p = DIX_PPI_MODE;
    } else if (ll_params->scan_mode == LL_SECTOR_MODE) {
      *scan_mode_p = DIX_SECTOR_MODE;
    } else if (ll_params->scan_mode == LL_RHI_MODE) {
      *scan_mode_p = DIX_RHI_MODE;
    } else {
      *scan_mode_p = DIX_UNKNOWN_MODE;
    }

    break;
      
  case RP7_HEADER:

    get_rp7_ptrs(&rp7_params, &data);

    tilt_number = rp7_params->tilt_num;
    gate_spacing = (double) rp7_params->gate_spacing;
    elevation = (double) rp7_params->elevation * cf;
    
    if (rp7_params->scan_mode == RP7_SURVEILLANCE_MODE) {
      *scan_mode_p = DIX_PPI_MODE;
    } else if (rp7_params->scan_mode == RP7_SECTOR_MODE) {
      *scan_mode_p = DIX_SECTOR_MODE;
    } else if (rp7_params->scan_mode == RP7_RHI_MODE) {
      *scan_mode_p = DIX_RHI_MODE;
    } else {
      *scan_mode_p = DIX_UNKNOWN_MODE;
    }

    break;
    
  case BPRP_HEADER:

    get_bprp_ptrs(&bprp_params, &bprp_data);

    tilt_number = (si32) bprp_params->tilt_num;
    gate_spacing = (double) BPRP_GATE_SPACING * 1000000.0 + 05;  
    elevation = bprp_params->elevation;
    *scan_mode_p = DIX_PPI_MODE;

    break;

  case GATE_DATA_HEADER:
    
    get_gate_data_ptrs(&gate_rparams, &gate_fparams,
		       &gate_header, &data);
    
    tilt_number = gate_header->tilt_num;
    gate_spacing = (double) gate_rparams->gate_spacing / 1000.0;
    elevation = (double) gate_header->elevation / 1000000.0;

    if (gate_rparams->scan_mode == GATE_DATA_SURVEILLANCE_MODE) {
      *scan_mode_p = DIX_PPI_MODE;
    } else if (gate_rparams->scan_mode == GATE_DATA_SECTOR_MODE) {
      *scan_mode_p = DIX_SECTOR_MODE;
    } else if (gate_rparams->scan_mode == GATE_DATA_RHI_MODE) {
      *scan_mode_p = DIX_RHI_MODE;
    } else {
      *scan_mode_p = DIX_UNKNOWN_MODE;
    }

  } /* switch (Glob->header_type) */
      
  /*
   * If required, check that the tilt_number is valid.
   */
      
  if (Glob->check_tilt_number) {
    if (tilt_number > Glob->max_tilt_number) {
      beam_valid = FALSE;
    } else {
      if (!Glob->tilt_flags[tilt_number])
	beam_valid = FALSE;
    }
  }
      
  /*
   * If required, check that the gate_spacing is correct.
   * If not, flag beam as invalid
   */
      
  if (Glob->check_gate_spacing) {
    if (fabs(gate_spacing - Glob->target_gate_spacing) > 0.5) {
      beam_valid = FALSE;
    }
  }
      
  /*
   * If required, check that the elevation is within limits
   * If not, flag beam as invalid
   */
      
  if (Glob->check_elev_limits) {
      
    if (elevation < Glob->min_elevation ||
	elevation > Glob->max_elevation) {
      beam_valid = FALSE;
    }
    
  } /* if (Glob->check_elev_limits) */
      
  /*
   * Check that the scan mode is acceptable. If not, 
   * flag beam as invalid
   */

  switch (*scan_mode_p) {

  case DIX_RHI_MODE:
    if (!Glob->rhi_mode) {
      beam_valid = FALSE;
    }
    break;
    
  case DIX_SECTOR_MODE:
    if (!Glob->sector_mode) {
      beam_valid = FALSE;
    }
    break;
    
  case DIX_PPI_MODE:
  default:
    if (!Glob->surveillance_mode) {
      beam_valid = FALSE;
    }
    
  } /* switch */

  return (beam_valid);

}

/**************************************************************
 * clear_shmem_flag()
 *
 * clear the not-ready semaphore so that the client may begin
 * reading the shared memory buffer
 */

static void clear_shmem_flag(void)

{

  if (usem_clear(Glob->sem_id, POLAR_INGEST_NOT_READY_SEM) != 0) {
    fprintf(stderr, "ERROR - %s.\n", Glob->prog_name);
    fprintf(stderr, "Clearing not-ready semaphore");
    tidy_and_exit(-1);
  }
    
  if (Glob->debug)
    fprintf(stderr,
	    "%s:process_data_stream - cleared not_ready sem\n",
	    Glob->prog_name);
  
}
	
/***************************************************************
 * initialize()
 *
 * initialize the shared memory and flags
 */

static void initialize(si32 incoming_beam)

{

  /*
   * set up shared memory
   */

  setup_shmem();
  Shdr = Glob->shmem_header;
      
  /*
   * set up the first beam buffer
   */

  load_beam(incoming_beam, TRUE);

}

/****************************************************************
 * print_beam()
 */

static void print_beam(void)

{

  static int header_count = 0;
  static int summary_count = 0;

  /*
   * print out as requested
   */

  if (Glob->header_print) {
    
    if (header_count == 0)
      print_header();
	
    header_count++;
	
    if (header_count == Glob->header_interval)
      header_count = 0;
	
   } /* if (Glob->header_print) */
      
  if (Glob->summary_print) {
    
    if (summary_count == 0)
      
      print_summary();
    
    summary_count++;
    
    if (summary_count == Glob->summary_interval)
      summary_count = 0;
    
  } /* if (Glob->summary_print) */

}
      
/*******************************************************
 * read_beam()
 *
 * read in a beam from the correct device
 *
 * returns 0 on success, 1 on failure
 */
 
static int read_beam(void)

{
   
  switch (Glob->device) {

  case FMQ_DEVICE:
    
    {
      
      int read_size;
      char *beam_buffer;
      ui08 *ll_data;
      ll_params_t *ll_params;
      
      while ((read_size = read_fmq_beam(&beam_buffer)) <= 0) {
      }
      
      if (Glob->header_type == LINCOLN_HEADER) {

	set_lincoln_ptrs(beam_buffer);

	/*
	 * make sure year is in full resolution
	 */
	
	get_lincoln_ptrs(&ll_params, &ll_data);
	
	if (ll_params->year < 1900) {
	  if (ll_params->year > 70) {
	    ll_params->year += 1900;
	  } else {
	    ll_params->year += 2000;
	  }
	}

      } else {
	fprintf(stderr, "ERROR: %s\n", Glob->prog_name);
	fprintf(stderr, "Only lincoln format "
		"supported for fmq device\n");
	tidy_and_exit (-1);

      } /* if (Glob->header_type == LINCOLN_HEADER) */
      
    }

  break;
      
  case TAPE_DEVICE:

    {

      int read_size;
      char *beam_buffer;
      
      switch (Glob->header_type) {
	
      case BPRP_HEADER:

        if (read_tape_bprp()) {
	  return (1);
	}
	break;

      default:          

	read_size = read_tape_beam(&beam_buffer);
      
	/*
	 * if logical record size is zero, the end of the
	 * tape has been reached, so return.
	 */
	
	if (read_size == 0)
	  return (1);

      }
	
      if (Glob->header_type == LINCOLN_HEADER) {
	set_lincoln_ptrs(beam_buffer);
      } else if (Glob->header_type == RP7_HEADER) {
	set_rp7_ptrs(beam_buffer);
      } else if (Glob->header_type == BPRP_HEADER) {
      } else {
	fprintf(stderr, "ERROR: %s\n", Glob->prog_name);
	fprintf(stderr, "Only lincoln and rp7 formats "
		"supported for tape device\n");
	tidy_and_exit (-1);
      } /* if (Glob->header_type == LINCOLN_HEADER) */
      
    }
    
    break;
    
  case LL_UDP_DEVICE:
  case NCAR_UDP_DEVICE:
    
    {  

      char *beam_buffer;
      ui08 *ll_data;
      ll_params_t *ll_params;
      
      if (Glob->header_type == LINCOLN_HEADER) {
	
	if (Glob->device == LL_UDP_DEVICE) {
	  beam_buffer = read_ll_udp_beam();
	} else {
	  beam_buffer = read_ncar_udp_beam();
	}
	
      } else {
	
	fprintf(stderr, "ERROR: %s\n", Glob->prog_name);
	fprintf(stderr, "Only lincoln format supported for udp device\n");
	tidy_and_exit (-1);
	
      } /* if (Glob->header_type == LINCOLN_HEADER) */
      
      /*
       * set buffer pointers
       */
      
      set_lincoln_ptrs(beam_buffer);
      
      /*
       * make sure year is in full resolution
       */
      
      get_lincoln_ptrs(&ll_params, &ll_data);
      
      if (ll_params->year < 1900) {
	if (ll_params->year > 70) {
	  ll_params->year += 1900;
	} else {
	  ll_params->year += 2000;
	}
      }

    }
      
    break;
	
  case TCPIP_DEVICE:
    
    {

      ui08 *gate_data;
      gate_data_beam_header_t *gate_header;
      gate_data_radar_params_t *gate_rparams;
      gate_data_field_params_t *gate_fparams;
      
      if (Glob->header_type == GATE_DATA_HEADER) {
	
	read_tcpip_beam(&gate_rparams, &gate_fparams,
			&gate_header, &gate_data);
	set_gate_data_ptrs(gate_rparams, gate_fparams,
			   gate_header, gate_data);

      } else {
	
	fprintf(stderr, "ERROR: %s\n", Glob->prog_name);
	fprintf(stderr, "Only gate_data format supported for tcpip device\n");
	tidy_and_exit (-1);
	
      } 
      
    }
    
    break;
    
  } /* switch (Glob->device) */

  return (0);
  
}


