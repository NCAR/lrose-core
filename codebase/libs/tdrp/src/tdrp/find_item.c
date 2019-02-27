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
 * find_item.c
 *
 * Find items in a token list
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Jan 1998
 *
 ****************************************************************/

#include <tdrp/tdrp.h>
#include <string.h>

/*****************
 * tdrpFindLabel()
 *
 * Finds given label.
 *
 * Sets label_tok.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpFindLabel(int start_tok, int end_tok,
		  const tdrpToken_t *tokens, int ntok,
		  const char *item_label, int *label_tok_p)

{

  int i;
  int item_found = FALSE;
  const tdrpToken_t *token = tokens + start_tok;

  /*
   * search for item definition
   */
  
  for (i = start_tok; i <= end_tok; i++, token++) {
    if (!strcmp(token->tok, item_label)) {
      item_found = TRUE;
      break;
    }
  } /* i */
  
  if (item_found) {
    *label_tok_p = i;
    return (0);
  } else {
    return (-1);
  }

  return (0);

}

/*********************
 * tdrpFindParamLast()
 *
 * Finds last instance of given param in file.
 * Checks for trailing '='.
 *
 * Sets param_tok.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpFindParamLast(int start_tok, int end_tok,
		      const tdrpToken_t *tokens, int ntok,
		      const char *param_label, int *param_tok_p)

{

  int i;
  int param_found = FALSE;
  int param_tok = 0;
  int n_unpaired = 0;

  /*
   * search for item definition - it must lie outside any
   * braces
   */
  
  for (i = start_tok; i <= end_tok; i++) {
    if (!strcmp(tokens[i].tok, ";")) {
      n_unpaired = 0;
    } else if (!strcmp(tokens[i].tok, "{")) {
      n_unpaired++;
    } else if (!strcmp(tokens[i].tok, "}")) {
      n_unpaired--;
    }
    if (n_unpaired == 0 && !strcmp(tokens[i].tok, param_label)) {
      if (!strcmp(tokens[i+1].tok, "=")) {
	param_found = TRUE;
	param_tok = i;
      }
    }
  } /* i */
  
  if (param_found) {
    *param_tok_p = param_tok;
    return (0);
  } else {
    return (-1);
  }

}

/**********************
 * tdrpFindBracesPair()
 *
 * Finds the next pair of braces.
 *
 * Sets:
 *  *n_gap_p: number of tokens before braces start
 *  *n_prs_enclosed_p: number of enclosed braces, excluding the outside pair.
 *                     These may be nested or sequential.
 *  *braces_start_tok_p: token for start of braces.
 *  *braces_end_tok_p: token for end of braces.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpFindBracesPair(token_handle_t *handle,
		       const char *label,
		       int start_tok, int end_tok,
		       const tdrpToken_t *tokens, int ntok,
		       int *n_gap_p, int *n_prs_enclosed_p,
		       int *braces_start_tok_p,
		       int *braces_end_tok_p)
     
{

  int itok, jtok;
  int n_unpaired;
  int n_prs_enclosed;

  /*
   * find for starting '{'
   */

  for (itok = start_tok; itok <= end_tok; itok++) {
    if (!strcmp(tokens[itok].tok, "{")) {
      break;
    }
  }
  
  if (itok > end_tok) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< '%s'\n", label);
    fprintf(stderr, "  Cannot match braces\n");
    fprintf(stderr, "  Start: %s\n", tdrpLineInfo(handle, &tokens[start_tok]));
    fprintf(stderr, "  End: %s\n", tdrpLineInfo(handle, &tokens[end_tok]));
    return (-1);
  }

  /*
   * set start token to opening brace
   */

  *n_gap_p = itok - start_tok;
  *braces_start_tok_p = itok;
  *braces_end_tok_p = itok;

  /*
   * find end '}'
   */
  
  n_unpaired = 1;
  n_prs_enclosed = 0;
  for (jtok = itok + 1; jtok <= end_tok; jtok++) {
    if (!strcmp(tokens[jtok].tok, "{")) {
      n_unpaired++;
      n_prs_enclosed++;
    } else if (!strcmp(tokens[jtok].tok, "}")) {
      n_unpaired--;
      if (n_unpaired == 0) {
	break;
      }
    }
  } /* jtok */
  
  if (n_unpaired != 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< '%s'\n", label);
    fprintf(stderr, "  Cannot match braces\n");
    fprintf(stderr, "  Start: %s\n", tdrpLineInfo(handle, &tokens[start_tok]));
    fprintf(stderr, "  End: %s\n", tdrpLineInfo(handle, &tokens[end_tok]));
    return (-1);
  }

  *n_prs_enclosed_p = n_prs_enclosed;
  *braces_end_tok_p = jtok;

  return (0);

}

