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
/****************************************************************************

	wsi_net_io.c
	
	Source code for the TCP/IP network functions for the WSI satellite
 weather database applications.
 
GRAPHICS

 Developed by: K. E. Stringham, Jr. Group 42 | 19-MAR-1997
 
****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

#include <sys/time.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>

#include "fs.h"


#define READ_RETRIES     10
#define WAIT_MSECS     1000


union
{
  struct sockaddr sock_addr;    /* Socket address structure */
  struct sockaddr_in sock_in;   /* Internet socket address structure */
} u;

struct pollfd gDevPollArray[2];

/*
 * Prototypes for forward functions.
 */

int NetIORead(ui08 *pBuf,
	      int count);
int ReadMsg(char *program_name,
	    ui08 *pBuf);

static void handle_sigpoll(int signal_num);

/*
 * Globals
 */

char Ip_address[MAX_PATH_LEN];
int Port;

/***** Function Designed to open a TCP/IP port *****/

int open_net(char *ip_address, int iPort)
{
  /* Save the address information */

  STRcopy(Ip_address,ip_address, MAX_PATH_LEN);
  Port = iPort;
  
  /* set up signals */

  PORTsignal(SIGIO, handle_sigpoll);

  /* Open the connection */

  do
  {
    PMU_auto_register("Trying to connect to client");
    
    gDevPollArray[0].fd = SKU_open_client_timed(ip_address, iPort, 1000);

    if (gDevPollArray[0].fd < 0)
    {
      fprintf(stderr,
	      "Error %d opening client on address %s, port %d\n",
	      SKUerrno, ip_address, iPort);
      
      sleep(1);
    }
      
  } while (gDevPollArray[0].fd < 0);
  
  return( FALSE);
}    

/***** Read Message into Buffer *****/

int ReadMsg(char *program_name, ui08 *pBuf)
{
  ui32   len;
  int    msgLen;
  ui08   *pTmp, c1 = 0, c2 = 0, c3 = 0, c4 = 0;

  /***** Initializing I/O Buffers *****/

  pTmp = pBuf;

  c4 = 1;
  msgLen = 0;

  FOREVER
  {
    /* look for sync pattern for next message */

    FOREVER
    {
      PMU_auto_register("Looking for sync pattern");
      
      /* shift the bytes read in so far */

      c1 = c2; c2 = c3; c3 = c4;

      if (NetIORead(&c4, 1) != 1)
      {
	fprintf(stderr, "\nSync Read returned an Error");
	return( -1);
      }

      if (!c1 && c2==0xff && !c3 && c4==0xff)
	break;
    }

    c4 = 1;

    FOREVER
    {
      PMU_auto_register("Reading next message from socket");
      
      if (NetIORead((ui08 *)&len, sizeof(len)) != sizeof(len))
      {
	fprintf(stderr, "\nReadMsg Length Read returned an Error");
	return( -1);
      }

      len = ntohl(len);

      if (len == 0)
	return(msgLen);
      else if (len == -1)
      {
	fprintf(stderr, "\n%s: error in data\n", program_name);

	pTmp   = pBuf;
	msgLen = 0;
	break;
      }
      else if (len & 0xff000000)
      {
	fprintf(stderr, "\n%s: out of sync with wsidata\n", program_name);

	pTmp   = pBuf;
	msgLen = 0;
	break;
      }

      if (NetIORead(pTmp, len) != len)
      {
	fprintf(stderr, "\nReadMsg data Read returned an Error");
	return( -1);
      }

      msgLen += len;
      pTmp   += len;
    }
  }
}

/***** Get Character(s) *****/

static ui08 buff[kMsgIOBufSz];
static ui08 *pExtract0;
static int    bufLen = 0;
    
int ReadBuf(char *program_name, ui08 *pBuf, int len)
{
  if (len > bufLen && bufLen)
  {
    memcpy( pBuf, pExtract0, bufLen);
    pBuf   += bufLen;
    len    -= bufLen;
    bufLen  = 0;
  }

  if (!bufLen)
  {
    if ((bufLen = ReadMsg(program_name, buff)) == -1)
      return( -1);

    pExtract0 = buff;
  }

  if (len)
  {
    memcpy(pBuf, pExtract0, len);

    pExtract0 += len;
    bufLen    -= len;
  }

  return(len);
}

/***** Read N Characters from the NetIO Buffer *****/

static ui08 buffer[kNetBufSz];
static ui08 *pExtract;
static int    byteCount = 0;

int NetIORead(ui08 *pBuf, int count)
{
  int chCount, lenmsg, nread, nbyts;
    
  PMU_auto_register("Getting network data");
  
  chCount = count;
    
  if (count > byteCount)
  {
    if (byteCount)
    {
      memcpy( pBuf, pExtract, byteCount);
      pBuf    += byteCount;
      chCount -= byteCount;
    }
        
    pExtract = buffer;
        
    for (byteCount = 0; lenmsg = kNetRdSz, byteCount < kNetRdSz; /* NULL increment */)
    {
      nread = kNetRdSz;
            
      if (nread > (lenmsg - byteCount))
	nread = lenmsg - byteCount;
            
      do
      {
/*	if (Params.debug_level >= DEBUG_EXTRA)
 *	  fprintf(stderr,
 *		  "Trying to read %d bytes from socket\n",
 *		  nread);
 */
	
	nbyts = SKU_read_timed_hb(gDevPollArray[0].fd,
				  (void *)&buffer[byteCount], nread,
				  READ_RETRIES, WAIT_MSECS,
				  1, PMU_auto_register);

/*	if (Params.debug_level >= DEBUG_EXTRA)
 *	  fprintf(stderr,
 *		  "   %d bytes read\n", nbyts);
 */
	
      } while (nbyts == -1);
      
      if (nread > 0 && nbyts <= 0)
      {
	/*
	 * Try to reconnect to the socket in case it died
	 */

	fprintf(stderr, 
		"*** Error %d returned from SKU_read_timed_hb(), reconnecting...\n",
		nbyts);
	
	SKU_close(gDevPollArray[0].fd);
	open_net(Ip_address, Port);
	
	perror("receive");
	return(nbyts);
      }
            
      byteCount += nbyts;

    }
        
    if (chCount > byteCount)
    {
      fprintf(stderr, "\nNetIORead Size Exceeded!\n");
      return( -1);
    }
  }
        
  memcpy(pBuf, pExtract, chCount);
  pExtract  += chCount;
  byteCount -= chCount;
    
  return(count);
}

/* signal functions */

static void handle_sigpoll(int signal_num)
{
  fprintf(stderr, "*** Received SIGPOL event\n");
  
  gDevPollArray[0].events = POLLIN;
  poll(gDevPollArray, 1, 1000);
    
  if (gDevPollArray[0].revents | POLLERR)
  {
    fprintf(stderr, "    POLLERR occurred\n");
    
    SKU_close(gDevPollArray[0].fd);
    open_net(Ip_address, Port);
  }
  
  if (gDevPollArray[0].revents | POLLHUP)
  {
    fprintf(stderr, "    POLLHUP occurred\n");
    
    SKU_close(gDevPollArray[0].fd);
    open_net(Ip_address, Port);
  }
  
  if (gDevPollArray[0].revents | POLLNVAL)
  {
    fprintf(stderr, "    POLLNVAL occurred\n");
    
    SKU_close(gDevPollArray[0].fd);
    open_net(Ip_address, Port);
  }
  
  PORTsignal(signal_num, handle_sigpoll);
}
