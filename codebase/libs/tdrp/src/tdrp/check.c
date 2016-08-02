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
 * check.c
 *
 * TDRP check functions.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307, USA
 *
 * May 1997
 */

#include <tdrp/tdrp.h>
#include <string.h>

/***********************************************************
 * tdrpCheckAllSet()
 *
 * Return TRUE if all set, FALSE if not.
 *
 * If out is non-NULL, prints out warning messages for those
 * parameters which are not set.
 */

int tdrpCheckAllSet(FILE *out, const TDRPtable *table, const void *params)
{
  int ret = TRUE;
  while (table->param_name != NULL) {
    if (table->ptype != COMMENT_TYPE &&
	!table->is_private &&
	!table->is_set) {

      /*
       * Exception - empty arrays, 2D and 1D - are OK - Niles Oien
       */

      if (
	  ((table->is_array2D) && (table->array_n1 == 0) && (table->array_n2 == 0)) ||
	  ((table->is_array)   && (table->array_n == 0))
	  ){
	table++;
	continue;
      }

      if (out) {
	fprintf(out,
		"TDRP_WARNING: parameter not set, using default - '%s'\n",
		table->param_name);
      }
      ret = FALSE;
    }
    table++;
  }
  return (ret);
}

/***********************************************************
 * tdrpCheckIsSet()
 *
 * Return TRUE if parameter is set, FALSE if not.
 *
 */

int tdrpCheckIsSet(const char *paramName, const TDRPtable *table, const void *params)
{
  int ret;

  while ( strcmp(table->param_name, paramName)) {
    table++;
  }

  if (table->ptype != COMMENT_TYPE && !table->is_private) {
    ret = table->is_set;
  }
  else {
    ret = FALSE;
  }
  return (ret);
}
