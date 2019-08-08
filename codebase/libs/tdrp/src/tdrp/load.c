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
 * load.c
 *
 * TDRP load functions.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80303
 *
 * September 1998
 *
 */

#include <tdrp/tdrp.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/*
 * file scope functions
 */

static int do_load(token_handle_t *handle,
		   const char *param_file_path, TDRPtable *table,
		   int expand_env, int debug);
static int expand_for_single_val(tdrpVal_t *var);
static int expand_token(tdrpToken_t *token);
static int expand_all_vals(TDRPtable *table);
static void structFieldFromUser(void *source, tdrpVal_t *val, int ptype);
static void structFieldToUser(tdrpVal_t *val, void *target, int ptype);
/* static void warn_on_extra_params(token_handle_t *handle, */
/* 				 const char *param_file_path, */
/* 				 TDRPtable *table, */
/* 				 tdrpToken_t *tokens, int ntok); */

/*********************************************************
 * tdrpUsage()
 *
 * Prints out usage message for TDRP args as passed in to
 * TDRP_load_from_args()
 */

void tdrpUsage(FILE *out)

{

  fprintf(out, "%s",
	  "TDRP args: [options as below]\n"
	  "   [ -params/--params path ] specify params file path\n"
	  "   [ -check_params/--check_params] check which params are not set\n"
	  "   [ -print_params/--print_params [mode]] print parameters\n"
	  "     using following modes, default mode is 'norm'\n"
	  "       short:   main comments only, no help or descriptions\n"
	  "                structs and arrays on a single line\n"
	  "       norm:    short + descriptions and help\n"
	  "       long:    norm  + arrays and structs expanded\n"
	  "       verbose: long  + private params included\n"
	  "       short_expand:   short with env vars expanded\n"
	  "       norm_expand:    norm with env vars expanded\n"
	  "       long_expand:    long with env vars expanded\n"
	  "       verbose_expand: verbose with env vars expanded\n"
	  "   [ -tdrp_debug] debugging prints for tdrp\n"
	  "   [ -tdrp_usage] print this usage\n"
	  "\n");
  
}

/*********************************************************
 * tdrpIsArgValid()
 *
 * returns TRUE if the arg is a valid TDRP command line arg,
 *         FALSE otherwise.
 */

int tdrpIsArgValid(const char *arg)

{

  if (!strcmp(arg, "-params")) {
    return TRUE;
  }
  if (!strcmp(arg, "--params")) {
    return TRUE;
  }
  if (!strcmp(arg, "-print_params")) {
    return TRUE;
  }
  if (!strcmp(arg, "--print_params")) {
    return TRUE;
  }
  if (!strcmp(arg, "-check_params")) {
    return TRUE;
  }
  if (!strcmp(arg, "--check_params")) {
    return TRUE;
  }
  if (!strcmp(arg, "short")) {
    return TRUE;
  }
  if (!strcmp(arg, "norm")) {
    return TRUE;
  }
  if (!strcmp(arg, "long")) {
    return TRUE;
  }
  if (!strcmp(arg, "verbose")) {
    return TRUE;
  }
  if (!strcmp(arg, "short_expand")) {
    return TRUE;
  }
  if (!strcmp(arg, "norm_expand")) {
    return TRUE;
  }
  if (!strcmp(arg, "long_expand")) {
    return TRUE;
  }
  if (!strcmp(arg, "verbose_expand")) {
    return TRUE;
  }
  if (!strcmp(arg, "-tdrp_debug")) {
    return TRUE;
  }
  if (!strcmp(arg, "-tdrp_usage")) {
    return TRUE;
  }
  return FALSE;
}

/*********************************************************
 * tdrpIsArgValidN()
 *
 * returns number of tokens consumed by arg
 * returns zero if arg is not a valid tdrp command line arg.
 */

int tdrpIsArgValidN(const char *arg)

{

  if (!strcmp(arg, "-params")) {
    return 2;
  }
  if (!strcmp(arg, "--params")) {
    return 2;
  }
  if (!strcmp(arg, "-print_params")) {
    return 1;
  }
  if (!strcmp(arg, "--print_params")) {
    return 1;
  }
  if (!strcmp(arg, "-check_params")) {
    return 1;
  }
  if (!strcmp(arg, "--check_params")) {
    return 1;
  }
  if (!strcmp(arg, "short")) {
    return 1;
  }
  if (!strcmp(arg, "norm")) {
    return 1;
  }
  if (!strcmp(arg, "long")) {
    return 1;
  }
  if (!strcmp(arg, "verbose")) {
    return 1;
  }
  if (!strcmp(arg, "short_expand")) {
    return 1;
  }
  if (!strcmp(arg, "norm_expand")) {
    return 1;
  }
  if (!strcmp(arg, "long_expand")) {
    return 1;
  }
  if (!strcmp(arg, "verbose_expand")) {
    return 1;
  }
  if (!strcmp(arg, "-tdrp_debug")) {
    return 1;
  }
  if (!strcmp(arg, "-tdrp_usage")) {
    return 1;
  }
  return 0;
}

