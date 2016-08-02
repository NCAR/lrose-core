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
 * write_reply.c
 *
 * Writes reply struct to the client
 *
 * returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"
#include <netinet/in.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

static si32 seq_no = -1;

int write_reply(int sockfd,
		int status,
		char *text)

{

  tdata_reply_t reply;

  /*
   * load up reply struct
   */

  reply.status = BE_from_si32((si32) status);
  strncpy(reply.text, text, TDATA_TEXT_LEN);
  
  seq_no++;
  
  if (SKU_writeh(sockfd,
		 (char *) &reply,
		 (si32) sizeof(tdata_reply_t),
		 (si32) TDATA_REPLY_PACKET_ID,
		 seq_no) < 0) {
    
    return (-1);

  } else {

    return (0);

  }

}
