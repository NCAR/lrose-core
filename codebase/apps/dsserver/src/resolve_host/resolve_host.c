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
 * resolve_host.c
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1996
 *
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <toolsa/port.h>
#include <netdb.h>
#include <sys/socket.h>

int main(int argc, char **argv)

{

  int i;
  struct hostent *hh;
  char **aliases;
  char **addr_list;
  char ipaddr[128];
  char *strp;
  
  if (argc < 2) {
    fprintf(stderr, "Usage: resolve_host hostname\n");
    return (-1);
  }

  fprintf(stderr, "Resolving host '%s'\n", argv[1]);

  if ((hh = gethostbyname(argv[1])) == NULL) {
    fprintf(stderr, "gethostbyname failed\n");
    switch (h_errno) {
    case HOST_NOT_FOUND:
      fprintf(stderr, "  HOST_NOT_FOUND\n");
      break;
    case TRY_AGAIN:
      fprintf(stderr, "  TRY_AGAIN\n");
      break;
    case NO_RECOVERY:
      fprintf(stderr, "  NO_RECOVERY\n");
      break;
    case NO_DATA:
      fprintf(stderr, "  NO_DATA\n");
      break;
    }
    return (-1);
  }


  fprintf(stderr, "h_name: %s\n", hh->h_name);

  aliases = hh->h_aliases;
  while (*aliases != NULL) {
    fprintf(stderr, "alias: %s\n", *aliases);
    aliases++;
  }
   
  if (hh->h_addrtype == AF_INET) {
    fprintf(stderr, "addrtype: AF_INET\n");
#ifdef AF_INET6
  } else if (hh->h_addrtype == AF_INET6) {
    fprintf(stderr, "addrtype: AF_INET6\n");
#endif
  } else {
    fprintf(stderr, "addrtype: UNKNOWN\n");
  }
  
  fprintf(stderr, "h_length: %d\n", hh->h_length);
  
  addr_list = hh->h_addr_list;
  while (*addr_list != NULL) {
    strp = ipaddr;
    for (i = 0; i < hh->h_length; i++) {
      sprintf (strp, "%d", (unsigned char) (*addr_list)[i]);
      strp = ipaddr + strlen(ipaddr);
      if (i < hh->h_length - 1) {
	*strp = '.';
	strp++;
      }
    }
    fprintf(stderr, "ipaddr: %s\n", ipaddr);
    addr_list++;
  }

  return (0);

}

