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

#ifndef TDRP_WAS_INCLUDED
#define TDRP_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>

/****************************************************
 * tdrp.h
 *
 * This is the public API for C-based TDRP routines.
 */

/*
 * boolean - defined to avoid conflicts
 */

typedef enum {
  pFALSE = 0, pTRUE = 1
} tdrp_bool_t;

/*
 * printing modes
 */
     
typedef enum {
  NO_PRINT, PRINT_SHORT, PRINT_NORM, PRINT_LONG, PRINT_VERBOSE
} tdrp_print_mode_t;

/*
 * override list - used to override params with command line
 * arg entries
 */

typedef struct {
  char **list;
  int n;
} tdrp_override_t;

/************************************
 * include tdrp_p.h, the private API.
 *
 * NOTE: programmers should only make use of the public API.
 * Do not make direct use of info in "tdrp_p.h"
 */

#include <tdrp/tdrp_p.h>

/*********************************************************
 * TDRP_usage()
 *
 * Prints out usage message for TDRP args as passed in to
 * TDRP_load_from_args()
 */

extern void TDRP_usage(FILE *out);

/*************************************************************************
 * TDRP_load_from_args()
 *
 * Loads up TDRP using the command line args for control.
 *
 * Check TDRP_usage() for command line actions associated with
 * this function.
 *
 *   argc, argv: command line args
 *
 *   TDRPtable *table: table obtained from <mod>_tdrp_init().
 *
 *   void *params: this is actually of type *<mod>_tdrp_struct,
 *     as declared in <mod>_tdrp.h.
 *     This function loads the values of the parameters into this structure.
 * 
 *   char **override_list: A null-terminated list of overrides to the
 *     parameter file.
 *     An override string has exactly the format of the
 *     parameter file itself.
 *
 *   char **params_path_p: if non-NULL, this is set to point to the
 *                         path of the params file used.
 *
 *  Returns 0 on success, -1 on failure.
 */

extern int TDRP_load_from_args(int argc, char **argv,
			       TDRPtable *table, void *params,
			       char **override_list,
			       char **params_path_p);

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
 *   void *params: this is actually of type *<mod>_tdrp_struct,
 *     as declared in <mod>_tdrp.h.
 *     This function loads the values of the parameters into this structure.
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

extern int TDRP_load(const char *param_file_path,
		     TDRPtable *table, void *params,
		     char **override_list,
		     int expand_env, int debug);

/*************************************************************************
 * TDRP_load_from_buf()
 *
 * Loads up TDRP for a given module from a buffer.
 *
 * This version of load gives the programmer the option to load up more
 * than one module for a single application, using buffers which have
 * been read from a specified source.
 *
 *   char *param_source_str: a string which describes the source
 *     of the parameter information. It is used for error
 *     reporting only.
 *
 *   TDRPtable *table: table obtained from <mod>_tdrp_init().
 *
 *   void *params: this is actually &<mod>_params,
 *     as declared in <mod>_tdrp.h.
 *     TDRP_read places the values of the parameters in this structure.
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

 extern int TDRP_load_from_buf(const char *param_source_str, TDRPtable *table,
			       void *params, char **override_list,
			       const char *inbuf, int inlen,
			       int start_line_num,
			       int expand_env, int debug);

/**********************************************************
 * TDRP_load_defaults()
 *
 * Loads up TDRP for a given module using defaults only.
 *
 * See TDRP_load() for details.
 *
 * Returns 0 on success, -1 on failure.
 */

extern int TDRP_load_defaults(TDRPtable *table, void *params,
			      int expand_env);

/***********************************************************
 * TDRP_sync()
 *
 * Syncs the user struct data back into the parameter table,
 * in preparation for printing.
 */

extern void TDRP_sync(TDRPtable *table, void *params);

/*************
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

extern void TDRP_print(FILE *out, TDRPtable *table,
		       char *module, tdrp_print_mode_t mode);
     
/***********************************************************
 * TDRP_check_all_set()
 *
 * Return TRUE if all set, FALSE if not.
 *
 * If out is non-NULL, prints out warning messages for those
 * parameters which are not set.
 */

extern int TDRP_check_all_set(FILE *out,
			      TDRPtable *table, void *params);

/***********************************************************
 * TDRP_free_all()
 *
 * Frees up memory associated with a module and its table
 */

extern void TDRP_free_all(TDRPtable *table, void *params);

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

extern int TDRP_array_realloc(TDRPtable *table, void *params,
			      const char *param_name, int new_array_n);

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

extern int TDRP_array2D_realloc(TDRPtable *table, void *params,
				const char *param_name,
				int new_array_n1, int new_array_n2);

/***********************************************************
 * TDRP_str_replace()
 *
 * Replace a string.
 */

extern void TDRP_str_replace(char **s1, const char *s2);

/**********************
 * TDRP_init_override()
 *
 * Initialize the override list
 */

extern void TDRP_init_override(tdrp_override_t *override);

/*********************
 * TDRP_add_override()
 *
 * Add a string to the override list
 */

extern void TDRP_add_override(tdrp_override_t *override,
			      const char *override_str);

/**********************
 * TDRP_free_override()
 *
 * Free up the override list
 */

extern void TDRP_free_override(tdrp_override_t *override);

/***********************************************************
 * tdrpCopyTable()
 * 
 * Copy one table to another.
 * The target table must be at least as large as the source
 * table.
 */

void tdrpCopyTable(const TDRPtable *source, TDRPtable *target);

/***********************************************************
 * Tdrp_warn_if_exta_params
 * 
 * set flag for warning when there are extra params or not.
 * If value = TRUE warning is enabled.
 * If value = FALSE warning is disabled.
 */

void TDRP_warn_if_extra_params(int value);

#ifdef __cplusplus
}
#endif

#endif /* TDRP_WAS_INCLUDED */

