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
 * tdrp_obsolete.c
 *
 * API routines for obsolete C TDRP functions.
 *
 * These are included for backward compatibility only.
 * Do not use these functions, they will later be
 * removed from the library.
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Sept 1998
 */

#include <tdrp/tdrp.h>

/*************
 * TDRP_read()
 *
 *   char *in_file: the parameter file to read in.
 *
 *   TDRPtable *table: table obtained from <mod>_tdrp_init().
 *
 *   void *params: thisbuffer is actually &<mod>_params,
 *     as declared in <mod>_tdrp.h.
 *     TDRP_read places the values of the parameters in thisbuffer structure.
 * 
 *   char **override_list: A null-terminated list of overrides to the
 *     parameter file.
 *     An override string has exactly the format of the
 *     parameter file itself.
 *
 *  Returns TRUE on success, FALSE on failure.
 */

int TDRP_read(const char *param_file_path, TDRPtable *table,
	      void *params, char **override_list)

{

  if (tdrpLoad(param_file_path, table, params,
	       override_list, TRUE, FALSE)) {
    return (FALSE);
  } else {
    return (TRUE);
  }

}

/*********************
 * TDRP_print_params()
 * 
 * Print params file, with module name
 */

void TDRP_print_params(const TDRPtable *table, const void *params,
		       const char *module, int print_comments)
     
{

  if (print_comments) {
    tdrpPrint(stdout, table, module, PRINT_NORM);
  } else {
    tdrpPrint(stdout, table, module, PRINT_SHORT);
  }

  /*
   * suppress compiler warnings
   */

  params = 0;

}

/*********************
 * TDRP_print_struct()
 *
 * print out the values of the parameters in the <mod>_params structure.
 */

void TDRP_print_struct(TDRPtable *table, void *params)
{
  TDRP_print_params(table, params, "", FALSE);
}

/********************
 * TDRP_print_table()
 *
 * Debugging printout of table.
 */
void TDRP_print_table(TDRPtable *table)
{
  tdrpPrint(stdout, table, "", PRINT_NORM);
}

/*********************
 * TDRP_set_defaults()
 *
 * Set the parameters back to their default values.
 */

void TDRP_set_defaults(TDRPtable *table, void *params)
{
  /*
   * do nothing - suppress compiler warnings
   */
  table = NULL;
  params = NULL;
}

/*********************
 * TDRP_check_is_set()
 * Prints out warning messages if any parameters are not set.
 * Return TRUE if all set, FALSE if not.
 */

int TDRP_check_is_set(TDRPtable *table, void *params)
{
  return (tdrpCheckAllSet(stderr, table, params));
}

/**************************
 * TDRP_check_update_time()
 *
 * Return 1 if the parameter file has changed since the last time
 * the parameter structure was changed.
 * Return 0 if not changed.
 * Return -1 if error on opening file.
 */

int TDRP_check_update_time(char *in_file, void *params)
{
  /*
   * do nothing - suppress compiler warnings
   */
  in_file = NULL;
  params = NULL;
  return(0);
}
