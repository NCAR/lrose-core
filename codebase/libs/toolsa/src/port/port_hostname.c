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
/* port_hostname.c
   portability covers */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/str.h>
#include <toolsa/port.h>

#define MAX_HOSTNAME 128

/**************************
 * get name from uname call
 */

static char *_uname_(void)

{
  
  static struct utsname u;

  if (uname(&u) < 0) {
    return ("Unknown");
  }
  
  return (u.nodename);

}

/**********************************
 * get name from gethostbyname call
 */

static char *_gethostbyname_(void)

{

  static char hostname[MAX_HOSTNAME];
  struct hostent *h;
  struct utsname u;

  if (uname(&u) < 0) {
    return ("Unknown");
  }
  
  if ((h = gethostbyname(u.nodename)) == NULL) {
    return ("Unknown");
  }

  STRncopy(hostname, h->h_name, MAX_HOSTNAME);

  return (hostname);

}

/**************************************
 * PORThostname()
 *
 * Short host name - no internet detail.
 *
 * Returns pointer to static memory - do not free.
 */

char *PORThostname(void)

{
  
#if defined(DECOSF1)

  return (_gethostbyname_());

#else

  return (_uname_());

#endif

}

/**************************************
 * PORThostnameFull()
 *
 * Fully qualified internet host name.
 *
 * Returns pointer to static memory - do not free.
 */

char *PORThostnameFull(void)

{
  
#if defined(DECOSF1)

  return (_uname_());

#else

  return (_gethostbyname_());

#endif

}

/****************************
 * PORThostIpAddr()
 * 
 * Returns IP address of host.
 *
 * Returns pointer to static memory - do not free.
 */

char *PORThostIpAddr(void)

{
  
  int i;

  static char ipaddr[MAX_HOSTNAME];
  char *strp;

  struct hostent *h;
  struct utsname u;

  if (uname(&u) < 0) {
    return ("Unknown");
  }
  
  if ((h = gethostbyname(u.nodename)) == NULL) {
    return ("Unknown");
  }

  strp = ipaddr;
  for (i = 0; i < h->h_length; i++) {
    char addrPart[32];
    snprintf(addrPart,31,"%d",(unsigned char)h->h_addr_list[0][i]);
    sprintf (strp, "%s", addrPart);
    strp = ipaddr + strlen(ipaddr);
    if (i < h->h_length - 1) {
      *strp = '.';
      strp++;
    }
  }

  return (ipaddr);

}

/****************************
 * PORTremoteIpAddr()
 * 
 * Returns IP address of remote host.
 *
 * Returns pointer to static memory - do not free.
 */

char *PORTremoteIpAddr(char *remote_hostname)

{
  
  int i;

  static char ipaddr[MAX_HOSTNAME];
  char *strp;

  struct hostent *h;
  
  if ((h = gethostbyname(remote_hostname)) == NULL) {
    return ("Unknown");
  }
  
  
  strp = ipaddr;
  for (i = 0; i < h->h_length; i++) {
    char addrPart[32];
    snprintf(addrPart,31,"%d",(unsigned char)h->h_addr_list[0][i]);
    sprintf (strp, "%s", addrPart);
    strp = ipaddr + strlen(ipaddr);
    if (i < h->h_length - 1) {
      *strp = '.';
      strp++;
    }
  }

  return (ipaddr);

}


/****************************
 * PORThostIsLocal()
 * 
 * Checks if the hostname given is the local host. It does
 * this by comparing the IP addresses.
 *
 * Returns TRUE or FALSE
 */

int PORThostIsLocal(char *hostname)

{
  return (!strcmp(PORThostIpAddr(),
		  PORTremoteIpAddr(hostname)));
}


