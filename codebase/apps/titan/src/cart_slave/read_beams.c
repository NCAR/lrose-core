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
 * read_beams.c
 *
 * reads the beam packets from the socket, and stores the data in the
 * cartesian grid
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1991
 *
 ****************************************************************************/

#include "cart_slave.h"

int read_beams(void)

{

  time_t now;
  time_t last_write = 0;
  int ireturn = CS_SUCCESS;
  si32 packet_id;
  beam_packet_header_t *bhdr;

  /*
   * enter loop which reads packets and writes the data
   * out to shared memory
   */

  while(ireturn != CS_FAILURE) {

    if ((ireturn = read_packet(&packet_id)) != CS_FAILURE) {

      bhdr = (beam_packet_header_t *) Glob->packet;

      if (packet_id == END_OF_VOLUME_PACKET_CODE) {
	
	if (Glob->params.debug)
	  fprintf(stderr, "end of scan\n");
	
      } else if (packet_id == CART_DATA_PACKET_CODE) {
	
	load_beam(Glob->packet);

	/*
	 * check for file writes
	 */
	
	now = time(NULL);

	if ((now - last_write) > Glob->params.output_interval) {
	  if (write_output()) {
	    return (CS_FAILURE);
	  }
	  last_write = now;
	}

      }
      
    } /* if (ireturn != CS_FAILURE) */

  } /* while */
  
  return(ireturn);

}

