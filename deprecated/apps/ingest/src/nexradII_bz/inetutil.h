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
/* 
 * Miscellaneous functions to make dealing with internet addresses easier.
 */

#ifndef _INETUTIL_H_
#define _INETUTIL_H_

#include <netinet/in.h>

#ifdef IPPROTO_IP /* we included netinet/in.h, so struct sockaddr_in is */
extern char *hostbyaddr(struct sockaddr_in *paddr);
extern int addrbyhost(const char *hostname, struct sockaddr_in *paddr);
extern char *s_sockaddr_in(struct sockaddr_in *paddr);
extern int gethostaddr_in(struct sockaddr_in *paddr);
#endif
extern int getservport(const char *servicename, const char *proto);
extern char *ghostname(void);
extern int usopen(const char *name);
extern int udpopen(const char *hostname, const char *servicename);
extern int isMe(const char *remote);
extern int local_sockaddr_in(struct sockaddr_in* addr);
extern int sockbind(const char *type, unsigned short port);

#endif /* !_INETUTIL_H_ */