/*************************************************************************
 * tdrpLoadFromArgs()
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
 *   void *params: this is actually &<mod>_params,
 *     as declared in <mod>_tdrp.h.
 *     TDRP_read places the values of the parameters in this structure.
 * 
 *   char **override_list: A null-terminated list of overrides to the
 *     parameter file.
 *     An override string has exactly the format of the
 *     parameter file itself.
 *
 *   char **params_path_p:
 *    If this is non-NULL, it is set to point to the path of the
 *    params file used.
 *
 *  Returns 0 on success, -1 on failure.
 */

int tdrpLoadFromArgs(int argc, char **argv,
		     TDRPtable *table,
		     void *params,
		     char **override_list,
		     char **params_path_p)

{
  int exit_deferred;
  return (_tdrpLoadFromArgs(argc, argv, table, params,
			    override_list, params_path_p, argv[0],
			    FALSE, &exit_deferred));
}


/*************************************************************************
 * _tdrpLoadFromArgs()
 *
 * Same as tdrpLoadFromArgs, with deferred exit option.
 *
 *   char *module:  module name
 *
 *   defer_exit: if true, and an exit would have occurred, the exit
 *               is deferred, and the exit_deferred_p flag is set.
 *               This allows the calling program to exit at its
 *               discretion later on.
 *
 *  *exit_deferred_p: see defer_exit.
 */

int _tdrpLoadFromArgs(int argc, char **argv,
		      TDRPtable *table,
		      void *params,
		      char **override_list,
		      char **params_path_p,
		      const char *module,
		      int defer_exit,
		      int *exit_deferred_p)

{

  char *params_path = NULL;
  int i;
  int iret = 0;
  int usage = FALSE;
  int debug = FALSE;
  int expand_env = TRUE;
  int check_params = FALSE;
  tdrp_print_mode_t print_mode = NO_PRINT;

  *exit_deferred_p = FALSE;
  
  /*
   * look for command options
   */
  
  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-params") ||
        !strcmp(argv[i], "--params")) {
      
      if (i < argc - 1) {
	params_path = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-print_params") ||
               !strcmp(argv[i], "--print_params")) {
      
      print_mode = PRINT_LONG;
      expand_env = FALSE;

      if (i < argc - 1) {

	if (!strcmp(argv[i+1], "short")) {
	  print_mode = PRINT_SHORT;
	} else if (!strcmp(argv[i+1], "norm")) {
	  print_mode = PRINT_NORM;
	} else if (!strcmp(argv[i+1], "long")) {
	  print_mode = PRINT_LONG;
	} else if (!strcmp(argv[i+1], "verbose")) {
	  print_mode = PRINT_VERBOSE;
	} else if (!strcmp(argv[i+1], "short_expand")) {
	  print_mode = PRINT_SHORT;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "norm_expand")) {
	  print_mode = PRINT_NORM;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "long_expand")) {
	  print_mode = PRINT_LONG;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "verbose_expand")) {
	  print_mode = PRINT_VERBOSE;
	  expand_env = TRUE;
	}

      }
	
    } else if (!strcmp(argv[i], "-check_params") ||
               !strcmp(argv[i], "--check_params")) {
      
      check_params = TRUE;
      
    } else if (!strcmp(argv[i], "-tdrp_debug")) {
      
      debug = TRUE;
      
    } else if (!strcmp(argv[i], "-tdrp_usage")) {
      
      usage = TRUE;
      
    } /* if */
    
  } /* i */

  if (usage) {
    tdrpUsage(stdout);
    if (defer_exit) {
      *exit_deferred_p = TRUE;
    } else {
      exit(0);
    }
  }

  if (iret) {
    tdrpUsage(stderr);
    return (-1);
  }

  if (params_path_p) {
    *params_path_p = params_path;
  }

  if (tdrpLoad(params_path, table, params,
	       override_list, expand_env, debug)) {
    return (-1);
  }

  if (print_mode != NO_PRINT) {
    if (!strcmp(module, "Params")) {
      tdrpPrint(stdout, table, argv[0], print_mode);
    } else {
      tdrpPrint(stdout, table, module, print_mode);
    }
  }

  if (check_params) {
    if (TDRP_check_all_set(stdout, table, params)) {
      fprintf(stdout, "TDRP - all parameters are set.\n");
    }
  }
    
  fflush(stdout);

  if (print_mode != NO_PRINT || check_params) {
    tdrpFreeAll(table, params);
    if (defer_exit) {
      *exit_deferred_p = TRUE;
    } else {
      exit(0);
    }
  }

  return (0);

}

