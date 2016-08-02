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

/***************************************************************
 * read_item.c
 *
 * Read routines for the various types of parameter.
 * Used to read both the defaults and the runtime files.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * April 1998
 *
 ****************************************************************/

#include <tdrp/tdrp.h>

#include <limits.h>
#include <string.h>
#include <float.h>

#define MIN_KEYWORD "MIN"
#define MAX_KEYWORD  "MAX"

/*
 * file scope prototypes
 */

static void free_struct_vals(TDRPtable *tt, tdrpVal_t *vals, int nvals);

static int load_single_val(token_handle_t *handle,
			   const char *param_name, const char *val_label,
			   const enum_def_t *enum_def,
			   const tdrpToken_t *token, int type, int expand_env,
			   tdrpVal_t *val);
     
static int read_single_val(token_handle_t *handle,
			   TDRPtable *tt, const char *val_label,
			   const tdrpToken_t *tokens, int ntok,
			   int type, int expand_env,
			   tdrpVal_t *val, int *val_tok_p);
     
static int read_array_vals(token_handle_t *handle,
			   TDRPtable *tt,
			   const tdrpToken_t *tokens, int ntok,
			   const char *val_label, int type, int expand_env,
			   tdrpVal_t **vals_p, int *label_tok_p,
			   int *nvals_p);

static int read_array2D_vals(token_handle_t *handle,
			     TDRPtable *tt,
			     const tdrpToken_t *tokens, int ntok,
			     const char *val_label, int type, int expand_env,
			     tdrpVal_t **vals_p, int *val_tok_p,
			     int *nvals1_p, int *nvals2_p);
     
static int read_val(token_handle_t *handle,
		    TDRPtable *tt, const char *label,
		    const tdrpToken_t *tokens, int ntok, int type, int expand_env);
     
static double val_as_double(tdrpVal_t *val, int type);

/****************
 * tdrpReadBool()
 *
 * Read boolean value and store in table entry.
 */

int tdrpReadBool(token_handle_t *handle,
		 TDRPtable *tt, const char *label,
		 tdrpToken_t *tokens, int ntok, int expand_env)

