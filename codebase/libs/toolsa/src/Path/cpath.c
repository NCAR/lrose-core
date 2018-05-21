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
/********************************************************************
 *
 * Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
 * April 2018
 *
 * C implementation of get executable path
 * From Bruon Melli, LROSE, CSU
 *
 ********************************************************************/

#if defined(__linux__)

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
  
#elif defined (__APPLE__)

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libproc.h>

#endif

char *get_exec_path()
{

#if defined(__linux__)

  int length;
  char full_path[PATH_MAX];
     
  length = readlink("/proc/self/exe", full_path, sizeof(full_path));
     
  if (length < 0)
    return NULL;
  if (length >= PATH_MAX)
    return NULL;
  full_path[length] = '\0';
  return strdup(full_path);

#elif defined (__APPLE__)

  pid_t pid; int ret;
  char path_buf[PROC_PIDPATHINFO_MAXSIZE];

  pid = getpid();
  ret = proc_pidpath (pid, path_buf, sizeof(path_buf));
  if ( ret <= 0 )
    return NULL;
  
  return strdup(path_buf);

#else

  return strdup("");

#endif

}

