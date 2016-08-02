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

/******************************************************************
 * warn_on_extra_params.c
 *
 * TDRP warning when extra params are present.
 *
 * Dave Albo, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80303
 *
 * October 2009
 *
 */

#include <tdrp/tdrp.h>
#include <string.h>
/* #include <stdlib.h> */
/* #include <sys/stat.h> */

/*
 * Set do_warn = TRUE to do the warning, FALSE to not warn.
 */
static int do_warn = TRUE;

/*
 * Set do_warn flag to status passed in
 */
void tdrpSetWarnOnExtraParams(int value)
{
  do_warn = value;
}

/**********************************************************
 * Warn if there are parameters in the param file which do
 * not belong there.
 */

void tdrpWarnOnExtraParams(token_handle_t *handle,
			   const char *param_file_path,
			   TDRPtable *table,
			   tdrpToken_t *tokens, int ntok)
{
  if (do_warn == 0)
    return;
  
  int itok;
  int param_found = FALSE;
  int n_unpaired = 0;
  char *param_name;
  TDRPtable *tt = table;

  /*
   * search for all '=' characters outside braces
   * braces
   */
  
  for (itok = 0; itok < ntok; itok++) {

    if (!strcmp(tokens[itok].tok, "{")) {
      n_unpaired++;
    }

    if (!strcmp(tokens[itok].tok, "}")) {
      n_unpaired--;
    }

    if (n_unpaired == 0 && itok > 0 && !strcmp(tokens[itok].tok, "=")) {
      
      param_name = tokens[itok-1].tok;

      tt = table;
      param_found = FALSE;
      while (tt->param_name != NULL) {
	if (!strcmp(param_name, tt->param_name)) {
	  param_found = TRUE;
	  break;
	}
	tt++;
      } /* while */

      if (!param_found) {
	fprintf(stderr, "\n>>> TDRP_WARNING <<< - parameter '%s'\n",
		param_name);
	fprintf(stderr,	"    This parameter is not relevant.\n");
	fprintf(stderr,
		"    To suppress this warning, remove from file '%s'\n",
		param_file_path);
	fprintf(stderr, "    %s\n", tdrpLineInfo(handle, &tokens[itok-1]));
      }

    } /* if (n_unpaired == 0 ... */

  } /* itok */
      
}

