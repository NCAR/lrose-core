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
 * apply_params.c
 *
 * Applies the params, and checks them for consistency.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Feb 1997
 * 
 * Returns 0 on success, -1 on failure.
 *
 ****************************************************************************/

#include "rdacs2gate.h"

int apply_params(void)

{
  
  int ret = 0;

  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  if (Glob->params.fields.len < 1 || Glob->params.fields.len > 3) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr,
	    "Parameter 'fields' must have between 1 and 3 members.\n");
    ret = -1;
  }

  if (strcmp(Glob->params.fields.val[0].type, "DBZ")) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "First field must be \"DBZ\"\n");
    ret = -1;
  }

  if (Glob->params.fields.len >= 2) {
    if (strcmp(Glob->params.fields.val[1].type, "VEL")) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Second field must be \"VEL\"\n");
      ret = -1;
    }
  }

  if (Glob->params.fields.len >= 3) {
    if (strcmp(Glob->params.fields.val[2].type, "WIDTH")) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Third field must be \"WIDTH\"\n");
      ret = -1;
    }
  }

  return (ret);
  
}