{
  if (read_val(handle, tt, label, tokens, ntok, BOOL_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}

/***************
 * tdrpReadInt()
 *
 * Read int value and store in table entry.
 */

int tdrpReadInt(token_handle_t *handle,
		TDRPtable *tt, const char *label,
		tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  if (read_val(handle, tt, label, tokens, ntok, INT_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}
     
/****************
 * tdrpReadLong()
 *
 * Read long value and store in table entry.
 */

int tdrpReadLong(token_handle_t *handle,
		 TDRPtable *tt, const char *label,
		 tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  if (read_val(handle, tt, label, tokens, ntok, LONG_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}
     
/*****************
 * tdrpReadFloat()
 *
 * Read float value and store in table entry.
 */

int tdrpReadFloat(token_handle_t *handle,
		  TDRPtable *tt, const char *label,
		  tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  if (read_val(handle, tt, label, tokens, ntok, FLOAT_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}
     
/******************
 * tdrpReadDouble()
 *
 * Read double value and store in table entry.
 */

int tdrpReadDouble(token_handle_t *handle,
		   TDRPtable *tt, const char *label,
		   tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  if (read_val(handle, tt, label, tokens, ntok, DOUBLE_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}
     
/*****************
 * tdrpReadString()
 *
 * Read string value and store in table entry.
 */

int tdrpReadString(token_handle_t *handle,
		   TDRPtable *tt, const char *label,
		   tdrpToken_t *tokens, int ntok, int expand_env)

{

  if (read_val(handle, tt, label, tokens, ntok, STRING_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}

/****************
 * tdrpReadEnum()
 *
 * Read enum value and store in table entry.
 */

int tdrpReadEnum(token_handle_t *handle,
		 TDRPtable *tt, const char *label,
		 tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  if (read_val(handle, tt, label, tokens, ntok, ENUM_TYPE, expand_env)) {
    return (-1);
  }
  tt->is_set = TRUE;
  return (0);
}
     
/******************
 * tdrpReadStruct()
 *
 * Read struct and store in table entry.
 */

int tdrpReadStruct(token_handle_t *handle,
		   TDRPtable *tt, const char *label,
		   tdrpToken_t *tokens, int ntok, int expand_env)
     
{
  char *following_token;

  int iret;
  int istruct, ifield, ival;
  int nstructs, nvals;
  int label_tok;
  int itok;
  int last_tok;
  int braces_start_tok, braces_end_tok;
  int n_gap, n_prs_enclosed;

  tdrpToken_t *token_list;
  tdrpVal_t *vals;

  /*
   * find label and following "="
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    label, &label_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
    fprintf(stderr, "  Cannot find '%s' for struct data\n", label);
    fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[tt->start_tok]));
    return (-1);
  }
  itok = label_tok;
  itok++;
  
  if (strcmp(tokens[itok].tok, "=")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
    fprintf(stderr, "  Cannot find '=' after label '%s'\n", label);
    fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[label_tok]));
    return (-1);
  }
  itok++;

  /*
   * reset the token list, ready for it to be loaded up
   */

  tdrpTokenListReset(handle);

  if (!tt->is_array) {

    nstructs = 1;

    if (tdrpLoadStruct(handle, tt,
		       itok, tt->end_tok, tokens, ntok, label,
		       ";", &last_tok)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
      fprintf(stderr, "  Cannot load struct data for '%s'\n", label);
      fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
      return (-1);
    }

  } else { /* if (!tt->is_array) */

    /*
     * array of structs
     */
    
    /*
     * find the number of enclosed braces in array
     */
    
    if (tdrpFindBracesPair(handle,
			   label, itok, tt->end_tok,
			   tokens, ntok,
			   &n_gap, &n_prs_enclosed,
			   &braces_start_tok, &braces_end_tok)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
      fprintf(stderr, "  Syntax error - braces.\n");
      fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
      return (-1);
    }

    if ((n_prs_enclosed == 0) &&
        (braces_end_tok != (braces_start_tok + 1))) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
      fprintf(stderr, "  Syntax error - braces.\n");
      fprintf(stderr, "  Need nested braces for struct arrays.\n");
      fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
      return (-1);
    }
    
    if (n_prs_enclosed == 0) {
      /* zero-length array */
      free_struct_vals(tt, tt->struct_vals, tt->n_struct_vals);
      tt->n_struct_vals = 0;
      tt->struct_vals = NULL;
      tt->array_n = 0;
      return (0);
    }

    nstructs = n_prs_enclosed;

    /*
     * check array size
     */
    
    if (tt->array_len_fixed && nstructs != tt->array_n) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
      fprintf(stderr, "  Fixed length struct array must have %d elements\n",
	      tt->array_n);
      fprintf(stderr, "  %d elems found, this is incorrent\n", nstructs);
      fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[label_tok]));
      return (-1);
    }

    itok = braces_start_tok + 1;
    for (istruct = 0; istruct < nstructs; istruct++) {

      if (istruct == nstructs - 1) {
	following_token = "}";
      } else {
	following_token = ",";
      }
      
      if (tdrpLoadStruct(handle, tt,
			 itok, tt->end_tok, tokens, ntok, label,
			 following_token, &last_tok)) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
	fprintf(stderr, "  Cannot load struct data for '%s'\n", label);
	fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }
      
      itok = last_tok + 1;
      
    } /* istruct */

  } /* if (!tt->is_array) */

  /*
   * allocate val array
   */
  
  nvals = tdrpNTokenList(handle);
  vals = (tdrpVal_t *) tdrpCalloc(nvals, sizeof(tdrpVal_t));

  /*
   * load up val array
   */
  
  token_list = tdrpTokenListPtr(handle);
  ival = 0;
  iret = 0;
  for (istruct = 0; istruct < nstructs; istruct++) {
    for (ifield = 0; ifield < tt->struct_def.nfields; ifield++, ival++) {
      if (load_single_val(handle, tt->param_name,
			  tt->struct_def.fields[ifield].ftype,
			  &tt->struct_def.fields[ifield].enum_def,
			  &token_list[ival],
			  tt->struct_def.fields[ifield].ptype,
			  expand_env, vals + ival)) {
	iret = -1;
      }
    } /* ifield */
  } /* istruct */

  if (iret < 0) {
    /* error - free up new val array */
    free_struct_vals(tt, vals, nvals);    
    tt->n_struct_vals = 0;
    tt->struct_vals = NULL;
    tt->array_n = 0;
    return (-1);
  } else {
    /* good - free up old array, move in new one */
    free_struct_vals(tt, tt->struct_vals, tt->n_struct_vals);
    tt->n_struct_vals = nvals;
    tt->struct_vals = vals;
    tt->array_n = nstructs;
  }
    
  tt->is_set = TRUE;

  return (0);

}
     
/***************
 * tdrpReadMin()
 *
 * Read min value and store in min_val.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpReadMin(token_handle_t *handle,
		TDRPtable *tt, tdrpToken_t *tokens,
		int ntok, int min_tok, int expand_env)
     
{
  int val_tok;
  if (read_single_val(handle, tt, "p_min", tokens, ntok, tt->ptype,
		      expand_env, &tt->min_val, &val_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
    fprintf(stderr, "Syntax error in 'p_min'.\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[min_tok]));
    return (-1);
  }
  return (0);
}

/***************
 * tdrpReadMax()
 *
 * Read max value and store in max_val.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpReadMax(token_handle_t *handle,
		TDRPtable *tt, tdrpToken_t *tokens,
		int ntok, int max_tok, int expand_env)
     
{
  int val_tok;
  if (read_single_val(handle, tt, "p_max", tokens, ntok, tt->ptype,
		      expand_env, &tt->max_val, &val_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
    fprintf(stderr, "Syntax error in 'p_max'.\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[max_tok]));
    return (-1);
  }
  return (0);
}

/*********************
 * tdrpCheckValRange()
 *
 * Check that the values lie within the min/max range as
 * applicable
 *
 * Return 0 on success, -1 on failure.
 */

int tdrpCheckValRange(TDRPtable *tt)

{

  int i;
  int iret = 0;
  int type = tt->ptype;
  double dval, dmax, dmin;

  if (type != INT_TYPE && type != LONG_TYPE &&
      type != FLOAT_TYPE && type == DOUBLE_TYPE) {
    return (0);
  }

  if (!tt->has_min && !tt->has_max) {
    return (0);
  }
  
  /*
   * check default vals are within min and max limits
   */
  
  if (tt->is_array) {
    
    for (i = 0; i < tt->array_n; i++) {
      
      dval = val_as_double(&tt->array_vals[i], type);
      dmax = val_as_double(&tt->max_val, type);
      dmin = val_as_double(&tt->min_val, type);
      
      if (tt->has_max && dval > dmax) {
	fprintf(stderr,	"\n>>> TDRP_ERROR <<< param '%s'\n",
		tt->param_name);
	fprintf(stderr,
		"Value %g exceeds max %g.\n", dval, dmax);
	iret = -1;
      }
      if (tt->has_min && dval < dmin) {
	fprintf(stderr,	"\n>>> TDRP_ERROR <<< param '%s'\n",
		tt->param_name);
	fprintf(stderr,
		"Value %g below min %g.\n", dval, dmin);
	iret = -1;
      }
    } /* i */
    
  } else { /* if(tt->is_array) */
    
    dval = val_as_double(&tt->single_val, type);
    dmax = val_as_double(&tt->max_val, type);
    dmin = val_as_double(&tt->min_val, type);
    
    if (tt->has_max && dval > dmax) {
      fprintf(stderr,	"\n>>> TDRP_ERROR <<< param '%s'\n",
	      tt->param_name);
      fprintf(stderr,
	      "Value %g exceeds max %g.\n", dval, dmax);
      iret = -1;
    }
    if (tt->has_min && dval < dmin) {
      fprintf(stderr,	"\n>>> TDRP_ERROR <<< param '%s'\n",
	      tt->param_name);
      fprintf(stderr,
	      "Value %g below min %g.\n", dval, dmin);
      iret = -1;
    }
    
  } /* if(tt->is_array) */
    
  return (iret);

}

/*********************
 * file scope routines
 */
     
/************
 * read_val()
 *
 * Reads val for param, single or array, and stores in table entry.
 *
 * Valid for bool, enum, int, long, float, double, string
 */

static int read_val(token_handle_t *handle,
		    TDRPtable *tt, const char *label,
		    const tdrpToken_t *tokens, int ntok, int type, int expand_env)
     
{
  
  tdrpVal_t val, *vals;
  int nvals, nvals1, nvals2;
  int itok = 0;
  int label_tok;
  int iret = 0;

  /*
   * check for label in paramdef file
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    label, &label_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
    fprintf(stderr, "Cannot find '%s'\n", label);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[tt->start_tok]));
    return (-1);
  }
  
  if (tt->is_array2D) {
    
    /*
     * load up array vals
     */
    
    if (read_array2D_vals(handle, tt, tokens, ntok,
			  label, type, expand_env, 
			  &vals, &itok, &nvals1, &nvals2) == 0) {
      
      if (tt->array_len_fixed &&
	  (nvals1 != tt->array_n1 || nvals2 != tt->array_n2)) {
	
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
	fprintf(stderr, "2D fixed len array must be %d x %d\n",
		tt->array_n1, tt->array_n2);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	if (type == STRING_TYPE) {
	  int i;
	  for (i = 0; i < nvals1 * nvals2; i++) {
	    tdrpFree(tt->array_vals[i].s);
	  }
	}
	tdrpFree(vals);
	iret = -1;

      } else {

	nvals = nvals1 * nvals2;

	/*
	 * free up old strings
	 */
	
	if (type == STRING_TYPE) {
	  int i;
	  for (i = 0; i < tt->array_n; i++) {
	    tdrpFree(tt->array_vals[i].s);
	  }
	}

	/*
	 * free up vals array
	 */

	tdrpFree(tt->array_vals);

	/*
	 * replace old vals with new ones
	 */

	tt->array_vals = vals;
	tt->array_n = nvals;
	tt->array_n1 = nvals1;
	tt->array_n2 = nvals2;

      } /* if (tt->array_len_fixed ... */

    } else { /* if (read_array2D_vals ... */

      iret = -1;

    }
    
  } else if (tt->is_array) {
    
    /*
     * load up array vals
     */
    
    if (read_array_vals(handle, tt, tokens, ntok,
			label, type, expand_env,
			&vals, &itok, &nvals) == 0) {
      
      if (tt->array_len_fixed && nvals != tt->array_n) {
	
	/*
	 * error - incorrect number of items
	 */
	
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr, "Fixed len array must have %d elems\n",
		tt->array_n);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	if (type == STRING_TYPE) {
	  int i;
	  for (i = 0; i < nvals; i++) {
	    tdrpFree(tt->array_vals[i].s);
	  }
	}
	tdrpFree(vals);
	iret = -1;
	
      } /* if (tt->array_len_fixed ... */

      /*
       * free up old memory
       */
      
      if (type == STRING_TYPE) {
	int i;
	for (i = 0; i < tt->array_n; i++) {
	  tdrpFree(tt->array_vals[i].s);
	}
      }
      tdrpFree(tt->array_vals);
      
      /*
       * replace old vals with new ones
       */
      
      tt->array_vals = vals;
      tt->array_n = nvals;
      
    } else { /* if (read_array_vals ... */

      iret = -1;

    }
    
  } else {

    /*
     * load up single val
     */
    
    if (read_single_val(handle, tt, label, tokens, ntok,
			type, expand_env, &val, &itok) == 0) {

      /*
       * replace old val with new one
       */
      
      if (type == STRING_TYPE) {
	tdrpFree(tt->single_val.s);
      }
      tt->single_val = val;

    } else {
      iret = -1;
    }

  } /* if (tt->is_array) */

  return (iret);

}

/********************
 * read_single_val()
 *
 * Reads in given simple value, checking the syntax.
 *
 * Valid types are boolean, enum, string, int, long, float and double.
 *
 * Legal syntax is:
 *   item_label = val;
 * or
 *   item_label = {val};
 *
 * Sets val.
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_single_val(token_handle_t *handle,
			   TDRPtable *tt, const char *val_label,
			   const tdrpToken_t *tokens, int ntok,
			   int type, int expand_env,
			   tdrpVal_t *val, int *val_tok_p)
     
{

  int val_tok;
  const tdrpToken_t *token;
  
  if (tdrpFindSingleItem(handle,
			 tt->start_tok, tt->end_tok, tokens, ntok,
			 val_label, &val_tok)) {
    return (-1);
  }

  token = tokens + val_tok;
  *val_tok_p = val_tok;

  if (load_single_val(handle, tt->param_name, val_label, &tt->enum_def,
		      token, type, expand_env, val)) {
    return (-1);
  }
  return (0);

}

/******************
 * read_array_vals()
 *
 * Allocates given array and reads values, checking the syntax.
 *
 * Valid types are boolean, enum, string, int, long, float and double.
 *
 * Legal syntax is:
 *   item_label = val, val, val, ...;
 * or
 *   item_label = {val, val, val, ...};
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_array_vals(token_handle_t *handle,
			   TDRPtable *tt,
			   const tdrpToken_t *tokens, int ntok,
			   const char *val_label, int type, int expand_env,
			   tdrpVal_t **vals_p, int *label_tok_p,
			   int *nvals_p)
     
{

  int i;
  int nvals;
  int label_tok;
  int last_tok;
  tdrpToken_t *token_list;
  tdrpVal_t *vals, *val;

  *vals_p = NULL;
  *nvals_p = 0;

  /*
   * reset the token list, ready for it to be loaded up
   */

  tdrpTokenListReset(handle);

  /*
   * find the multiple item, loading up the token list
   */
  
  if (tdrpFindMultipleItem(handle,
			   tt->start_tok, tt->end_tok, tokens, ntok,
			   val_label, &label_tok, &last_tok)) {
    return (-1);
  }

  /*
   * allocate val array
   */
  
  nvals = tdrpNTokenList(handle);
  vals = (tdrpVal_t *) tdrpCalloc(nvals, sizeof(tdrpVal_t));
  *vals_p = vals;
  *nvals_p = nvals;
  *label_tok_p = label_tok;

  /*
   * load up val array
   */
  
  token_list = tdrpTokenListPtr(handle);
  val = vals;
  for (i = 0; i < nvals; i++, val++) {
    if (load_single_val(handle, tt->param_name, val_label, &tt->enum_def,
			&token_list[i], type, expand_env, val)) {
      tdrpFree(vals);
      return (-1);
    }
  }
  
  return (0);

}

/********************
 * read_array2D_vals()
 *
 * Allocates given array and reads values, checking the syntax.
 *
 * Valid types are boolean, enum, string, int, long, float and double.
 *
 * Legal syntax is:
 *   item_label = {{val, val, val, ...},
 *                 {val, val, val, ...},
 *                 {val, val, val, ...},
 *                 {val, val, val, ...}, ...}
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_array2D_vals(token_handle_t *handle,
			     TDRPtable *tt,
			     const tdrpToken_t *tokens, int ntok,
			     const char *val_label, int type, int expand_env,
			     tdrpVal_t **vals_p, int *val_tok_p,
			     int *nvals1_p, int *nvals2_p)
     
{

  char *following_token;

  int i;
  int nvals, nvals1, nvals2 = 0;
  /* int array_start_tok; */
  int val_tok;
  int itok;
  int braces_start_tok, braces_end_tok;
  int n_gap, n_prs_enclosed;
  int nitems;
  int last_tok;

  tdrpToken_t *token_list;
  tdrpVal_t *vals, *val;

  *vals_p = NULL;
  *nvals1_p = 0;
  *nvals2_p = 0;

  /*
   * find label and following "="
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    val_label, &val_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
    fprintf(stderr, "  Cannot find '%s' 2D array\n", val_label);
    fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[tt->start_tok]));
    return (-1);
  }
  *val_tok_p = val_tok;
  itok = val_tok;
  itok++;
  
  if (strcmp(tokens[itok].tok, "=")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
    fprintf(stderr, "  Cannot find '='\n");
    fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[val_tok]));
    return (-1);
  }
  itok++;

  /*
   * find the number of enclosed braces in array
   */

  if (tdrpFindBracesPair(handle,
			 val_label, itok, tt->end_tok,
			 tokens, ntok,
			 &n_gap, &n_prs_enclosed,
			 &braces_start_tok, &braces_end_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
    fprintf(stderr, "  Syntax error - braces\n");
    fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }

  if (n_prs_enclosed == 0) {
    /* zero length array */
    return (0);
  }

  itok++;
  /* array_start_tok = itok; */

  /*
   * the major dimension is obtained from the number of enclosed
   * brace pairs
   */

  nvals1 = n_prs_enclosed;

  /*
   * reset the token list, ready for it to be loaded up
   */

  tdrpTokenListReset(handle);

  /*
   * loop through the major dimension, finding the 
   * minor dimensions, loading up the token list
   */

  for (i = 0; i < nvals1; i++) {

    /*
     * check that we have a valid multiple item
     */
    
    if (i == nvals1 - 1) {
      following_token = "}";
    } else {
      following_token = ",";
    }
    
    /*
     * load up multiple item into token list
     */
    
    if (tdrpLoadMultipleItem(handle,
			     itok, tt->end_tok, tokens, ntok,
			     val_label, following_token, &last_tok)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
      fprintf(stderr, "  Syntax error - array entry\n");
      fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
      return (-1);
    }

    if (i == 0) {
      nvals2 = tdrpNTokenList(handle);
    } else {
      nitems = tdrpNTokenList(handle) / (i+1);
      if (nitems != nvals2) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", tt->param_name);
	fprintf(stderr, "  2D arrays must have constant 2nd dimension\n");
	fprintf(stderr, "  Conflicting dimensions %d and %d\n",
		nvals2, nitems);
	fprintf(stderr, "  %s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }
    }

    itok = last_tok + 1;

  } /* i */

  /*
   * allocate val array
   */
  
  nvals = tdrpNTokenList(handle);
  vals = (tdrpVal_t *) tdrpCalloc(nvals, sizeof(tdrpVal_t));

  /*
   * load up val array
   */
  
  token_list = tdrpTokenListPtr(handle);
  val = vals;
  for (i = 0; i < nvals; i++, val++) {
    if (load_single_val(handle, tt->param_name, val_label, &tt->enum_def,
			&token_list[i], type, expand_env, val)) {
      tdrpFree(vals);
      return (-1);
    }
  }
  
  *vals_p = vals;
  *nvals1_p = nvals1;
  *nvals2_p = nvals2;

  return (0);

}

/******************
 * load_single_val()
 *
 * Load given simple value.
 *
 * Valid types are enum, string, int, long, float and double.
 *
 * Returns 0 on success, -1 on failure.
 */

static int load_single_val(token_handle_t *handle,
			   const char *param_name, const char *val_label,
			   const enum_def_t *enum_def,
			   const tdrpToken_t *token, int type,
			   int expand_env, tdrpVal_t *val)
     
{

  int i, ii;
  int field_found;
  long ll;
  int bool_true;
  double dd;
  char keyword[1024];
  
  /*
   * set print tok to NULL. If it is not null, it will be used for
   * printing instead of the numeric value
   */
  
  memset(val, 0, sizeof(tdrpVal_t));
  
  switch (type) {

  case BOOL_TYPE:
    if (tdrpBoolStrTrue(token->tok, &bool_true)) {
      if (!expand_env && strstr(token->tok, "$(")) {
	val->print.tok = tdrpStrDup(token->tok);
	return 0;
      } else {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - param '%s'\n", param_name);
	fprintf(stderr, "Cannot decode bool for %s\n", val_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	return -1;
      }
    }
    if (bool_true) {
      val->b = pTRUE;
    } else {
      val->b = pFALSE;
    }
    break;

  case ENUM_TYPE:
    field_found = FALSE;
    for (i = 0; i < enum_def->nfields; i++) {
      char *tmpstr = tdrpStrDup(token->tok);
      int same = strcmp(tmpstr, enum_def->fields[i].name);
      tdrpFree(tmpstr);
      if (!same) {
	val->e = enum_def->fields[i].val;
	field_found = TRUE;
	break;
      }
    }
    if (!field_found) {
      if (!expand_env && strstr(token->tok, "$(")) {
	val->print.tok = tdrpStrDup(token->tok);
	return 0;
      } else {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr, "Invalid option '%s' for enum for %s\n",
		token->tok, val_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	return -1;
      }
    }
    break;

  case STRING_TYPE:
    val->s = tdrpStrDup(token->tok);
    break;

  case INT_TYPE:
    if (sscanf(token->tok, "%d", &ii) != 1) {
      /*
       * check for the special keywords MIN/MAX
       */
      if (sscanf(token->tok, "%s", keyword) == 1) {
        if ( !strcmp( keyword, MIN_KEYWORD )) {
          ii = INT_MIN;
        } else if ( !strcmp( keyword, MAX_KEYWORD )) {
          ii = INT_MAX;
	} else {
	  if (!expand_env && strstr(token->tok, "$(")) {
	    val->print.tok = tdrpStrDup(token->tok);
	    return 0;
	  } else {
	    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	    fprintf(stderr, "Cannot decode int for %s\n", val_label);
	    fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	    return -1;
	  }
	} /*  if ( !strcmp( keyword, MIN_KEYWORD )) */
      }
    }
    val->i = ii;
    break;

  case LONG_TYPE:
    if (sscanf(token->tok, "%ld", &ll) != 1) {
      /*
       * check for the special keywords MIN/MAX
       */
      if (sscanf(token->tok, "%s", keyword) == 1) {
        if ( !strcmp( keyword, MIN_KEYWORD )) {
          ll = LONG_MIN;
        } else if ( !strcmp( keyword, MAX_KEYWORD )) {
          ll = LONG_MAX;
        } else {
	  if (!expand_env && strstr(token->tok, "$(")) {
	    val->print.tok = tdrpStrDup(token->tok);
	    return 0;
	  } else {
	    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	    fprintf(stderr, "Cannot decode long for %s\n", val_label);
	    fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	    return -1;
	  }
        }
      }
    }
    val->l = ll;
    break;

  case FLOAT_TYPE:
    if (sscanf(token->tok, "%lg", &dd) != 1) {
      /*
       * check for the special keywords MIN/MAX
       */
      if (sscanf(token->tok, "%s", keyword) == 1) {
        if ( !strcmp( keyword, MIN_KEYWORD )) {
          dd = FLT_MIN;
        } else if ( !strcmp( keyword, MAX_KEYWORD )) {
          dd = FLT_MAX;
        } else {
	  if (!expand_env && strstr(token->tok, "$(")) {
	    val->print.tok = tdrpStrDup(token->tok);
	    return 0;
	  } else {
	    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	    fprintf(stderr, "Cannot decode float for %s\n", val_label);
	    fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	    return -1;
	  }
        }
      }
    }
    val->f = (float) dd;
    break;

  case DOUBLE_TYPE:
    if (sscanf(token->tok, "%lg", &dd) != 1) {
      /*
       * check for the special keywords MIN/MAX
       */
      if (sscanf(token->tok, "%s", keyword) == 1) {
        if ( !strcmp( keyword, MIN_KEYWORD )) {
          dd = DBL_MIN;
        } else if ( !strcmp( keyword, MAX_KEYWORD )) {
          dd = DBL_MAX;
        } else {
	  if (!expand_env && strstr(token->tok, "$(")) {
	    val->print.tok = tdrpStrDup(token->tok);
	    return 0;
	  } else {
	    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	    fprintf(stderr, "Cannot decode double for %s\n", val_label);
	    fprintf(stderr, "%s\n", tdrpLineInfo(handle, token));
	    return -1;
	  }
        }
      }
    }
    val->d = dd;
    break;

  default:
    break;

  } /* switch */
  
  return (0);

}

/*****************
 * val_as_double()
 *
 * Returns int, long, float or double as double.
 */

static double val_as_double(tdrpVal_t *val, int type)

{

  switch (type) {

  case INT_TYPE:
    return ((double) val->i);
    break;

  case LONG_TYPE:
    return ((double) val->l);
    break;

  case FLOAT_TYPE:
    return ((double) val->f);
    break;

  case DOUBLE_TYPE:
    return (val->d);
    break;

  }

  return (0.0);

}

/*********************
 * free_struct_vals()
 *
 * Free up vals array and strings associated with the array
 */

static void free_struct_vals(TDRPtable *tt, tdrpVal_t *vals, int nvals)

{
  int i;
  int ifield;
  for (i = 0; i < nvals; i++) {
    ifield = i % tt->struct_def.nfields;
    if (tt->struct_def.fields[ifield].ptype == STRING_TYPE) {
      tdrpFree(vals[i].s);
    }
  }
  tdrpFree(vals);
}
