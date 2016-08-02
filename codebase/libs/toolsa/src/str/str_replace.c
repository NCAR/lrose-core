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
#include <string.h>
#include <stdlib.h>
#include <toolsa/str.h>

char *STRreplace(char *string, 
		 char *old_substring, char *new_substring)
{
  char *return_string = NULL;
  char *substring_position;
  
  /*
   * Make sure the old substring exists in the string.
   */

  if ((substring_position = strstr(string, old_substring)) == (char *)NULL)
    return(NULL);
  
  /*
   * Allocate space for the new string.
   */

  return_string = (char *)malloc(strlen(string) -
				 strlen(old_substring) +
				 strlen(new_substring) + 1);
  
  /*
   * Now construct the new string.
   */

  strncpy(return_string, string,
	  (int)(substring_position - string));
  return_string[(int)(substring_position-string)] = '\0';
  strcat(return_string, new_substring);
  strcat(return_string, substring_position + strlen(old_substring));
  
  return(return_string);
}