/*************************************************************************
 * tdrpLoadApplyArgs()
 *
 * Loads up TDRP using the params path passed in, and applies the
 * command line args for printing and checking.
 *
 * Check TDRP_usage() for command line actions associated with
 * this function.
 *
 *   char *param_file_path: the parameter file to be read in.
 *
 *   argc, argv: command line args
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
 *   char *module:  module name
 *
 *   defer_exit: if true, and an exit would have occurred, the exit
 *               is deferred, and the exit_deferred_p flag is set.
 *               This allows the calling program to exit at its
 *               discretion later on.
 *
 *  *exit_deferred_p: see defer_exit.
 *
 *  Returns 0 on success, -1 on failure.
 */

int tdrpLoadApplyArgs(const char *params_path,
		      int argc, char **argv,
		      TDRPtable *table,
		      void *params,
		      char **override_list,
		      const char *module,
		      int defer_exit,
		      int *exit_deferred_p)

{

  int i;
  int iret = 0;
  int usage = FALSE;
  int debug = FALSE;
  int expand_env = TRUE;
  int check_params = FALSE;
  tdrp_print_mode_t print_mode = NO_PRINT;

  *exit_deferred_p = FALSE;
  
  /*
   * look for command options
   */
  
  iret = FALSE;
  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-print_params") ||
        !strcmp(argv[i], "--print_params")) {
      
      print_mode = PRINT_NORM;
      expand_env = FALSE;

      if (i < argc - 1) {

	if (!strcmp(argv[i+1], "short")) {
	  print_mode = PRINT_SHORT;
	} else if (!strcmp(argv[i+1], "norm")) {
	  print_mode = PRINT_NORM;
	} else if (!strcmp(argv[i+1], "long")) {
	  print_mode = PRINT_LONG;
	} else if (!strcmp(argv[i+1], "verbose")) {
	  print_mode = PRINT_VERBOSE;
	} else if (!strcmp(argv[i+1], "short_expand")) {
	  print_mode = PRINT_SHORT;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "norm_expand")) {
	  print_mode = PRINT_NORM;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "long_expand")) {
	  print_mode = PRINT_LONG;
	  expand_env = TRUE;
	} else if (!strcmp(argv[i+1], "verbose_expand")) {
	  print_mode = PRINT_VERBOSE;
	  expand_env = TRUE;
	}

      }
	
    } else if (!strcmp(argv[i], "-check_params") ||
               !strcmp(argv[i], "--check_params")) {
      
      check_params = TRUE;
      
    } else if (!strcmp(argv[i], "-tdrp_debug")) {
      
      debug = TRUE;
      
    } else if (!strcmp(argv[i], "-tdrp_usage")) {
      
      usage = TRUE;
      
    } /* if */
    
  } /* i */

  if (usage) {
    tdrpUsage(stdout);
    if (defer_exit) {
      *exit_deferred_p = TRUE;
    } else {
      exit(0);
    }
  }

  // remove dead code-- iret == 0
  //if (iret != 0) {
  //  tdrpUsage(stderr);
  //  return (-1);
  //}

  if (tdrpLoad(params_path, table, params,
	       override_list, expand_env, debug)) {
    return (-1);
  }

  if (print_mode != NO_PRINT) {
    if (!strcmp(module, "Params")) {
      tdrpPrint(stdout, table, argv[0], print_mode);
    } else {
      tdrpPrint(stdout, table, module, print_mode);
    }
  }

  if (check_params) {
    if (TDRP_check_all_set(stdout, table, params)) {
      fprintf(stdout, "TDRP - all parameters are set.\n");
    }
  }
    
  fflush(stdout);

  if (print_mode != NO_PRINT || check_params) {
    tdrpFreeAll(table, params);
    if (defer_exit) {
      *exit_deferred_p = TRUE;
    } else {
      exit(0);
    }
  }

  return (0);

}

/*************************************************************************
 * tdrpLoad()
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
 *   void *params: this is actually &<mod>_params,
 *     as declared in <mod>_tdrp.h.
 *     TDRP_read places the values of the parameters in this structure.
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

int tdrpLoad(const char *param_file_path, TDRPtable *table,
	     void *params, char **override_list,
	     int expand_env, int debug)

{

  int iret = 0;
  token_handle_t handle;

  /*
   * free up user space
   */

  tdrpFreeUser(table, params);

  /*
   * tokenization of parameter file
   */
  
  tdrpInitTokenize(&handle);
  if (tdrpInitFileForTokens(&handle, param_file_path, override_list)) {
    return (-1);
  }

  if (tdrpTokenize(&handle)) {
    iret = -1;
  }

  /*
   * do the load
   */

  if (do_load(&handle, param_file_path, table, expand_env, debug)) {
    iret = -1;
  }
  
  /*
   * free up tokens
   */
  
  tdrpFreeTokenize(&handle);

  if (iret) {
    return (-1);
  } else {
    /*
     * copy table info to user struct
     */
    tdrpTable2User(table, params);
    return (0);
  }

}