/*************************
 * tdrpFindArrayBrackets()
 *
 * Find brackets for array, setting attributes as they are found.
 *
 * Expects bracket to start immediately.
 *
 * Returns 0 on success, -1 on failure
 */

int tdrpFindArrayBrackets(token_handle_t *handle,
			  int start_tok,
			  const tdrpToken_t *tokens, int ntok,
			  int *last_tok_used_p,
			  int *is_fixed_p,
			  int *fixed_len_p)

{

  int itok = start_tok;
  if (itok > ntok - 1) {
    return (-1);
  }
  
  /*
   * look for left bracket
   */

  if (strcmp(tokens[itok].tok, "[")) {
    return (-1);
  }
  itok++;
  if (itok > ntok - 1) {
    return (-1);
  }

  /*
   * look for right bracket - if found here this is a variable length array
   */
  
  if (!strcmp(tokens[itok].tok, "]")) {
    *is_fixed_p = FALSE;
    *last_tok_used_p = itok;
    return (0);
  }

  /*
   * probably variable length array - find length
   */
  
  if (sscanf(tokens[itok].tok, "%d", fixed_len_p) != 1) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Expecting array count, found '%s'\n",
	    tokens[itok].tok);
    fprintf(stderr, "%s", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }
  *is_fixed_p = TRUE;
  itok++;
  if (itok > ntok - 1) {
    return (-1);
  }

  /*
   * look for right bracket
   */
  
  if (strcmp(tokens[itok].tok, "]")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Expecting closing ']', found '%s'\n",
	    tokens[itok].tok);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }

  /*
   * success
   */

  *last_tok_used_p = itok;
  return (0);

}

/**********************
 * tdrpFindSingleItem()
 *
 * Finds given simple item, checking the syntax.
 *
 * Legal syntax is:
 *   item_label = val;
 * or
 *   item_label = {val};
 *
 * Sets item_tok.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpFindSingleItem(token_handle_t *handle,
		       int start_tok, int end_tok,
		       const tdrpToken_t *tokens, int ntok,
		       const char *item_label, int *item_tok_p)

{

  int colon_tok;
  int item_tok;
  int label_tok;
  const tdrpToken_t *token;

  /*
   * search for item definition
   */
  
  if (tdrpFindLabel(start_tok, end_tok, tokens, ntok,
		    item_label, &label_tok)) {
    return (-1);
  }
  
  /*
   * check syntax
   */
  
  token = tokens + label_tok;
  if (strcmp(token[1].tok, "=")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No '=' found to set item %s\n", item_label);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &token[1]));
    return (-1);
  }

  if (!strcmp(token[2].tok, "{") && !strcmp(token[4].tok, "}")) {
    /* braces included */
    item_tok = 3;
    colon_tok = 5;
  } else {
    /* no braces */
    item_tok = 2;
    colon_tok = 3;
  }


  if (strcmp(token[colon_tok].tok, ";")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No ';' found to end item %s\n", item_label);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &token[colon_tok]));
    return (-1);
  }

  /*
   * OK - set return val
   */

  *item_tok_p = label_tok + item_tok;

  return (0);

}

/************************
 * tdrpFindMultipleItem()
 *
 * Finds given multiple item, checking the syntax.
 *
 * Legal syntax is:
 *   item_label = val,val,val...;
 * or
 *   item_label = {val,val,val...};
 *
 * Loads up the token list with the tokens found. See
 * token_list.c. The token list is then used by
 * the calling function to gain access to the tokens.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpFindMultipleItem(token_handle_t *handle,
			 int start_tok, int end_tok,
			 const tdrpToken_t *tokens, int ntok,
			 const char *item_label,
			 int *label_tok_p,
			 int *last_tok_p)

{

  int itok;
  int label_tok;

  /*
   * search for item definition
   */

  if (tdrpFindLabel(start_tok, end_tok, tokens, ntok,
		    item_label, &label_tok)) {
    return (-1);
  }
  *label_tok_p = label_tok;
  itok = label_tok + 1;
  
  /*
   * check for "="
   */
  
  if (strcmp(tokens[itok].tok, "=")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No '=' found to set item %s\n", item_label);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }
  itok++;

  /*
   * loading up the mult item into token list
   */
  
  if (tdrpLoadMultipleItem(handle,
			   itok, end_tok, tokens, ntok,
			   item_label, ";", last_tok_p)) {
    return (-1);
  }
  
  return (0);

}

