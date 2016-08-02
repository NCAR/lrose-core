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
/*********************************************************************
 * check_format.c
 *
 * check the entry format
 *
 * Returns the length of the data base chunk, -1 on ERROR
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "load_spdb.h"

static char *Format_type_str[FT_NUM] = {"int", "float", "string"};

static int Format_enums[FT_NUM] = {FT_INT,
				   FT_FLOAT,
				   FT_STRING};

int check_format(char *params_file)

{

  char *format_str;
  int i, j;
  int chunk_len;
  int format_found;
  int error_flag = FALSE;

  /*
   * allocate the global format type array
   */
  
  Glob->format_types =
    (int *) umalloc(Glob->params.data_format.len * sizeof(int));

  /*
   * initialize chunk len
   */

  chunk_len = 0;
  
  for (i = 0; i < Glob->params.data_format.len; i++) {
    
    format_str = Glob->params.data_format.val[i];
    
    format_found = FALSE;
    for (j = 0; j < FT_NUM; j++) {
      if (!strncmp(format_str, Format_type_str[j], strlen(format_str))) {
	Glob->format_types[i] = Format_enums[j];
	format_found = TRUE;
	break;
      }
    } /* j */

    if (format_found) {

      switch (Glob->format_types[i]) {
      case FT_INT:
	chunk_len += sizeof(si32);
	break;
      case FT_FLOAT:
	chunk_len += sizeof(fl32);
	break;
      case FT_STRING:
	chunk_len += Glob->params.string_len;
	break;
      } /* switch */      

      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stderr, "Format %d is %s\n", i, format_str);
      }

    } else {
      
      fprintf(stderr, "ERROR - %s:check_format\n", Glob->prog_name);
      fprintf(stderr, "Incorrect format type: %s\n",
	      format_str);
      fprintf(stderr, "Check params file %s\n", params_file);
      error_flag = TRUE;

    } /* if (format_found) */

  } /* i */

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stderr, "Chunk len: %d bytes\n", chunk_len);
  }

  if (error_flag) {
    return (-1);
  } else {
    return (chunk_len);
  }

}