/*************************************************************************
 * tdrpLoadFromBuf()
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

int tdrpLoadFromBuf(const char *param_source_str, TDRPtable *table,
		    void *params, char **override_list,
		    const char *inbuf, int inlen,
		    int start_line_num,
		    int expand_env, int debug)
     
{

  int iret = 0;
  token_handle_t handle;

  /*
   * free up user space
   */

  tdrpFreeUser(table, params);

  /*
   * tokenization of parameter file
   */
  
  tdrpInitTokenize(&handle);
  tdrpInitBufForTokens(&handle, inbuf, inlen, start_line_num, override_list);
  if (tdrpTokenize(&handle)) {
    iret = -1;
  }

  /*
   * do the load
   */

  if (do_load(&handle, param_source_str, table, expand_env, debug)) {
    iret = -1;
  }
  
  /*
   * free up tokens
   */
  
  tdrpFreeTokenize(&handle);

  if (iret) {
    return (-1);
  } else {
    /*
     * copy table info to user struct
     */
    tdrpTable2User(table, params);
    return (0);
  }

}

/**********************************************************
 * tdrpLoadDefaults()
 *
 * Loads up TDRP for a given module using defaults only.
 *
 * See tdrpLoad() for details.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpLoadDefaults(TDRPtable *table, void *params, int expand_env)

{

  return (tdrpLoad(NULL, table, params,
		   NULL, expand_env, FALSE));

}

/***********************************************************
 * tdrpTable2User()
 *
 * Copy the parameter info in the table over into user space.
 */

void tdrpTable2User(TDRPtable *table, void *params)

{

  int i, j;
  TDRPtable *tt = table;
  void **array_p = NULL;
  void ***array2D_p;

  while (tt->param_name != NULL) {
      
    /*
     * check memory allocation for arrays
     */
    
    if (tt->is_array) {

      /* 1D allocation */

      array_p = (void **) ((char *) params + tt->array_offset);
      if (*array_p == NULL) {
	*array_p = (void *) tdrpCalloc(tt->array_n, tt->array_elem_size);
	/* *((void **) ((char *) params + tt->val_offset))= *array_p; */
      }

      /* 1D array length */

      *((int *) ((char *) params + tt->len_offset)) = tt->array_n;
      *((int *) ((char *) params + tt->array_n_offset)) = tt->array_n;

      if (tt->is_array2D) {

	/* 2D allocation and set 2D pointers */

	array2D_p = (void ***) ((char *) params + tt->array2D_offset);
	if (*array2D_p == NULL) {
	  *array2D_p = (void **) tdrpCalloc(tt->array_n1, sizeof(void *));
	  for (j = 0; j < tt->array_n1; j++) {
	    (*array2D_p)[j] = (char *) *array_p +
	      j * tt->array_n2 * tt->array_elem_size;
	  }
	}

	/* 2D array length */

	*((int *) ((char *)params + tt->array_n1_offset)) = tt->array_n1;
	*((int *) ((char *)params + tt->array_n2_offset)) = tt->array_n2;

      }

    }

    switch (tt->ptype) {
      
    case BOOL_TYPE:
      if (tt->is_array) {
	tdrp_bool_t *bptr = (tdrp_bool_t *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  bptr[i] = tt->array_vals[i].b;
	}
      } else {
	*((tdrp_bool_t *) ((char *) params + tt->val_offset)) =
	  tt->single_val.b;
      }
      break;
      
    case INT_TYPE:
      if (tt->is_array) {
	int *iptr = (int *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  iptr[i] = tt->array_vals[i].i;
	}
      } else {
	*((int *) ((char *) params + tt->val_offset)) =
	  tt->single_val.i;
      }
      break;
      
    case LONG_TYPE:
      if (tt->is_array) {
	long *lptr = (long *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  lptr[i] = tt->array_vals[i].l;
	}
      } else {
	*((long *) ((char *) params + tt->val_offset)) =
	  tt->single_val.l;
      }
      break;
      
    case FLOAT_TYPE:
      if (tt->is_array) {
	float *fptr = (float *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  fptr[i] = tt->array_vals[i].f;
	}
      } else {
	*((float *) ((char *) params + tt->val_offset)) =
	  tt->single_val.f;
      }
      break;
      
    case DOUBLE_TYPE:
      if (tt->is_array) {
	double *dptr = (double *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  dptr[i] = tt->array_vals[i].d;
	}
      } else {
	*((double *) ((char *) params + tt->val_offset)) =
	  tt->single_val.d;
      }
      break;
      
    case STRING_TYPE:
      if (tt->is_array) {
	char **cptr = (char **) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tdrpStrReplace(&cptr[i], tt->array_vals[i].s);
	}
      } else {
	char **cptr = (char **) ((char *) params + tt->val_offset);
	tdrpStrReplace(cptr, tt->single_val.s);
      }
      break;
      
    case ENUM_TYPE:
      if (tt->is_array) {
	int *iptr = (int *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  iptr[i] = tt->array_vals[i].e;
	}
      } else {
	*((int *) ((char *) params + tt->val_offset)) = tt->single_val.e;
      }
      break;
      
    case STRUCT_TYPE:

      if (tt->is_array) {
	
	void **val_ptr = (void **) ((char *) params + tt->array_offset);
	
	for (i = 0; i < tt->array_n; i++) {
	  for (j = 0; j < tt->struct_def.nfields; j++) {
	    int index = i * tt->struct_def.nfields + j;
	    int field_offset =
	      i * tt->array_elem_size +
	      tt->struct_def.fields[j].rel_offset;
	    structFieldToUser(tt->struct_vals + index,
			      (char *) (*val_ptr) + field_offset,
			      tt->struct_def.fields[j].ptype);
	  } /* j */
	} /* i */

      } else {
	
	for (j = 0; j < tt->struct_def.nfields; j++) {
	  int field_offset =
	    tt->val_offset + tt->struct_def.fields[j].rel_offset;
	  structFieldToUser(tt->struct_vals + j,
			    (char *) params + field_offset,
			    tt->struct_def.fields[j].ptype);
	}

      }

      break;
      
    case COMMENT_TYPE:
      break;
      
    } /* switch */
    
    tt++;
  
  } /* while */

}

