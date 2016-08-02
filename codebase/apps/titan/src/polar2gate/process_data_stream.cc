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
/********************************************************************
 * process_data_stream.c
 *
 * reads in the beam data, processes it and writes it to socket
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * September 1992
 *
 *******************************************************************/

#include "polar2gate.h"
#include <dataport/bigend.h>

static int transmit(ui08 *data,
		    si32 len,
		    si32 product_id,
		    si32 seq_no);

void process_data_stream(void)

{

  ui08 *beam_buffer;
  ui08 *gate_params_pkt;
  ui08 *current_gate_data_pkt, *prev_gate_data_pkt;
  ui08 *tmp_gate_data_pkt;

  int forever = TRUE;
  int read_size;
  int prev_end_of_tilt;
  int params_initialized = FALSE;
  int header_count = 0;
  int summary_count = 0;
  int beam_valid, prev_beam_valid;
  int params_have_changed;

  si32 gate_params_pkt_seq_no = 0;
  si32 gate_data_pkt_seq_no = 0;
  si32 nbytes_params_pkt;
  si32 maxbytes_data_pkt;
  si32 nbytes_data_pkt;
  si32 gate_spacing;
  
  rp7_params_t *rp7_params;
  ll_params_t *ll_params;
  chill_params_t *ch_params;
  lass_params_t *lass_params;
  gate_data_radar_params_t *rparams;
  alenia_params_t *al_params;

  /*
   * allocate beam buffer
   */

  beam_buffer = (ui08 *)
    umalloc ((ui32) Glob->max_nbytes_beam_buffer);
  
  /*
   * allocate parameters packet
   */
  
  nbytes_params_pkt = sizeof(gate_data_radar_params_t) +
    Glob->nfields_out * sizeof(gate_data_field_params_t);

  gate_params_pkt = (ui08 *) umalloc ((ui32) nbytes_params_pkt);

  rparams = (gate_data_radar_params_t *) gate_params_pkt;

  /*
   * allocate the space for the data packet, including space for
   * storing the immediately previous one
   */

  maxbytes_data_pkt = sizeof(gate_data_beam_header_t) +
    Glob->nfields_out * Glob->ngates_out;

  current_gate_data_pkt = (ui08 *) umalloc ((ui32) maxbytes_data_pkt);
  prev_gate_data_pkt = (ui08 *) umalloc ((ui32) maxbytes_data_pkt);

  /*
   * enter loop
   */

  while (forever) {

    /*
     * read in a beam from the correct device
     */
    
    switch (Glob->device) {
      
    case TAPE_DEVICE:

      switch (Glob->input_format) {

      case NCAR_FORMAT:
	
	read_size = read_tape_ncar (beam_buffer);
	break;
	
      case CHILL_FORMAT:
	read_size = read_tape_chill (beam_buffer);
	break;
	
      default:
	fprintf(stderr,
		"Only chill and ncar formats "
		"supported for tape device\n");
	tidy_and_exit(-1);
	
      } /* switch */
      
      /*
       * if logical record size is zero, the end of the
       * tape has been reached, so return.
       */
      
      if (read_size == 0)
	return;
      
      break;
      
    case DISK_DEVICE:
      
      switch (Glob->input_format) {
	
      case LASS_FORMAT: /* I am still treating it as a tape
			 * device as it will be emulation of tape */
	read_size = read_disk_lass  (beam_buffer);
	break;
	
      case ALENIA_FORMAT:
	read_size = read_disk_alenia  (beam_buffer);
	break;
	
      default:
	fprintf(stderr,
		"Only lass and alenia formats supported for disk device\n");
	tidy_and_exit(-1);

      } /* switch (Glob->header_type) */

      break;

    default:
      
      fprintf(stderr, "Unknown device type: %d\n", Glob->device);
      tidy_and_exit(-1);
      
    } /* switch (Glob->device) */
    
    /*
     * check the gate spacing and scan mode to decide
     * whether beam is valid
     */
    
    beam_valid = TRUE;

    switch (Glob->header_type) {
      
    case LINCOLN_HEADER:
      
      ll_params = (ll_params_t *) beam_buffer;
      gate_spacing = ll_params->range_seg[0].gate_spacing;
      
      if (Glob->scan_mode == GATE_DATA_SECTOR_MODE)
	      beam_valid = FALSE;

      if (Glob->scan_mode == GATE_DATA_RHI_MODE &&
	      ll_params->scan_mode != LL_RHI_MODE)
	      beam_valid = FALSE;

      if (Glob->scan_mode == GATE_DATA_SURVEILLANCE_MODE &&
	      ll_params->scan_mode != LL_SURVEILLANCE_MODE)
	      beam_valid = FALSE;

      break;
      
    case RP7_HEADER:

      rp7_params = (rp7_params_t *) beam_buffer;
      gate_spacing = rp7_params->gate_spacing;
      
      if (Glob->scan_mode == GATE_DATA_SECTOR_MODE &&
	      rp7_params->scan_mode != RP7_SECTOR_MODE)
	      beam_valid = FALSE;

      if (Glob->scan_mode == GATE_DATA_RHI_MODE &&
	      rp7_params->scan_mode != RP7_RHI_MODE)
	      beam_valid = FALSE;

      if (Glob->scan_mode == GATE_DATA_SURVEILLANCE_MODE &&
	      rp7_params->scan_mode != RP7_SURVEILLANCE_MODE)
	      beam_valid = FALSE;
      
      if (Glob->nfields_in != rp7_params->nfields) {
	
	if (Glob->debug) {
	  fprintf(stderr, "WARNING - %s:process_data_stream\n",
		  Glob->prog_name);
	  fprintf(stderr,
		  "Number of input fields requested - %ld - incorrect.\n",
		  (long) Glob->nfields_in);
	  fprintf(stderr, "Number of fields in data is %d.\n",
		  rp7_params->nfields);
	} /*  (Glob->debug) */
	
	beam_valid = FALSE;
	
      } /* if (Glob->nfields_in > ... */
      
      break;    
      
  
    case LASS_HEADER:
      
      lass_params = (lass_params_t *) beam_buffer;
      
      gate_spacing = lass_params->volSummary.gatewid;
      
      if (Glob->scan_mode == GATE_DATA_SECTOR_MODE &&
	  lass_params->volSummary.sweep_type != GATE_DATA_SECTOR_MODE)
	beam_valid = FALSE;
      
      if (Glob->scan_mode == GATE_DATA_RHI_MODE &&
	  lass_params->volSummary.sweep_type != GATE_DATA_RHI_MODE)
	beam_valid = FALSE;
      
      if (Glob->scan_mode == GATE_DATA_SURVEILLANCE_MODE &&
	  lass_params->volSummary.sweep_type != GATE_DATA_SURVEILLANCE_MODE)
	beam_valid = FALSE;
      
      break;
      
      
    case ALENIA_HEADER:
      
      al_params = (alenia_params_t *) beam_buffer;
      gate_spacing = al_params->gate_spacing;
      break;
      
      
    case CHILL_HEADER:
      
      ch_params = (chill_params_t *) beam_buffer;
      
      gate_spacing = ch_params->gate_spacing;
      
      if (Glob->scan_mode == GATE_DATA_SECTOR_MODE &&
	  ch_params->scan_mode != GATE_DATA_SECTOR_MODE)
	beam_valid = FALSE;
      
      if (Glob->scan_mode == GATE_DATA_RHI_MODE &&
	  ch_params->scan_mode != GATE_DATA_RHI_MODE)
	beam_valid = FALSE;
      
      if (Glob->scan_mode == GATE_DATA_SURVEILLANCE_MODE &&
	  ch_params->scan_mode != GATE_DATA_SURVEILLANCE_MODE)
	beam_valid = FALSE;
      
      break;
      
    } /* switch */
    
    if (Glob->check_gate_spacing)
      if (gate_spacing != Glob->target_gate_spacing)
	beam_valid = FALSE;
    
    /*
     * for valid beams, print out as required
     */
    
    if (beam_valid) {
      
      /*
       * print header if requested
       */
      
      if (Glob->header_print) {
	if (header_count == 0)
	  print_header(beam_buffer);
	header_count++;
	if (header_count == Glob->header_interval)
	  header_count = 0;
      }
      
      /*
       * print summary if requested
       */
      
      if (Glob->summary_print) {
	if (summary_count == 0)
	  print_summary(beam_buffer);
	summary_count++;
	if (summary_count == Glob->summary_interval)
	  summary_count = 0;
      }
      
    } /* if(beam_valid) */
    
    if (!params_initialized) {
      
      /*
       * looking for the first valid beam for initialization
       */
      
      if (beam_valid) {
	
	/*
	 * initialize
	 */
	
	params_have_changed =
	  update_params (beam_buffer, gate_params_pkt,
			 nbytes_params_pkt);
	
	/*
	 * send initial params packet
	 */
	
	if (transmit(gate_params_pkt,
		     nbytes_params_pkt,
		     (si32) GATE_PARAMS_PACKET_CODE,
		     gate_params_pkt_seq_no) == 0) {
	  
	  gate_params_pkt_seq_no++;
	  
	  /*
	   * set up the first beam buffer
	   */
	  
	  load_beam(beam_buffer, gate_params_pkt,
		    current_gate_data_pkt, TRUE);
	  prev_beam_valid = TRUE;
	  params_initialized = TRUE;
	  
	}
	
      } /* if (beam_valid) */
      
    } else {
      
      /*
       * initialization has been done
       */
      
      if (beam_valid) {
	
	/*
	 * beam valid
	 */
	
	if (prev_beam_valid) {
	  
	  /*
	   * set prev beam pointers to previously obtained beam
	   */
	  
	  tmp_gate_data_pkt = prev_gate_data_pkt;
	  prev_gate_data_pkt = current_gate_data_pkt;
	  current_gate_data_pkt = tmp_gate_data_pkt;
	  
	  /*
	   * check if params have changed
	   */
	  
	  params_have_changed =
	    update_params (beam_buffer, gate_params_pkt,
			   nbytes_params_pkt);
	  
	  /*
	   * load in the new beam
	   */
	  
	  load_beam(beam_buffer, gate_params_pkt,
		    current_gate_data_pkt, TRUE);
	  
	  /*
	   * set beam flags - if returns TRUE, tilt has changed
	   * so set flag
	   */
	  
	  if (set_beam_flags(beam_buffer,
			     current_gate_data_pkt,
			     prev_gate_data_pkt))
	    prev_end_of_tilt = TRUE;
	  else
	    prev_end_of_tilt = FALSE;
	  
	  /*
	   * write out prev beam
	   */
	  
	  BE_from_array_32((ui32 *) prev_gate_data_pkt,
			   (ui32) sizeof(gate_data_beam_header_t));
	  
	  nbytes_data_pkt = sizeof(gate_data_beam_header_t) +
	    rparams->nfields_current * Glob->ngates_out;
	  
	  if (transmit(prev_gate_data_pkt,
		       nbytes_data_pkt,
		       (si32) GATE_DATA_PACKET_CODE,
		       gate_data_pkt_seq_no) == 0) {
	    
	    gate_data_pkt_seq_no++;
	    
	  }
	  
	  /*
	   * if params have changed, write them out
	   */
	  
	  if (params_have_changed || prev_end_of_tilt) {
	    
	    if (transmit(gate_params_pkt,
			 nbytes_params_pkt,
			 (si32) GATE_PARAMS_PACKET_CODE,
			 gate_params_pkt_seq_no) == 0) {
	      
	      gate_params_pkt_seq_no++;
	      
	    }
	    
	  }
	  
	} else {
	  
	  /*
	   * prev beam not valid, so the outgoing beam will
	   * already have been written out. Therefore, just
	   * load in incoming beam
	   */
	  
	  load_beam(beam_buffer, gate_params_pkt,
		    current_gate_data_pkt, TRUE);
	  
	} /* if (prev_beam_valid) */
	
	prev_beam_valid = TRUE;
	
      } else {
	
	/*
	 * beam not valid
	 */
	
	if (prev_beam_valid) {
	  
	  /*
	   * Read in the invalid beam, and set the
	   * flags on the previous beam, which was valid.
	   * Then write out prev beam
	   */
	  
	  tmp_gate_data_pkt = prev_gate_data_pkt;
	  prev_gate_data_pkt = current_gate_data_pkt;
	  current_gate_data_pkt = tmp_gate_data_pkt;
	  
	  load_beam(beam_buffer, gate_params_pkt,
		    current_gate_data_pkt, TRUE);
	  
	  /*
	   * set beam flags - if returns TRUE, tilt has changed
	   * so set flag
	   */
	  
	  if (set_beam_flags(beam_buffer,
			     current_gate_data_pkt,
			     prev_gate_data_pkt))
	    prev_end_of_tilt = TRUE;
	  else
	    prev_end_of_tilt = FALSE;
	  
	  BE_from_array_32((ui32 *) prev_gate_data_pkt,
			   (ui32) sizeof(gate_data_beam_header_t));
	  
	  nbytes_data_pkt = sizeof(gate_data_beam_header_t) +
	    rparams->nfields_current * Glob->ngates_out;
	  
	  if (transmit(prev_gate_data_pkt,
		       nbytes_data_pkt,
		       (si32) GATE_DATA_PACKET_CODE,
		       gate_data_pkt_seq_no) == 0) {

	    gate_data_pkt_seq_no++;
	    
	  }
	  
	  /*
	   * if prev beam was end of tilt, write out params
	   * because they may change for new tilt
	   */
	  
	  if (prev_end_of_tilt) {
	    
	    if (transmit(gate_params_pkt,
			 nbytes_params_pkt,
		                        (si32) GATE_PARAMS_PACKET_CODE,
			 gate_params_pkt_seq_no) == 0) {
	      
	      gate_params_pkt_seq_no++;
	      
	    }
	    
	  }
	  
	} /* if (prev_beam_valid) */
	
	prev_beam_valid = FALSE;
	
      } /* if (beam_valid) */
      
    } /* if (!params_initialized) */
    
  } /* while */
  
}

