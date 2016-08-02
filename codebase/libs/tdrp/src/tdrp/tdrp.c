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
 * tdrp.c
 *
 * API routines for public C TDRP functions.
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Sept 1998
 */

#include <tdrp/tdrp.h>

/*********************************************************
 * TDRP_usage()
 *
 * Prints out usage message for TDRP args as passed in to
 * TDRP_load_from_args()
 */

void TDRP_usage(FILE *out)

{
  tdrpUsage(out);
}

/*************************************************************************
 * TDRP_load_from_args()
 *
 * Loads up TDRP using the command line args for control.
 *
 * Check TDRP_usage() for command line actions associated with
 * thisbuffer function.
 *
 *   argc, argv: command line args
 *
 *   TDRPtable *table: table obtained from <mod>_tdrp_init().
 *
 *   void *params: thisbuffer is actually of type *<mod>_tdrp_struct,
 *     as declared in <mod>_tdrp.h.
 *     This function loads the values of the parameters into thisbuffer structure.
 * 
 *   char **override_list: A null-terminated list of overrides to the
 *     parameter file.
 *     An override string has exactly the format of the
 *     parameter file itself.
 *
 *   char **params_path_p: if non-NULL, thisbuffer is set to point to the
 *                         path of the params file used.
 *
 *  Returns 0 on success, -1 on failure.
 */

int TDRP_load_from_args(int argc, char **argv,
			TDRPtable *table, void *params,
			char **override_list,
			char **params_path_p)
     
{
  if (tdrpLoadFromArgs(argc, argv, table, params,
		       override_list, params_path_p)) {
    return (-1);
  } else {
    return (0);
  }
}

/*************************************************************************
 * TDRP_load()
 *
 * Loads up TDRP for a given module.
 *
 * This version of load gives the programmer the option to load up more
 * than one module for a single application. It is a lower-level
 * routine than tdrpLoadFromArgs(), and hence more flexible, but
 * the programmer must do more work.
 *
 *   char *param_file_path: the parameter file to be read in.
 *
 *   TDRPtable *table: table obtained from <mod>_tdrp_init().
 *
 *   void *params: thisbuffer is actually of type *<mod>_tdrp_struct,
 *     as declared in <mod>_tdrp.h.
 *     This function loads the values of the parameters into thisbuffer structure.
 * 
 *   char **override_list: A null-terminated list of overrides to the
 *     parameter file.
 *     An override string has exactly the format of the
 *     parameter file itself.
 *
 *  expand_env: flag to control environment variable expansion during
 *                tokenization.
 *              If TRUE, environment expansion is set on.
 *              If FALSE, environment expansion is set off.
 *
 *  Returns 0 on success, -1 on failure.
 */

int TDRP_load(const char *param_file_path,
	      TDRPtable *table, void *params,
	      char **override_list,
	      int expand_env, int debug)

{
  if (tdrpLoad(param_file_path, table, params,
	       override_list, expand_env, debug)) {
    return (-1);
  } else {
    return (0);
  }
}

/*************************************************************************
 * TDRP_load_from_buf()
 *
 * Loads up TDRP for a given module from a buffer.
 *
 * This version of load gives the programmer the option to load up more
 * than one module for a single application, using buffers which have
 * been read from an unspecified source.
 *
 *   char *param_source_str: a string which describes the source
 *     of the parameter information. It is used for error
 *     reporting only.
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
 *   char *inbuf: the input buffer
 *
 *   int inlen: length of the input buffer.
 *
 *   int start_line_num: the line number in the source which corresponds
 *     to the start of the buffer.
 * 
 *   expand_env: flag to control environment variable expansion during
 *                tokenization.
 *              If TRUE, environment expansion is set on.
 *              If FALSE, environment expansion is set off.
 *
 *  Returns 0 on success, -1 on failure.
 */

int TDRP_load_from_buf(const char *param_source_str, TDRPtable *table,
		       void *params, char **override_list,
		       const char *inbuf, int inlen,
		       int start_line_num,
		       int expand_env, int debug)