/***********************************************************
 * tdrpUser2Table()
 *
 * Copy the user struct data back into the parameter table
 */

void tdrpUser2Table(TDRPtable *table, void *params)

{

  int i, j;
  TDRPtable *tt = table;
  void **array_p = NULL;

  while (tt->param_name != NULL) {
      
    if (tt->is_array) {
      array_p = (void **) ((char *) params + tt->array_offset);
    }

    switch (tt->ptype) {
      
    case BOOL_TYPE:
      if (tt->is_array) {
	tdrp_bool_t *bptr = (tdrp_bool_t *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].b = bptr[i];
	}
      } else {
	tt->single_val.b =
	  *((tdrp_bool_t *) ((char *) params + tt->val_offset));
      }
      break;
      
    case INT_TYPE:
      if (tt->is_array) {
	int *iptr = (int *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].i = iptr[i];
	}
      } else {
	tt->single_val.i =
	  *((int *) ((char *) params + tt->val_offset));
      }
      break;
      
    case LONG_TYPE:
      if (tt->is_array) {
	long *lptr = (long *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].l = lptr[i];
	}
      } else {
	tt->single_val.l =
	  *((long *) ((char *) params + tt->val_offset));
      }
      break;
      
    case FLOAT_TYPE:
      if (tt->is_array) {
	float *fptr = (float *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].f = fptr[i];
	}
      } else {
	tt->single_val.f =
	  *((float *) ((char *) params + tt->val_offset));
      }
      break;
      
    case DOUBLE_TYPE:
      if (tt->is_array) {
	double *dptr = (double *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].d = dptr[i];
	}
      } else {
	tt->single_val.d =
	  *((double *) ((char *) params + tt->val_offset));
      }
      break;
      
    case STRING_TYPE:
      if (tt->is_array) {
	char **cptr = (char **) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tdrpStrReplace(&tt->array_vals[i].s, cptr[i]);
	}
      } else {
	char **cptr = (char **) ((char *) params + tt->val_offset);
	tdrpStrReplace(&tt->single_val.s, *cptr);
      }
      break;
      
    case ENUM_TYPE:
      if (tt->is_array) {
	int *iptr = (int *) *array_p;
	for (i = 0; i < tt->array_n; i++) {
	  tt->array_vals[i].e = iptr[i];
	}
      } else {
	tt->single_val.e =
	  *((int *) ((char *) params + tt->val_offset));
      }
      break;
      
    case STRUCT_TYPE:

      if (tt->is_array) {
	
	void **val_ptr = (void **) ((char *) params + tt->array_offset);

	for (i = 0; i < tt->array_n; i++) {
	  for (j = 0; j < tt->struct_def.nfields; j++) {
	    int index = i * tt->struct_def.nfields + j;
	    int field_offset =
	      i * tt->array_elem_size +
	      tt->struct_def.fields[j].rel_offset;
	    structFieldFromUser((char *) (*val_ptr) + field_offset,
				tt->struct_vals + index,
				tt->struct_def.fields[j].ptype);
	  } /* j */
	} /* i */
	
      } else {
	
	for (j = 0; j < tt->struct_def.nfields; j++) {
	  int field_offset =
	    tt->val_offset + tt->struct_def.fields[j].rel_offset;
	  structFieldFromUser((char *) params + field_offset,
			      tt->struct_vals + j,
			      tt->struct_def.fields[j].ptype);
	}

      }
      break;
      
    case COMMENT_TYPE:
      break;
      
    } /* switch */
    
    tt++;
  
  } /* while */

}

