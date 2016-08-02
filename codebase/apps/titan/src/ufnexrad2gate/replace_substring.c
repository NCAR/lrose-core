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
/***************************************************************************
 * replace_substring.c
 *
 * Replaces the first occurrence of a substring of a given string with a new 
 * substring.  Assumes that the resulting string will fit in the given
 * buffer.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1995
 *
 ****************************************************************************/

#include <string.h>

#include "replace_substring.h"

int replace_substring(char *string,
		      char *old_substring,
		      char *new_substring,
		      char *new_string)      /* RETURNED */
{
  char *substring_pos;
  
  strcpy(new_string, string);

  if ((substring_pos = strstr(new_string, old_substring))
      == (char *)NULL)
    return(FALSE);
  
  /*
   * copy the ending portion of the old string before inserting the
   * new substring in case the new substring is longer than the old one.
   */

  strcpy(substring_pos + strlen(new_substring),
	 substring_pos + strlen(old_substring));
  strncpy(substring_pos, new_substring,
	  strlen(new_substring));
  
  return(TRUE);
}