/********************************************************************
 * transmit()
 *
 * sends data to client
 *
 * returns 0 on success, -1 on failure
 */

static int transmit(ui08 *data,
		    si32 len,
		    si32 product_id,
		    si32 seq_no)
     
{
  
  static int listening = FALSE;
  static int sock_open = FALSE;
  static int proto_fd, client_fd;
  
  /*
   * listen on socket
   */
  
  if (!listening) {
    
    if ((proto_fd = SKU_open_server(Glob->output_port)) < 0) {
      
      fprintf(stderr, "ERROR - %s:process_data_stream:transmit\n",
	      Glob->prog_name);
      tidy_and_exit(-1);
      
    }
    
    listening = TRUE;
    
    if (Glob->debug)
      printf("listening on port %d\n", Glob->output_port);
    
  } /* if (!listening) */
  
  /*
   * open socket to client if necessary
   */
  
  if (!sock_open) {
    
    if ((client_fd = SKU_get_client_timed(proto_fd, 0L)) < 0) {
      if (Glob->debug) {
	fprintf(stderr, "trying to get client - not successful\n");
      }
      sleep (1);
      return (-1);
    } else {
      if (Glob->debug)
	printf("got client - successful\n");
      SKU_set_headers_to_new();
      sock_open = TRUE;
    }
    
  } /* if (!sock_open) */
  
  /*
   * write the data - on failure, close the socket. It will then
   * be reopened on the next attempt at a write
   */
  
  if (Glob->debug) {
    fprintf(stderr, "Writing beam, product id %ld, len %ld",
	    (long) product_id, (long) len);
  }
  
  if (SKU_writeh(client_fd, (char *) data, len,
		 product_id, seq_no) < 0) {
    
    if (Glob->debug) {
      fprintf(stderr, " - failed\n");
    }
    
    SKU_close(client_fd);
    sock_open = FALSE;
    return (-1);
    
  } else {
    
    if (Glob->debug) {
      fprintf(stderr, " - successful\n");
    }
    
    return (0);
    
  }
  
}

