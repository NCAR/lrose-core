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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef PORT_WAS_INCLUDED
#define PORT_WAS_INCLUDED
#ifdef __APPLE__
#include <db.h>
#endif
#include <dirent.h>
#include <signal.h>

/**************************************
 * PORThostname()
 *
 * Short host name - no internet detail.
 *
 * Returns pointer to static memory - do not free.
 */

extern char *PORThostname(void);

/**************************************
 * PORThostnameFull()
 *
 * Fully qualified internet host name.
 *
 * Returns pointer to static memory - do not free.
 */

extern char *PORThostnameFull(void);

/****************************
 * PORThostIpAddr()
 * 
 * Returns IP address of host.
 *
 * Returns pointer to static memory - do not free.
 */

extern char *PORThostIpAddr(void);

/****************************
 * PORTremoteIpAddr()
 * 
 * Returns IP address of remote host.
 *
 * Returns pointer to static memory - do not free.
 */

extern char *PORTremoteIpAddr(char *remote_hostname);

/****************************
 * PORThostIsLocal()
 * 
 * Checks if the hostname given is the local host. It does
 * this by comparing the IP addresses.
 *
 * Returns TRUE or FALSE
 */

extern int PORThostIsLocal(char *hostname);

/***************
 * PORTsignal()
 *
 * cover for signal; ensures signals are "reliable"
 * specifies SA_RESTART, so interrupted system calls are restarted
 * unless its SIGALRM.
 * see Stevens p298 and p 396
 */

typedef void (*PORTsigfunc)(int);
extern PORTsigfunc PORTsignal(int signo, PORTsigfunc handler);

/***************
 * PORTscandir()
 *
 * cover for scandir which is not available on SUNOS4
 */
extern int PORTscandir(const char *dirp,
                       struct dirent ***namelist,
                       int (*filter)(const struct dirent *),
                       int (*compar)(const struct dirent **,
                                     const struct dirent **));

/***************
 * PORTalphasort()
 * cover for alphasort which is not available on SUNOS4
 */

extern int PORTalphasort(const struct dirent ** a,
                         const struct dirent ** b);


#endif
#ifdef __cplusplus
}
#endif