/************************
 * tdrpLoadMultipleItem()
 *
 * Checks syntax on multiple item, load up into token list.
 *
 * Legal syntax is:
 *   item_label = val,val,val... [following_token]
 * or
 *   item_label = {val,val,val...} [following_token]
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpLoadMultipleItem(token_handle_t *handle,
			 int start_tok, int end_tok,
			 const tdrpToken_t *tokens, int ntok,
			 const char *item_label,
			 char *following_token,
			 int *last_tok_p)

{

  int braces_included;
  int itok = start_tok;
  int done;

  if (following_token == NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No following token for multiple item data\n");
    return (-1);
  }

  if (!strcmp(tokens[itok].tok, "{")) {
    /* braces included */
    braces_included = TRUE;
    itok++;
    if (!strcmp(tokens[itok].tok, "}")) {
      /* empty - no tokens within braces */
      itok++;
      if (strcmp(tokens[itok].tok, following_token)) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr, "No '%s' found to end item %s\n",
		following_token, item_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }
      *last_tok_p = itok;
      return (0);
    }
  } else {
    /* no braces */
    braces_included = FALSE;
  }

  /*
   * check through the list, finding how many elements there are
   * and checking the syntax
   */
  
  done = FALSE;

  while (!done) {
    
    /*
     * look for "val ," pair
     */
    
    if (!strcmp(tokens[itok+1].tok, ",")) {
      
      /* 
       * check for invalid entries
       */

      if (!tokens[itok].is_string &&
	  tdrpReservedStr(tokens[itok].tok)) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr, "Syntax error in list for item %s\n", item_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }

      /*
       * valid value - add to token list
       */
    
      tdrpTokenListAdd(handle, tokens + itok);
      itok += 2;

    } else { /* if (!strcmp(tokens[itok+1].tok, ",")) */

      /*
       * entries must end now
       */

      if (strcmp(tokens[itok].tok, following_token) &&
	  strcmp(tokens[itok].tok, "}")) {
	
	/*
	 * item is not "}" or following_token so this is the last entry
	 */
	
	tdrpTokenListAdd(handle, tokens + itok);

	if (!strcmp(tokens[itok].tok, ",")) {
	  fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	  fprintf(stderr, "Incorrect val in list for item %s\n", item_label);
	  fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	  return (-1);
	}

	itok++;

      }

      /*
       * check for closing braces and following token as applicable
       */
      
      if (braces_included) {

	if (strcmp(tokens[itok].tok, "}")) {
	  fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	  fprintf(stderr, "No '}' found to end item %s\n", item_label);
	  fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	  return (-1);
	}
	itok++;
      
      } /* if (braces_included) */

      if (following_token != NULL) {

	if (strcmp(tokens[itok].tok, following_token)) {
	  fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	  fprintf(stderr, "No '%s' found to end item %s\n",
		  following_token, item_label);
	  fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	  return (-1);
	}
	itok++;
	
      } /* if (following_token != NULL) */

      done = TRUE;
      *last_tok_p = itok - 1;

    } /* if (!strcmp(tokens[itok+1].tok, ",")) */
    
    if (itok - 1 > end_tok) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "No completion to item %s\n", item_label);
      fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[end_tok]));
      return (-1);
    }

  } /* while (!done) */

  return (0);

}

/******************
 * tdrpLoadStruct()
 *
 * Checks syntax on struct, load up into token list.
 *
 * Legal syntax is:
 *   item_label = {val,val,val...} [following_token]
 * or
 *   item_label = {val,label=val,val...} [following_token]
 * where following_token is either ',' or '}'.
 *
 * Returns 0 on success, -1 on failure.
 */

int tdrpLoadStruct(token_handle_t *handle,
		   TDRPtable *tt, 
		   int start_tok, int end_tok,
		   const tdrpToken_t *tokens, int ntok,
		   const char *item_label,
		   const char *following_token,
		   int *last_tok_p)
     