{
  if (tdrpLoadFromBuf(param_source_str, table, params,
		      override_list,
		      inbuf, inlen, start_line_num,
		      expand_env, debug)) {
    return (-1);
  } else {
    return (0);
  }
}

     
/**********************************************************
 * TDRP_load_defaults()
 *
 * Loads up TDRP for a given module using defaults only.
 *
 * See TDRP_load() for details.
 *
 * Returns 0 on success, -1 on failure.
 */

int TDRP_load_defaults(TDRPtable *table, void *params,
		       int expand_env)

{
  if (tdrpLoadDefaults(table, params, expand_env)) {
    return (-1);
  } else {
    return (0);
  }
}

/***********************************************************
 * TDRP_sync()
 *
 * Syncs the user struct data back into the parameter table,
 * in preparation for printing.
 */

void TDRP_sync(TDRPtable *table, void *params)
     
{
  tdrpUser2Table(table, params);
}

/**************
 * TDRP_print()
 * 
 * Print params file
 *
 * The modes supported are:
 *
 *   PRINT_SHORT:   main comments only, no help or descriptions
 *                  structs and arrays on a single line
 *   PRINT_NORM:    short + descriptions and help
 *   PRINT_LONG:    norm  + arrays and structs expanded
 *   PRINT_VERBOSE: long  + private params included
 */

void TDRP_print(FILE *out, TDRPtable *table,
		char *module, tdrp_print_mode_t mode)

{
  tdrpPrint(out, table, module, mode);
}
     
/***********************************************************
 * TDRP_check_all_set()
 *
 * Return TRUE if all set, FALSE if not.
 *
 * If out is non-NULL, prints out warning messages for those
 * parameters which are not set.
 */

int TDRP_check_all_set(FILE *out,
		       TDRPtable *table, void *params)
{
  return (tdrpCheckAllSet(out, table, params));
}

/***********************************************************
 * TDRP_free_all()
 *
 * Frees up memory associated with a module and its table
 */

void TDRP_free_all(TDRPtable *table, void *params)
     
{
  tdrpFreeAll(table, params);
}

/**********************
 * TDRP_array_realloc()
 *
 * Realloc 1D array.
 *
 * If size is increased, the values from the last array entry is
 * copied into the new space.
 *
 * Returns 0 on success, -1 on error.
 */

int TDRP_array_realloc(TDRPtable *table, void *params,
		       const char *param_name, int new_array_n)
     
{
  if (tdrpArrayRealloc(table, params, param_name, new_array_n)) {
    return (-1);
  } else {
    return (0);
  }
}

/************************
 * TDRP_array2D_realloc()
 *
 * Realloc 2D array.
 *
 * If size is increased, the values from the last array entry is
 * copied into the new space.
 *
 * Returns 0 on success, -1 on error.
 */

int TDRP_array2D_realloc(TDRPtable *table, void *params,
			 const char *param_name,
			 int new_array_n1, int new_array_n2)
     
{
  if (tdrpArray2DRealloc(table, params, param_name,
			 new_array_n1, new_array_n2)) {
    return (-1);
  } else {
    return (0);
  }
}

/**********************
 * TDRP_init_override()
 *
 * Initialize the override list
 */

void TDRP_init_override(tdrp_override_t *override)
{
  tdrpInitOverride(override);
}

/*********************
 * TDRP_add_override()
 *
 * Add a string to the override list
 */

void TDRP_add_override(tdrp_override_t *override, const char *override_str)
{
  tdrpAddOverride(override, override_str);
}

/**********************
 * TDRP_free_override()
 *
 * Free up the override list
 */

void TDRP_free_override(tdrp_override_t *override)
{
  tdrpFreeOverride(override);
}


/***********************************************************
 * TDRP_str_replace()
 *
 * Replace a string.
 */

void TDRP_str_replace(char **s1, const char *s2)
{
  tdrpStrReplace(s1, s2);
}

/***********************************************************
 * Tdrp_warn_if_exta_params
 * 
 * set flag for warning when there are extra params or not.
 * If value = TRUE warning is enabled.
 * If value = FALSE warning is disabled.
 */

void TDRP_warn_if_extra_params(int value)
{
  tdrpSetWarnOnExtraParams(value);
}
