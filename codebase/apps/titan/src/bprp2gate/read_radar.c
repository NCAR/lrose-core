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
/***********************************************************************
 * read_radar.c
 *
 * Read the incoming radar stream
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 * Orig code by Mark Shorman
 *
 * Feb 1995
 *
 ************************************************************************/

#include "bprp2gate.h"

void read_radar(void)

{

  int iret;
  int radar_fd = -1;
  int forever = TRUE;

  bprp_response_t response;

  while (forever) {

    PMU_auto_register("In read_data() - top");

    if (radar_fd < 0) {

      /*
       * open connection to radar - we are the client in this
       */
      
      if ((radar_fd = SKU_open_client(Glob->params.input_host,
				      Glob->params.input_port)) < 0) {
	
	PMU_auto_register("In read_data() - waiting for connection");
	
	/*
	 * failure - sleep and retry later
	 */
	
	sleep(1);
	
      } else {

	if (Glob->params.debug) {
	  fprintf(stderr, "%s: connected to radar\n", Glob->prog_name);
	}

      }
      
    } else {

      /*
       * wait on packet, timing out after 10 secs each time
       */
      
      while ((iret = SKU_read_select(radar_fd, 10000)) == -1) {
	/*
	 * timeout
	 */
	PMU_auto_register("In read_data() - waiting for a beam");
      }
      
      if (iret == 1) {

	/*
	 * read a beam and process
	 */
	
	if (SKU_read(radar_fd, &response, sizeof(bprp_response_t), 20) ==
	    sizeof(bprp_response_t)) {
	  
	  /*
	   * success
	   */
	  
	  handle_response(&response);
	  
	} else {
	  
	  /*
	   * read error - disconnect and try again later
	   */
	  
	  if (Glob->params.debug) {
	    fprintf(stderr, "%s: read error - closing connection to radar\n",
		    Glob->prog_name);
	  }
	  
	  SKU_close(radar_fd);
	  radar_fd = -1;
	
	} /* SKU_read */

      } else {

	/*
	 * read error - disconnect and try again later
	 */
	
	if (Glob->params.debug) {
	  fprintf(stderr, "%s: read error - closing connection to radar\n",
		  Glob->prog_name);
	}
	
	SKU_close(radar_fd);
	radar_fd = -1;
	
      } /* if (iret == 1) */
      
    } /* if (radar_fd < 0) */
    
  } /* while (forever) */

  return;

}


