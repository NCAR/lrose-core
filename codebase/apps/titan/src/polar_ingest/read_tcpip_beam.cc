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
 * read_tcpip_beam.c
 *
 * reads a beam from a tcpip socket
 *
 * If no params have been read, this waits until params arive.
 * If new params arrive, these are read in.
 *
 * Mike Dixon September 1992
 *
 **************************************************************************/

#include "polar_ingest.h"
#include <dataport/bigend.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

static int count = 0;

static int read_packet(si32 *packet_id,
		       char **data_ptr);

void read_tcpip_beam (gate_data_radar_params_t **gate_rparams,
		      gate_data_field_params_t **gate_fparams,
		      gate_data_beam_header_t **gate_header,
		      ui08 **gate_data)

{

  static int params_read = FALSE;
  static gate_data_radar_params_t rparams;
  static gate_data_field_params_t *fparams = NULL;
  static si32 nfields_allocated = 0;

  char *beam_buffer;
  int data_read;
  si32 packet_id;
  gate_data_beam_header_t *header;

  /*
   * read in a packet - if the params have never been read, keep reading
   * until they are found
   */

  do { /* while (!params_read ... */

    int iret;

    data_read = FALSE;

    count++;

    iret = read_packet(&packet_id, &beam_buffer);

    if (iret == 0) {

      if (packet_id == GATE_PARAMS_PACKET_CODE) {

	memcpy ((void *) &rparams,
		(void *) beam_buffer,
		(size_t) sizeof(gate_data_radar_params_t));

	BE_to_array_32((ui32 *) &rparams,
		       (ui32) sizeof(gate_data_radar_params_t));
	
	if (fparams == NULL) {
	  
	  fparams = (gate_data_field_params_t *) umalloc
	    ((ui32) (rparams.nfields * sizeof(gate_data_field_params_t)));

	  nfields_allocated = rparams.nfields;

	} else {

	  if (nfields_allocated < rparams.nfields) {

	    fparams = (gate_data_field_params_t *) urealloc
	      ((char *) fparams,
	       (ui32) (rparams.nfields * sizeof(gate_data_field_params_t)));

	    nfields_allocated = rparams.nfields;

	  }

	} /* if (fparams ... */

	memcpy ((void *) fparams,
		(void *) (beam_buffer +
			  sizeof(gate_data_radar_params_t)),
		(size_t) (rparams.nfields *
			  sizeof(gate_data_field_params_t)));

	BE_to_array_32((ui32 *) fparams,
		       (ui32) (rparams.nfields *
			       sizeof(gate_data_field_params_t)));
	
	params_read = TRUE;

      } else if (packet_id == GATE_DATA_PACKET_CODE) {

	header = (gate_data_beam_header_t *) beam_buffer;
	BE_to_array_32((ui32 *) header,
		       (ui32) sizeof(gate_data_beam_header_t));
	
	data_read = TRUE;
	
      } /* if (packet_id ... */

    } else {

      sleep(1);

    } /* if (read_packet ... */

  } while (!params_read || !data_read);

  /*
   * wait a given time
   */

  if (Glob->tape_read_wait > 0)
    uusleep((ui32) Glob->tape_read_wait);

  /*
   * set pointers
   */

  *gate_rparams = &rparams;
  *gate_fparams = fparams;
  *gate_header = header;
  *gate_data = (ui08 *) (beam_buffer + sizeof(gate_data_beam_header_t));

  return;

}

/*********************************************************************
 * read_packet()
 *
 * reads a packet
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

static int read_packet(si32 *packet_id,
		       char **data_ptr)

{

  static int sock_dev;
  static int connected = FALSE;

  int read_success;
  int iret;
  long nread;
  SKU_header_t packet_header;

  /*
   * if not connected, try to connect
   */

  if (!connected) {

    if ((sock_dev =
	 SKU_open_client(Glob->tcpip_host,
			 (int) Glob->tcpip_port)) < 0) {
      PMU_auto_register("Trying to connect");
      return (-1);

    } else {

      if (Glob->debug)
	fprintf(stderr, "connected to server\n");

      PMU_force_register("Connected");
      connected = TRUE;

    }

  } /* if (!connected) */

  /*
   * read a new packet into the buffer
   */

  *packet_id = -1;
  read_success = TRUE;

  /*
   * wait on packet, timing out after 10 secs each time
   */
  
  while ((iret = SKU_read_select(sock_dev, 10000)) == -1) {
    /*
     * timeout
     */
    PMU_auto_register("Waiting for tcpip data");
  }
  
  if (iret == 1) {

    /*
     * packet ready - read it in, allowing 0.1 sec delay
     */
    
    if (SKU_read_message(sock_dev, &packet_header, data_ptr,
			 &nread, 10000) == 1) {
      
      *packet_id = packet_header.id;

    } else {

      /*
       * read failed - server probably down
       */
    
      if (Glob->debug)
	fprintf(stderr, "read failed - server probably down\n");

      read_success = FALSE;

    }

  } else {

    /*
     * select failed - server probably down
     */
    
    if (Glob->debug)
      fprintf(stderr, "select failed - server probably down\n");

    read_success = FALSE;

  }
  
  if (read_success) {
    
    PMU_auto_register("Reading tcpip data");
    return(0);
    
  } else {

    /*
     * server is down, so close socket and return
     */

    if (Glob->debug)
      fprintf(stderr, "server down - disconnected\n");

    SKU_close(sock_dev);
    connected = FALSE;
    PMU_force_register("Server is down");
    return (-1);
    
  } /* if (read_success) */
  
}

