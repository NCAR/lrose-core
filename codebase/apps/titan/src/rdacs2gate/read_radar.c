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
 *
 * Feb 1995
 *
 ************************************************************************/

#include "rdacs2gate.h"

/*
 * file scope prototypes
 */

static int LoginToRdacs(int radar_fd,
			char *user, char *password);

static void InitRdpHeader(RDP_HDR *hdr, ui32 id, ui32 length, ui32 status);

static int RdpRecv(int radar_fd, RDP_HDR *reply_hdr, void **reply_msg_p);

static int RdpSendRecv(int radar_fd,
		       RDP_HDR *out_hdr, void *pOut, ui32 cbOut,
		       RDP_HDR *reply_hdr, void **reply_msg_p);
     
static int SelectAsync(int radar_fd, ui32 SelectBits, ui32 RawPacing);

/*
 * main routine
 */

int read_radar(void)

{

  ui32 selectBits;
  int radar_fd = -1;
  int forever = TRUE;
  RDP_HDR reply_hdr;
  void *reply_msg;

  /*
   * initialize the radar params module
   */

  init_param_buffer();
  
  while (forever) {
    
    if (radar_fd < 0) {
      
      /*
       * open connection to radar - we are the client in this
       */
      
      if ((radar_fd = SKU_open_client(Glob->params.input_host,
				      Glob->params.input_port)) < 0) {
	
	/*
	 * failure - sleep and retry later
	 */
	
	sleep(1);
	
      } else {
	
	if (LoginToRdacs(radar_fd, Glob->params.user,
			 Glob->params.password) == FALSE) {
	  fprintf(stderr, "ERROR: %s:read_radar\n", Glob->prog_name);
	  fprintf(stderr, "Cannot log in to RDACS, user, password: %s, %s\n",
		  Glob->params.user, Glob->params.password);
	  return (-1);
	}

	/*
	 *  set up async message
	 */

	selectBits = (AS_CurStep | AS_AngData | AS_Site | AS_EngData);

	/*
	 * log or linear receiver for dbz
	 */
	
	if (Glob->params.log_receiver_for_dbz) {
	  selectBits |= AS_LogParms;
	  selectBits |= AS_LogData;
	} else {
	  selectBits |= AS_LinParms;
	  selectBits |= AS_LinData;
	}
	
	/*
	 * velocity data?
	 */

	if (Glob->params.fields.len >= 2) {
	  selectBits |= AS_VelParms;
	  selectBits |= AS_VelData;
	}

	/*
	 * spectral width data?
	 */

	if (Glob->params.fields.len >= 3) {
	  selectBits |= AS_TurbParms;
	  selectBits |= AS_TurbData;
	}
	
	/*
	 *  ask for async delivery of data
	 */

	if (SelectAsync(radar_fd, selectBits, 1) == FALSE) {
	  fprintf(stderr, "ERROR: %s:read_radar\n", Glob->prog_name);
	  fprintf(stderr, "Cannot SelectAsync\n");
	  return (-1);
	}
	
	if (Glob->params.debug) {
	  fprintf(stderr, "%s: connected to radar\n", Glob->prog_name);
	}

      }
      
    } else {

      /*
       * read an async message from rdacs
       */

      if(RdpRecv(radar_fd, &reply_hdr, &reply_msg)) {

	/*
	 * success
	 */

	handle_msg(&reply_hdr, reply_msg);
	
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

    } /* if (radar_fd < 0) */
    
  } /* while (forever) */

    return (0);

}


/********************
 * LoginToRdacs()
 ********************/

static int LoginToRdacs(int radar_fd,
			char *user, char *password)

{

  RDP_Login m;
  RDP_Login_R *r;
  RDP_HDR out_hdr, reply_hdr;
  
  InitRdpHeader(&out_hdr, ID_RDP_Login, sizeof(m), 0);
  strcpy((char *) m.User, user);
  strcpy((char *) m.Password, password);

  if (!RdpSendRecv(radar_fd, &out_hdr, &m, sizeof(m),
		   &reply_hdr, (void **) &r)) {
    printf("Bad response to login\n");
    return (FALSE);
  }

  if (reply_hdr.Status != 0) {
    printf("Login failed\n");
    return (FALSE);
  }

  return (TRUE);

}

/************************
 * InitRdpHeader()
 ************************/

static void InitRdpHeader(RDP_HDR *hdr, ui32 id, ui32 length, ui32 status)
     
{
  hdr->Length = sizeof(RDP_HDR) + length;
  hdr->Id = id;
  hdr->XA5 = 0xa5;
  hdr->Status = status;
}

/************************
 * RdpRecv()
 ************************/

static int RdpRecv(int radar_fd, RDP_HDR *reply_hdr, void **reply_msg_p)
     
{
  
  int nleft;
  int nneeded;
  int npad;
  char *msg_body;

  RDP_GetRayParms_R *rayparms_r;
  SITECFG *cfg;

  static int nmsg_alloc = 0;
  static void *reply_msg = NULL;
  
  if (SKU_read(radar_fd, reply_hdr, sizeof(RDP_HDR), -1)
      != sizeof(RDP_HDR)) {
    fprintf(stderr, "Bad header response\n");
    return (FALSE);
  }

  SwapRDP_HDR(reply_hdr);
  
  if (reply_hdr->XA5 != 0xa5 || reply_hdr->Length < sizeof(RDP_HDR)) {
    fprintf(stderr, "Bad header response 2\n");
    return (FALSE);
  }

  /*
   * For some of the structs,
   * we need to pad out things to make sure about alignment.
   * The 'padding' members were added for that purpose.
   */

  npad = 0;
  if (reply_hdr->Id == ID_RDP_GetRayParms) {
    npad = sizeof(rayparms_r->padding);
  }
  if (reply_hdr->Id == ID_RDP_GetSite) {
    npad = sizeof(cfg->padding);
  }

  nneeded = reply_hdr->Length + npad;

  if (nneeded > nmsg_alloc) {
    if (reply_msg == NULL) {
      reply_msg = umalloc(nneeded);
    } else {
      reply_msg = urealloc(reply_msg, nneeded);
    }
    nmsg_alloc = nneeded;
  }

  nleft = reply_hdr->Length - sizeof(RDP_HDR);
  msg_body = ((char *) reply_msg) + npad;
  
  if (nleft > 0) {
    if (SKU_read(radar_fd, msg_body, nleft, -1) != nleft) {
      fprintf(stderr, "Cannot read message body\n");
      return (FALSE);
    }
  }

  *reply_msg_p = reply_msg;
  return (TRUE);
    
}

/************************
 * RdpSendRecv()
 ************************/

static int RdpSendRecv(int radar_fd,
		       RDP_HDR *out_hdr, void *pOut, ui32 cbOut,
		       RDP_HDR *reply_hdr, void **reply_msg_p)
     
{

  RDP_HDR swap_hdr;

  swap_hdr = *out_hdr;
  SwapRDP_HDR(&swap_hdr);
  
  if (SKU_write(radar_fd, (void *) &swap_hdr, sizeof(RDP_HDR), -1)
      != sizeof(RDP_HDR)) {
    fprintf(stderr, "Send of out_hdr to RDACS failed\n");
    return (FALSE);
  }
  
  if (SKU_write(radar_fd, pOut, cbOut, -1) != cbOut) {
    fprintf(stderr, "Send of out_msg to RDACS failed\n");
    return (FALSE);
  }

  if (RdpRecv(radar_fd, reply_hdr, reply_msg_p) == FALSE) {
    return (FALSE);
  }
  
  if (out_hdr->Id != reply_hdr->Id)	{
    fprintf(stderr, "Received bad ID from RDACS\n");
    return (FALSE);
  }

  return (TRUE);
    
}

/************************
 * SelectAsync()
 ************************/

/*
 * send the rdp select async command
 */

static int SelectAsync(int radar_fd, ui32 SelectBits, ui32 RawPacing)
{

  RDP_SelectAsync m;
  RDP_HDR out_hdr, reply_hdr;
  void *reply;

  InitRdpHeader(&out_hdr, ID_RDP_SelectAsync, sizeof(m), 0);

  m.SelectBits = SelectBits; /* AS_xxx */
  m.RawPacing = RawPacing; /* pacing value if
			    * raw data selected */

  SwapRDP_SelectAsync(&m);

  if (!RdpSendRecv(radar_fd, &out_hdr, &m, sizeof(m),
		   &reply_hdr, &reply)) {
    return (FALSE);
  }
  return (reply_hdr.Status == 0);

}

