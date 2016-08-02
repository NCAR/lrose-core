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
/********************************************************************
 * process_data_stream.c
 *
 * reads in the beam data, processes it and writes it to socket
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * September 1992
 *
 * Copied from rdata_to_socket for use in uf2gate by Nancy Rehak,
 * Mar 1995.
 *******************************************************************/

#include "uf2gate.h"
#include <dataport/bigend.h>

/* Prototypes for static functions */

static int transmit(ui08 *data,
		    si32 len,
		    si32 product_id,
		    si32 seq_no);

/******************************************************************/

void process_data_stream(int n_input_files, char **input_file_list)

{

  ui08 *beam_buffer;
  ui08 *param_buffer;
  ui08 *gate_params_pkt;
  ui08 *current_gate_data_pkt, *prev_gate_data_pkt;
  ui08 *tmp_gate_data_pkt;

  int forever = TRUE;
  int prev_end_of_tilt;
  int params_initialized = FALSE;
  int header_count = 0;
  int summary_count = 0;
  int beam_valid, prev_beam_valid;
  int end_of_data;
  int params_have_changed;

  si32 gate_params_pkt_seq_no = 0;
  si32 gate_data_pkt_seq_no = 0;
  si32 nbytes_params_pkt;
  si32 nbytes_data_pkt;

  gate_data_radar_params_t *rparams;

  /*
   * allocate beam buffer
   */

  nbytes_data_pkt = sizeof(gate_data_beam_header_t) +
    (Glob->num_fields_out * Glob->params.ngates_out);
  
  beam_buffer = (ui08 *)umalloc((ui32)nbytes_data_pkt);
  
  /*
   * allocate the param buffer
   */

  nbytes_params_pkt = sizeof(gate_data_radar_params_t) +
    Glob->num_fields_out * sizeof(gate_data_field_params_t);

  param_buffer = (ui08 *)umalloc((ui32)nbytes_params_pkt);
  
  /*
   * allocate parameters packet
   */
  
  gate_params_pkt = (ui08 *) umalloc ((ui32) nbytes_params_pkt);

  rparams = (gate_data_radar_params_t *) gate_params_pkt;

  /*
   * allocate the space for the data packet, including space for
   * storing the immediately previous one
   */

  current_gate_data_pkt = (ui08 *) umalloc ((ui32) nbytes_data_pkt);
  prev_gate_data_pkt = (ui08 *) umalloc ((ui32) nbytes_data_pkt);

  /*
   * enter loop
   */

  while (forever)
  {
    /*
     * read in a beam from the current file.
     */
    
    switch(read_disk_uf(n_input_files, input_file_list,
			beam_buffer, param_buffer))
    {
    case UF_READ_BEAM_VALID :
      beam_valid = TRUE;
      end_of_data = FALSE;
      break;
    case UF_READ_BEAM_INVALID :
      beam_valid = FALSE;
      end_of_data = FALSE;
      break;
    case UF_READ_END_OF_DATA :
      beam_valid = FALSE;
      end_of_data = TRUE;
      break;
    }
    
    /*
     * for valid beams, print out as required
     */
    
    if (beam_valid)
    {
      /*
       * print header if requested
       */
      
      if (Glob->params.header_print)
      {
	if (header_count == 0)
	{
	  print_radar_params(param_buffer);
	  print_beam_buffer(beam_buffer);
	}
	header_count++;
	if (header_count == Glob->params.header_interval)
	  header_count = 0;
      }
      
      /*
       * print summary if requested
       */
      
      if (Glob->params.summary_print)
      {
	if (summary_count == 0)
	  print_summary(beam_buffer, param_buffer);
	summary_count++;
	if (summary_count == Glob->params.summary_interval)
	  summary_count = 0;
      }
      
    } /* if(beam_valid) */
    
    if (!params_initialized)
    {
      /*
       * looking for the first valid beam for initialization
       */
      
      if (beam_valid)
      {
	/*
	 * initialize
	 */
	
	params_have_changed =
	  update_params (param_buffer, gate_params_pkt,
			 nbytes_params_pkt);
	
	fprintf(stdout, "Initial radar parameters:\n");
	print_radar_params(param_buffer);
	
	/*
	 * send initial params packet
	 */
	
	if (transmit(gate_params_pkt,
		     nbytes_params_pkt,
		     (si32) GATE_PARAMS_PACKET_CODE,
		     gate_params_pkt_seq_no) == 0)
	{
	  gate_params_pkt_seq_no++;
	  
	  /*
	   * set up the first beam buffer
	   */
	  
	  memcpy((void *)current_gate_data_pkt,
		 (void *)beam_buffer,
		 (size_t)nbytes_data_pkt);
	  memcpy((void *)gate_params_pkt,
		 (void *)param_buffer,
		 (size_t)nbytes_params_pkt);

	  prev_beam_valid = TRUE;
	  params_initialized = TRUE;
	  
	}
	
      } /* if (beam_valid) */
      
    }
    else
    {
      /*
       * initialization has been done
       */
      
      if (beam_valid)
      {
	/*
	 * beam valid
	 */
	
	if (prev_beam_valid)
	{
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
	    update_params (param_buffer, gate_params_pkt,
			   nbytes_params_pkt);
	  
	  /*
	   * load in the new beam
	   */
	  
	  memcpy((void *)current_gate_data_pkt,
		 (void *)beam_buffer,
		 (size_t)nbytes_data_pkt);
	  memcpy((void *)gate_params_pkt,
		 (void *)param_buffer,
		 (size_t)nbytes_params_pkt);

	  /*
	   * set beam flags - if returns TRUE, tilt has changed
	   * so set flag
	   */
	  
	  prev_end_of_tilt = set_beam_flags(current_gate_data_pkt,
					    prev_gate_data_pkt,
					    end_of_data);
	  
	  /*
	   * write out prev beam
	   */
	  
	  BE_from_array_32((ui32 *) prev_gate_data_pkt,
			   (ui32) sizeof(gate_data_beam_header_t));
	  
	  if (transmit(prev_gate_data_pkt,
		       nbytes_data_pkt,
		       (si32) GATE_DATA_PACKET_CODE,
		       gate_data_pkt_seq_no) == 0)
	  {
	    gate_data_pkt_seq_no++;
	  }
	  
	  /*
	   * if params have changed, write them out
	   */
	  
	  if (params_have_changed || prev_end_of_tilt)
	  {
	    fprintf(stdout, "New radar parameters:\n");
	    print_radar_params(param_buffer);
	    
	    if (transmit(gate_params_pkt,
			 nbytes_params_pkt,
			 (si32) GATE_PARAMS_PACKET_CODE,
			 gate_params_pkt_seq_no) == 0)
	    {
	      gate_params_pkt_seq_no++;
	    }
	    
	  }
	  
	}
	else
	{
	  /*
	   * prev beam not valid, so the outgoing beam will
	   * already have been written out. Therefore, just
	   * load in incoming beam
	   */
	  
	  memcpy((void *)current_gate_data_pkt,
		 (void *)beam_buffer,
		 (size_t)nbytes_data_pkt);
	  memcpy((void *)gate_params_pkt,
		 (void *)param_buffer,
		 (size_t)nbytes_params_pkt);

	} /* if (prev_beam_valid) */
	
	prev_beam_valid = TRUE;
	
      }
      else
      {
	/*
	 * beam not valid
	 */
	
	if (prev_beam_valid)
	{
	  /*
	   * Read in the invalid beam, and set the
	   * flags on the previous beam, which was valid.
	   * Then write out prev beam
	   */
	  
	  tmp_gate_data_pkt = prev_gate_data_pkt;
	  prev_gate_data_pkt = current_gate_data_pkt;
	  current_gate_data_pkt = tmp_gate_data_pkt;
	  
	  memcpy((void *)current_gate_data_pkt,
		 (void *)beam_buffer,
		 (size_t)nbytes_data_pkt);
	  memcpy((void *)gate_params_pkt,
		 (void *)param_buffer,
		 (size_t)nbytes_params_pkt);

	  /*
	   * set beam flags - if returns TRUE, tilt has changed
	   * so set flag
	   */
	  
	  prev_end_of_tilt = set_beam_flags(current_gate_data_pkt,
					    prev_gate_data_pkt,
					    end_of_data);
	  
	  BE_from_array_32((ui32 *) prev_gate_data_pkt,
			   (ui32) sizeof(gate_data_beam_header_t));
	  
	  if (transmit(prev_gate_data_pkt,
		       nbytes_data_pkt,
		       (si32) GATE_DATA_PACKET_CODE,
		       gate_data_pkt_seq_no) == 0)
	  {
	    fprintf(stdout, "Sending last data packet, end_of_tilt = %d, end_of_volume = %d\n",
		   ((gate_data_beam_header_t *)prev_gate_data_pkt)->end_of_tilt,
		   ((gate_data_beam_header_t *)prev_gate_data_pkt)->end_of_volume);
	    gate_data_pkt_seq_no++;
	  }
	  
	  /*
	   * if prev beam was end of tilt, write out params
	   * because they may change for new tilt
	   */
	  
	  if (prev_end_of_tilt)
	  {
	    if (transmit(gate_params_pkt,
			 nbytes_params_pkt,
			 (si32) GATE_PARAMS_PACKET_CODE,
			 gate_params_pkt_seq_no) == 0)
	    {
	      gate_params_pkt_seq_no++;
	    }
	    
	  }

	  /*
	   * If we are at the end of the data, send out an extra
	   * packet so the last file will be written.
	   */

	  if (end_of_data)
	    if (transmit(prev_gate_data_pkt,
			 nbytes_data_pkt,
			 (si32) GATE_DATA_PACKET_CODE,
			 gate_data_pkt_seq_no) == 0)
	    {
	      fprintf(stdout, "Sending duplicate packet for last packet.\n");
	      gate_data_pkt_seq_no++;
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
  
  if (!listening)
  {
    if ((proto_fd = SKU_open_server(Glob->params.output_port)) < 0)
    {
      fprintf(stderr, "ERROR - %s:process_data_stream:transmit\n",
	      Glob->prog_name);
      tidy_and_exit(-1);
    }
    
    listening = TRUE;
    
    if (Glob->params.debug)
      fprintf(stdout, "listening on port %ld\n", Glob->params.output_port);
    
  } /* if (!listening) */
  
  /*
   * open socket to client if necessary
   */
  
  if (!sock_open)
  {
    if ((client_fd = SKU_get_client_timed(proto_fd, 0L)) < 0)
    {
      if (Glob->params.debug)
      {
	fprintf(stderr, "trying to get client - not successful\n");
      }
      sleep (1);
      return (-1);
    }
    else
    {
      if (Glob->params.debug)
	fprintf(stdout, "got client - successful\n");
      SKU_set_headers_to_new();
      sock_open = TRUE;
    }
    
  } /* if (!sock_open) */
  
  /*
   * write the data - on failure, close the socket. It will then
   * be reopened on the next attempt at a write
   */
  
  if (Glob->params.debug)
  {
    fprintf(stderr, "Writing beam, product id %d, len %ld",
	    product_id, (long) len);
  }
  
  if (SKU_writeh(client_fd, (char *) data, len,
		 product_id, seq_no) < 0)
  {
    if (Glob->params.debug)
    {
      fprintf(stderr, " - failed\n");
    }
    
    SKU_close(client_fd);
    sock_open = FALSE;
    return (-1);
    
  }
  else
  {
    if (Glob->params.debug)
    {
      fprintf(stderr, " - successful\n");
    }
    
    return (0);
    
  }
  
}