{
  
  char *expected_tok = NULL;
  int itok = start_tok;
  int ifield;
  struct_def_t *def = &tt->struct_def;

  if (following_token == NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No following token for struct data\n");
    return (-1);
  }

  if (strcmp(tokens[itok].tok, "{")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr,
	    "No leading brace for struct data, item %s\n", item_label);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }
  itok++;

  /*
   * read through the tokens, loading up the struct fields
   */

  for (ifield = 0; ifield < def->nfields; ifield++) {
    
    if (!strcmp(tokens[itok+1].tok, "=")) {
      
      /*
       * found a "label = val" syntax
       */
      
      if (tdrpReservedStr(tokens[itok].tok) ||
	  (!tokens[itok+2].is_string &&
	   tdrpReservedStr(tokens[itok+2].tok))) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr,
		"Syntax error in struct data, item %s\n", item_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }

      /*
       * check label is correct
       */
      
      if (strcmp(tokens[itok].tok, def->fields[ifield].fname)) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr,
		"Invalid field name '%s' in struct data, expecting '%s'\n",
		tokens[itok].tok, def->fields[ifield].fname);
	fprintf(stderr, "Item %s\n", item_label);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }
      
      /*
       * valid value - add to token list
       */
      
      itok += 2;
      tdrpTokenListAdd(handle, tokens + itok);
      itok++;

    } else {

      /*
       * assume not "label = val" syntax - just plain field val
       */

      if (!tokens[itok].is_string &&
          tdrpReservedStr(tokens[itok].tok)) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
	fprintf(stderr, "Syntax error in struct data, item %s\n", item_label);
	fprintf(stderr, "'%s' out of place\n", tokens[itok].tok);
	fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
	return (-1);
      }
      
      /*
       * add value to token list
       */
      
      tdrpTokenListAdd(handle, tokens + itok);
      itok++;

    }
    
    /*
     * check for trailing "}"
     */

    if (!strcmp(tokens[itok].tok, "}")) {

      int ii;

      if (ifield == def->nfields - 1) {
	/* normal ending */
	itok++;
	break;
      }

      /*
       * found "}" early, so insert defaults for remaining
       * fields in struct
       */

      for (ii = ifield + 1; ii < def->nfields; ii++) {
	
	// Make sure that there are defaults specified for the field
	// values.  If there are no defaults specified, print out an
	// error message

	if (tt->struct_vals == 0) {
	  fprintf(stderr, "\n");
	  fprintf(stderr, ">>> TDRP_ERROR <<< - struct parameter '%s'\n",
		  tt->param_name);
	  fprintf(stderr,
		  "    Missing field '%s'\n", def->fields[ii].fname);
	  fprintf(stderr, "    The application hasn't specified default values for this field\n");
	  fprintf(stderr, "    Update your parameter file and try again\n");
	  return (-1);
	}
	
	tdrpToken_t tmp_tok;
	tmp_tok.tok = sprintf_val(tt->struct_def.fields[ii].ptype,
				  &tt->struct_def.fields[ii].enum_def,
				  tt->struct_vals + ii);
	tmp_tok.used = 1;
	tmp_tok.line_num = tokens[itok].line_num;
	if (tt->struct_def.fields[ii].ptype == STRING_TYPE) {
	  /* remove the quotes */
	  int len = strlen(tmp_tok.tok);
	  memmove(tmp_tok.tok, tmp_tok.tok + 1, len - 2);
	  tmp_tok.tok[len - 2] = '\0';
	  tmp_tok.is_string = 1;
	} else {
	  tmp_tok.is_string = 0;
	}

	tdrpTokenListAdd(handle, &tmp_tok);

	fprintf(stderr, "\n");
	fprintf(stderr, ">>> TDRP_WARNING <<< - struct parameter '%s'\n",
		tt->param_name);
	fprintf(stderr,
		"    Missing field '%s': setting to default value: %s\n",
		def->fields[ii].fname, tmp_tok.tok);
	fprintf(stderr, "    %s\n", tdrpLineInfo(handle, &tokens[itok]));
	fprintf(stderr, "    Update your file using -print_params\n");

	tdrpFree(tmp_tok.tok);

      } /* ii */

      itok++;
      break;
      
    } /* looking for } */

    /*
     * else check for trailing ","
     */

    if (strcmp(tokens[itok].tok, ",")) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "Syntax error in struct data - expecting '%s'\n",
	      expected_tok);
      fprintf(stderr, "Item %s\n", item_label);
      fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
      return (-1);
    }

    itok++;

  } /* ifield */

  /*
   * check for the expected following_token
   */
  if (strcmp(tokens[itok].tok, following_token)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Syntax error in struct data\n");
    fprintf(stderr, "Found '%s', expecting '%s'\n",
	    tokens[itok].tok, following_token);
    fprintf(stderr, "%s\n", tdrpLineInfo(handle, &tokens[itok]));
    return (-1);
  }

  *last_tok_p = itok;
  return (0);

}