/*********************************************
 * perform the load
 */

static int do_load(token_handle_t *handle,
		   const char *param_file_path, TDRPtable *table,
		   int expand_env, int debug)
     
{

  tdrpToken_t *tokens;
  TDRPtable *tt = table;
  int ntok;
  int param_tok;
  int iret = 0;

  tokens = tdrpTokens(handle);
  ntok = tdrpNtok(handle);

  /*
   * substitute env if appropriate
   */
  
  if (expand_env) {
    int i;
    for (i = 0; i < ntok; i++) {
      if (expand_token(tokens + i)) {
	return -1;
      }
    }
  }

  /*
   * debug print
   */
  
  if (debug) {
    int i;
    for (i = 0; i < ntok; i++) {
      if (tokens[i].is_string) {
	fprintf(stderr, "Token # %4d, line %4d: \"%s\"\n",
		i, tokens[i].line_num, tokens[i].tok);
      } else {
	fprintf(stderr, "Token # %4d, line %4d: %s\n",
		i, tokens[i].line_num, tokens[i].tok);
      }
    } /* i */
  }
  
  /*
   * read the params - use the last instance of the 
   * params in the token array.
   */
  
  iret = 0;
  while (tt->param_name != NULL) {
    
    if (tdrpFindParamLast(0, ntok - 1, tokens, ntok,
			  tt->param_name, &param_tok) == 0) {
      
      if (tt->is_private) {
	fprintf(stderr, "\n>>> TDRP_WARNING - do_load <<<\n");
	fprintf(stderr, "  param '%s'\n", tt->param_name);
	fprintf(stderr, "  Parameter is private.\n");
	fprintf(stderr, "  You cannot override default in param file.\n");
	fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[param_tok]));
	fprintf(stderr, "  Will be ignored.\n");
	tt++;
        continue;
      }
      
      tt->start_tok = param_tok;
      tt->end_tok = ntok - 1;
      
      switch (tt->ptype) {
	
      case BOOL_TYPE:
	if (tdrpReadBool(handle, tt, tt->param_name,
			 tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case INT_TYPE:
	if (tdrpReadInt(handle, tt, tt->param_name,
			tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case LONG_TYPE:
	if (tdrpReadLong(handle, tt, tt->param_name,
			 tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case FLOAT_TYPE:
	if (tdrpReadFloat(handle, tt, tt->param_name,
			  tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case DOUBLE_TYPE:
	if (tdrpReadDouble(handle, tt, tt->param_name,
			   tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case STRING_TYPE:
	if (tdrpReadString(handle, tt, tt->param_name,
			   tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case ENUM_TYPE:
	if (tdrpReadEnum(handle, tt, tt->param_name,
			 tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case STRUCT_TYPE:
	if (tdrpReadStruct(handle, tt, tt->param_name, tokens, ntok, expand_env)) {
	  iret = -1;
	}
	break;
	
      case COMMENT_TYPE:
	break;
	
      } /* switch */
      
    }

    if (tdrpCheckValRange(tt)) {
      iret = -1;
    }
    
    tt++;
    
  } /* while */
  
  /*
   * check that there are no stray params in the file which
   * have not been set
   */
  
  tdrpWarnOnExtraParams(handle, param_file_path, table, tokens, ntok);
/*   warn_on_extra_params( */
  
  /*
   * substitute env if appropriate
   */

  if (expand_env) {
    if (expand_all_vals(table)) {
      iret = -1;
    }
  }

  return (iret);

}

/******************************************************************
 * expand_all_vals()
 *
 * Expand environment variables into string type for
 * the whole table.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

static int expand_all_vals(TDRPtable *table)

{

  int i, j, index;
  TDRPtable *tt = table;
  int iret = 0;

  while (tt->param_name != NULL) {
    
    if (tt->ptype == STRING_TYPE) {
      
      if (tt->is_array) {
	for (i = 0; i < tt->array_n; i++) {
	  iret |= expand_for_single_val(tt->array_vals + i);
	}
      } else {
	iret |= expand_for_single_val(&tt->single_val);
      }

    } else if (tt->ptype == STRUCT_TYPE) {

      if (tt->is_array) {

	for (j = 0; j <  tt->array_n; j++) {
	  for (i = 0; i < tt->struct_def.nfields; i++) {
	    index = j * tt->struct_def.nfields + i;
	    if (tt->struct_def.fields[i].ptype == STRING_TYPE) {
	      iret |= expand_for_single_val(tt->struct_vals + index);
	    }
	  } /* i */
	} /* j */

      } else {

	for (i = 0; i < tt->struct_def.nfields; i++) {
	  if (tt->struct_def.fields[i].ptype == STRING_TYPE) {
	    iret |= expand_for_single_val(tt->struct_vals + i);
	  } /* i */
	}
	    
      }

    } /* if (tt->ptype == STRING_TYPE) */
    
    tt++;

  } /* while */
  
  if (iret) {
    return (-1);
  } else {
    return (0);
  }

}

/******************************************************************
 * expand_for_single_val()
 *
 * Expand environment variables into string type for the given
 * table entry.
 *
 * The first env variable found is expanded.
 * The env variable must be in the $(ENV_VAR) format.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

static int expand_for_single_val(tdrpVal_t *val)

{

  char work_str[TDRP_LINE_MAX];
  char combo_str[TDRP_LINE_MAX];
  char env_cpy[TDRP_LINE_MAX];

  char *dollar_bracket;
  char *closing_bracket;
  char *pre_str;
  char *env_str;
  char *env_val;
  char *post_str;

  int pre_len, env_len, post_len, tot_len;
  int env_found = FALSE;
  int iret = 0;

  /*
   * copy in the string variable
   */

  tdrpStrNcopy(work_str, val->s, TDRP_LINE_MAX);

  /*
   * look for opening '$(' sequence
   */
  
  while ((dollar_bracket = strstr(work_str, "$(")) != NULL) {
    
    memset (env_cpy, 0, TDRP_LINE_MAX);
    pre_str = work_str;
    env_str = dollar_bracket + 2;
    
    if ((closing_bracket = strchr(env_str, ')')) == NULL) {
      
      /*
       * no closing bracket
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_for_single_val\n");
      fprintf(stderr, "No closing bracket for env variable\n");
      fprintf(stderr, "Expanding string '%s'", work_str);
      /* iret = -1; */
      break;
      
    } /* if ((closing_bracket = ... */
    
    post_str = closing_bracket + 1;
    
    /*
     * load up env string
     */
    
    strncpy(env_cpy, env_str, (int) (closing_bracket - env_str));
    
    /*
     * get env val
     */
    
    if ((env_val = getenv(env_cpy)) == NULL) {
      
      /*
       * no env variable set 
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_for_single_val\n");
      fprintf(stderr, "Env variable '%s' not set\n", env_cpy);
      /* iret = -1; */
      break;
      
    }
    
    /*
     * compute total length after substitution
     */
    
    pre_len = (int) (dollar_bracket - pre_str);
    env_len = strlen(env_val);
    post_len = strlen(post_str);
    
    tot_len = pre_len + env_len + post_len + 1;
    
    if (tot_len >= TDRP_LINE_MAX) {
      
      /*
       * expanded string too long
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_for_single_val\n");
      fprintf(stderr, "Env str too long.\n");
      fprintf(stderr, "Expanding string '%s'", work_str);
      /* iret = -1; */
      break;
      
    } /* if (tot_len > TDRP_LINE_MAX) */
    
    /*
     * set combination str and copy over
     */
    
    *dollar_bracket = '\0';
    snprintf(combo_str, TDRP_LINE_MAX, "%s%s%s", pre_str, env_val, post_str);
    strncpy(work_str, combo_str, TDRP_LINE_MAX-1);
    env_found = TRUE;
    
  } /* while */

  /*
   * copy in expanded string
   */
  
  if (env_found) {
    tdrpFree(val->s);
    val->s = tdrpStrDup(work_str);
  }

  return iret;

}

/******************************************************************
 * expand_token()
 *
 * Expand environment variables in the given token
 *
 * The env variable must be in the $(ENV_VAR) format.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

static int expand_token(tdrpToken_t *token)

{

  char work_str[TDRP_LINE_MAX];
  char combo_str[TDRP_LINE_MAX];
  char env_cpy[TDRP_LINE_MAX];

  char *dollar_bracket;
  char *closing_bracket;
  char *pre_str;
  char *env_str;
  char *env_val;
  char *post_str;

  int pre_len, env_len, post_len, tot_len;
  int env_found = FALSE;
  int iret = 0;

  /*
   * copy in the string variable
   */

  tdrpStrNcopy(work_str, token->tok, TDRP_LINE_MAX);

  /*
   * look for opening '$(' sequence
   */
  
  while ((dollar_bracket = strstr(work_str, "$(")) != NULL) {
    
    memset (env_cpy, 0, TDRP_LINE_MAX);
    pre_str = work_str;
    env_str = dollar_bracket + 2;
    
    if ((closing_bracket = strchr(env_str, ')')) == NULL) {
      
      /*
       * no closing bracket
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_token\n");
      fprintf(stderr, "No closing bracket for env variable\n");
      fprintf(stderr, "Expanding string '%s'", work_str);
      /* iret = -1; */
      break;
      
    } /* if ((closing_bracket = ... */
    
    post_str = closing_bracket + 1;
    
    /*
     * load up env string
     */
    
    strncpy(env_cpy, env_str, (int) (closing_bracket - env_str));
    
    /*
     * get env val
     */
    
    if ((env_val = getenv(env_cpy)) == NULL) {
      
      /*
       * no env variable set 
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_token\n");
      fprintf(stderr, "Env variable '%s' not set\n", env_cpy);
      /* iret = -1; */
      break;
      
    }
    
    /*
     * compute total length after substitution
     */
    
    pre_len = (int) (dollar_bracket - pre_str);
    env_len = strlen(env_val);
    post_len = strlen(post_str);
    
    tot_len = pre_len + env_len + post_len + 1;
    
    if (tot_len >= TDRP_LINE_MAX) {
      
      /*
       * expanded string too long
       */
      
      fprintf(stderr, "\n>>> TDRP_WARNING <<< - expand_token\n");
      fprintf(stderr, "Env str too long.\n");
      fprintf(stderr, "Expanding string '%s'", work_str);
      /* iret = -1; */
      break;
      
    } /* if (tot_len > TDRP_LINE_MAX) */
    
    /*
     * set combination str and copy over
     */
    
    *dollar_bracket = '\0';
    snprintf(combo_str, TDRP_LINE_MAX, "%s%s%s", pre_str, env_val, post_str);
    strncpy(work_str, combo_str, TDRP_LINE_MAX-1);
    env_found = TRUE;
    
  } /* while */

  /*
   * copy in expanded string
   */
  
  if (env_found) {
    tdrpFree(token->tok);
    token->tok = tdrpStrDup(work_str);
  }

  return iret;

}

/**********************************************************
 * Warn if there are parameters in the param file which do
 * not belong there.
 */

#ifdef NOTNOW
static void warn_on_extra_params(token_handle_t *handle,
				 const char *param_file_path,
				 TDRPtable *table,
				 tdrpToken_t *tokens, int ntok)

{
  
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
#endif

/*********************************
 * structFieldToUser()
 *
 * Copy struct field to user space
 */

static void structFieldToUser(tdrpVal_t *val, void *target, int ptype)

{

  switch (ptype) {
      
  case BOOL_TYPE:
    *((int *) target) = val->b;
    break;
      
  case INT_TYPE:
    *((int *) target) = val->i;
    break;
    
  case LONG_TYPE:
    *((int *) target) = val->l;
    break;

  case FLOAT_TYPE:
    *((float *) target) = val->f;
    break;
      
  case DOUBLE_TYPE:
    *((double *) target) = val->d;
    break;
      
  case STRING_TYPE:
    {
      char **cptr = (char **) target;
      tdrpStrReplace(cptr, val->s);
    }
  break;
      
  case ENUM_TYPE:
    *((int *) target) = val->e;
    break;

  case STRUCT_TYPE:
    break;
    
  case COMMENT_TYPE:
    break;
      
  } /* switch */
    
}

/*********************************
 * structFieldFromUser()
 *
 * Copy struct field from user space
 */

static void structFieldFromUser(void *source, tdrpVal_t *val, int ptype)
     
{

  switch (ptype) {
      
  case BOOL_TYPE:
    val->b = *((tdrp_bool_t *) source);
    break;
      
  case INT_TYPE:
    val->i = *((int *) source);
    break;
    
  case LONG_TYPE:
    val->l = *((int *) source);
    break;

  case FLOAT_TYPE:
    val->f = *((float *) source);
    break;
      
  case DOUBLE_TYPE:
    val->d = *((double *) source);
    break;
      
  case STRING_TYPE:
    {
      char **cptr = (char **) source;
      tdrpStrReplace(&val->s, *cptr);
    }
  break;
      
  case ENUM_TYPE:
    val->e = *((int *) source);
    break;

  case STRUCT_TYPE:
    break;
    
  case COMMENT_TYPE:
    break;
      
  } /* switch */
    
}




