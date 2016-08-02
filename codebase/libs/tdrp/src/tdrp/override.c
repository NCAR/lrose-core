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

/**********************************************************
 * override.c
 *
 * TDRP override functions.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307, USA
 *
 * May 1997
 */

#include <tdrp/tdrp.h>
#include <string.h>

/********************
 * tdrpInitOverride()
 *
 * Initialize the override list
 */

void tdrpInitOverride(tdrp_override_t *override)
{
  override->n = 0;
  (override->list) =
    (char **) tdrpMalloc ((override->n + 1) * sizeof(char *));
  (override->list)[override->n] = (char *) NULL;
}

/*******************
 * tdprAddOverride()
 *
 * Add a string to the override list
 */

void tdrpAddOverride(tdrp_override_t *override, const char *override_str)
{
  
  override->list[override->n] =
    (char *) tdrpMalloc(strlen(override_str) + 1); 
  strcpy(override->list[override->n], override_str);
    
  override->n++;
  override->list =
    (char **)tdrpRealloc((char *)override->list,
			 (override->n + 1) * sizeof(char *));
  override->list[override->n] = (char *) NULL;
}

/********************
 * tdrpFreeOverride()
 *
 * Free up the override list.
 */

void tdrpFreeOverride(tdrp_override_t *override)
{
  
  int i;

  for (i = 0; i < override->n; i++) {
    tdrpFree ((void *) override->list[i]);
  }
  tdrpFree ((void *) override->list);
  override->n = 0;
}

