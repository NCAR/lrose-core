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
 * create_data_stream.c
 *
 * writes test beam data to socket
 *
 * Nancy Rehak
 *
 * RAP NCAR Boulder CO USA
 *
 * September 1995
 *
 *******************************************************************/

#include <titan/file_io.h>
#include "test2gate.h"
#include <dataport/bigend.h>
#include <toolsa/pmu.h>

/* Prototypes for static functions */

static int transmit(ui08 *data,
		    si32 len,
		    si32 product_id,
		    si32 seq_no);

#define PARAMS_INTERVAL 90

/******************************************************************/

void create_data_stream(void)
{

  ui08 *beam_buffer;
  ui08 *param_buffer;
  int nbytes_param_buffer;
  int nbytes_beam_buffer;
  int count = 0;

  int forever = TRUE;
  int new_tilt;

  si32 gate_params_pkt_seq_no = 0;
  si32 gate_data_pkt_seq_no = 0;

  PMU_auto_register("Start of create_data_stream");

  /*
   * allocate beam buffer
   */

  nbytes_beam_buffer = sizeof(gate_data_beam_header_t) +
    (Glob->nfields * Glob->params.radar_params.num_gates);
  
  beam_buffer = (ui08 *)umalloc((ui32)nbytes_beam_buffer);
  
  /*
   * initialize the radar sampling
   */
  
  init_sampling();

  /*
   * allocate the param buffer and packet
   */

  param_buffer = create_param_buffer(&nbytes_param_buffer);

  /*
   * enter loop
   */

  while (forever)
  {

    PMU_auto_register("Creating tilt");

    /*
     * Create the beam data.
     */
    
    new_tilt = create_beam_buffer((gate_data_beam_header_t *)beam_buffer);

    if (count == 0)
    {
      
      if (Glob->params.debug) {
	print_radar_params(param_buffer, stderr);
      }
	
      /*
       * send params packet
       */
	
      if (transmit(param_buffer,
		   nbytes_param_buffer,
		   (si32) GATE_PARAMS_PACKET_CODE,
		   gate_params_pkt_seq_no) == 0)
      {
	gate_params_pkt_seq_no++;
      }

    }

    count++;
    if (count == PARAMS_INTERVAL) {
      count = 0;
    }
    
    /*
     * write out current beam
     */
	  
    if (transmit(beam_buffer,
		 nbytes_beam_buffer,
		 (si32) GATE_DATA_PACKET_CODE,
		 gate_data_pkt_seq_no) == 0)
    {
      gate_data_pkt_seq_no++;
    }
	
    /*
     * sleep for x milliseconds
     */

    uusleep(Glob->params.beam_wait_msecs * 1000);
    
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
      printf("listening on port %ld\n", Glob->params.output_port);
    
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
	printf("got client - successful\n");
      SKU_set_headers_to_new();
      sock_open = TRUE;
    }
    
  } /* if (!sock_open) */
  
  /*
   * write the data - on failure, close the socket. It will then
   * be reopened on the next attempt at a write
   */
  
  if (SKU_writeh(client_fd, (char *) data, len,
		 product_id, seq_no) < 0)
  {
    if (Glob->params.debug)
    {
      fprintf(stderr, "Writing beam, product id %d, len %ld", product_id,
	      (long) len);
      fprintf(stderr, " - failed\n");
    }
    
    SKU_close(client_fd);
    sock_open = FALSE;
    return (-1);
    
  }
  else
  {
    return (0);
  }
  
}

