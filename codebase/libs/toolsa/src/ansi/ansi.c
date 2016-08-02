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
 * ansi.c - missing ansi functions in SUNOS4
 *
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 * Sept 1994
 */

#ifdef SUNOS4

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <toolsa/ansi.h>

extern long int strtol(const char *, char **, int);

#define MAX_STATIC_LEN 256

void *memmove (void *s1, const void *s2, size_t n)

/*
 * copy s2[n] to s1[n] safely
 */

{

  char *sc1;
  const char *sc2;
  
  sc1 = s1;
  sc2 = s2;
  
  if (sc2 < sc1 && sc1 < sc2 + n) {

    for (sc1 += n, sc2 += n; 0 < n; --n) {
      *--sc1 = *--sc2; /* copy backwards */
    }

  } else {

    for (; 0 < n; --n) {
      *sc1++ = *sc2++;/* copy forwards */
    }

  }

  return (s1);

}


unsigned long strtoul (const char *str, char **ptr, int base)

{

  return ((unsigned long) strtol(str, ptr, base));

}

int atexit (void (*func)(void)) 

{

  /* return success of regsitration */
  
  return (0);
}

#else

void __dummy_function__(void) {

}

#endif
