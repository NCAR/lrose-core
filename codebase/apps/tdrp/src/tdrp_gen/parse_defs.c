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
 * parse_paramdefs.c
 *
 * Parses the paramdefs.
 *
 * Returns 0 on success, -1 on failure
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Jan 1998
 *
 ****************************************************************/

#include "tdrp_gen.h"

static int Debug;

typedef struct {
  int start_tok;
  int end_tok;
  int type;
  char *name;
} typedef_handle_t;

/***********************
 * file scope prototypes
 */

static int
define_enum_from_paramdef(token_handle_t *tok_handle,
			  char *enum_name,
			  int start_tok, int end_tok,
			  tdrpToken_t *tokens, int ntok);

static int
define_enum_from_typedef(token_handle_t *tok_handle,
			 typedef_handle_t *tdef_handle,
			 tdrpToken_t *tokens, int ntok);

static int
define_struct_from_paramdef(token_handle_t *tok_handle,
			    char *struct_name,
			    int start_tok, int end_tok,
			    tdrpToken_t *tokens, int ntok);
     
static int
define_struct_from_typedef(token_handle_t *tok_handle,
			   typedef_handle_t *tdef_handle,
			   tdrpToken_t *tokens, int ntok);

static int parse_commentdef(token_handle_t *tok_handle,
			    TDRPtable *tt, int start_pos,
			    tdrpToken_t *tokens, int ntok);

static int parse_paramdef(token_handle_t *tok_handle,
			  TDRPtable *tt, int start_pos,
			  tdrpToken_t *tokens, int ntok);

static int parse_typedef(token_handle_t *tok_handle,
			 typedef_handle_t *tdef_handle,
			 int start_pos,
			 tdrpToken_t *tokens, int ntok);

static int set_commentdef(token_handle_t *tok_handle,
			  TDRPtable *tt,
			  tdrpToken_t *tokens, int ntok);

static int set_paramdef(token_handle_t *tok_handle,
			TDRPtable *tt,
			tdrpToken_t *tokens, int ntok);

/*******************
 * primary routine
 */

int parse_defs(token_handle_t *tok_handle,
	       int ntok, tdrpToken_t *tokens,
	       TDRPtable *t_entries, int max_defs,
	       int *n_defs_p, int debug)
     
{
  
  int iret = 0;
  int i;
  int n_defs = *n_defs_p;
  TDRPtable *tt = t_entries;
  typedef_handle_t tdef_handle;

  Debug = debug;

  /*
   * check for 'char' on its own
   */
  
  for (i = 0; i < ntok; i++) {
    if (!strcmp(tokens[i].tok, "char")) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "Illegal type 'char', must be 'char*' or 'string'\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[i]));
      iret = -1;
    }
  }

  /*
   * parse the typedefs first
   */

  for (i = 0; i < ntok; i++) {

    if (!strcmp(tokens[i].tok, "typedef")) {

      if (parse_typedef(tok_handle, &tdef_handle, i+1, tokens, ntok)) {
	iret = -1;
	continue;
      }

      /*
       * register the typedefs in the lists
       */
      
      if (tdef_handle.type == ENUM_TYPE) {
	if (define_enum_from_typedef(tok_handle, &tdef_handle, tokens, ntok)) {
	  iret = -1;
	}
      } else if (tdef_handle.type == STRUCT_TYPE) {
	if (define_struct_from_typedef(tok_handle, &tdef_handle, tokens, ntok)) {
	  iret = -1;
	}
      }

      /*
       * advance
       */
      
      i = tdef_handle.end_tok;

    } /* if (!strcmp(tokens[i].tok, "typedef")) */

  } /* i */

  /*
   * parse comments and paramdefs
   */

  for (i = 0; i < ntok; i++) {

    if (!strcmp(tokens[i].tok, "paramdef")) {

      memset(tt, 0, sizeof(TDRPtable));

      if (parse_paramdef(tok_handle, tt, i+1, tokens, ntok)) {
	iret = -1;
	continue;
      }
      if (tt->ptype == ENUM_TYPE) {
	if (define_enum_from_paramdef(tok_handle, tt->enum_def.name,
				      tt->start_tok, tt->end_tok,
				      tokens, ntok)) {
	  iret = -1;
	}
      } else if (tt->ptype == STRUCT_TYPE) {
	if (define_struct_from_paramdef(tok_handle, tt->struct_def.name,
					tt->start_tok, tt->end_tok,
					tokens, ntok)) {
	  iret = -1;
	}
      }
      if (set_paramdef(tok_handle, tt, tokens, ntok)) {
	iret = -1;
	continue;
      }

      i = tt->end_tok;
      n_defs++;
      tt++;

    } else if (!strcmp(tokens[i].tok, "commentdef")) {

      memset(tt, 0, sizeof(TDRPtable));

      if (parse_commentdef(tok_handle, tt, i+1, tokens, ntok)) {
	iret = -1;
	continue;
      }
      if (set_commentdef(tok_handle, tt, tokens, ntok)) {
	iret = -1;
	continue;
      }

      i = tt->end_tok;
      n_defs++;
      tt++;

    }

    if (n_defs >= max_defs) {
      break;
    }

  } /* i */

  if (iret == 0) {
    *n_defs_p = n_defs;
    return (0);
  } else {
    return (-1);
  }

}


/******************
  * parse_paramdef()
  *
  * parse a param definition
  */

static int parse_paramdef(token_handle_t *tok_handle,
			  TDRPtable *tt, int start_pos,
			  tdrpToken_t *tokens, int ntok)

{

  char *type_str;
  char *param_name = NULL;
  char *enum_name = NULL;
  char *struct_name = NULL;

  int ptype;
  int itok = start_pos;
  int jtok;
  int n_unpaired;
  int is_array = FALSE;
  int is_array2D = FALSE;
  int array_len_fixed = FALSE;
  int array_n = 0;
  int array_n1 = 0;
  int array_n2 = 0;
  int last_tok_used, is_fixed, fixed_len;

  /*
   * initialize
   */

  tt->start_tok = start_pos;
  tt->end_tok = start_pos;

  /*
   * get type
   */

  type_str = tokens[itok++].tok;
  ptype = tdrpStr2TableEntry(type_str);

  if (ptype < 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Unknown type %s\n", type_str);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_pos]));
    return (-1);
  }

  /*
   * For enums and structs, check for a specific name.
   * If the next token is '{', no name is specified and
   * the param name will be used.
   */

  if (ptype == ENUM_TYPE) {
    if (strcmp(tokens[itok].tok, "{")) {
      enum_name = tokens[itok++].tok;
    }
  } else if (ptype == STRUCT_TYPE) {
    if (strcmp(tokens[itok].tok, "{")) {
      struct_name = tokens[itok++].tok;
    }
  }

  /*
   * check for starting '{'
   */

  if (strcmp(tokens[itok].tok, "{")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No opening '{'\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  /*
   * set start token to opening brace
   */

  tt->start_tok = itok;
  tt->end_tok = itok;

  /*
   * find end '}'
   */

  n_unpaired = 1;

  for (jtok = itok + 1; jtok < ntok - 2; jtok++) {

    if (!strcmp(tokens[jtok].tok, "{")) {
      n_unpaired++;
    } else if (!strcmp(tokens[jtok].tok, "}")) {
      n_unpaired--;
      if (n_unpaired == 0) {
	break;
      }
    }

  } /* jtok */

  if (n_unpaired != 0) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "paramdef type %s\n", type_str);
    fprintf(stderr, "No closing '}'\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  /*
   * set end token to closing brace
   */

  tt->end_tok = jtok;
  jtok++;

  /*
   * get param_name
   */

  param_name = tokens[jtok].tok;
  if (tdrpReservedStr(param_name)) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Param name missing\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[jtok]));
    return (-1);
  }
  jtok++;

  /*
   * check if param is array
   */

  if (tdrpFindArrayBrackets(tok_handle,
			    jtok, tokens, ntok, &last_tok_used, 
			    &is_fixed, &fixed_len) == 0) {

    is_array = TRUE;
    if (is_fixed) {
      array_len_fixed = TRUE;
      array_n = fixed_len;
    } else {
      array_n = 1;
    }
    jtok = last_tok_used + 1;

    /*
     * check if param is 2D array
     */
    
    if (tdrpFindArrayBrackets(tok_handle,
			      jtok, tokens, ntok, &last_tok_used, 
			      &is_fixed, &fixed_len) == 0) {
      
      /*
       * check for consistency in fixed or varying lengths
       */
      
      if ((is_fixed && !array_len_fixed) ||
	  (!is_fixed && array_len_fixed)) {
	fprintf(stderr,
		"\n>>> TDRP_ERROR <<< - param '%s'\n", param_name);
	fprintf(stderr, "For 2D arrays, dims must be either both "
		"fixed or varying\n");
	fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[jtok]));
	return (-1);
      }
      
      /*
       * check this is not a struct
       */
      
      if (ptype == STRUCT_TYPE) {
	fprintf(stderr,
		"\n>>> TDRP_ERROR <<< - param '%s'\n", param_name);
	fprintf(stderr, "2D arrays not valid for structs.\n");
	fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[jtok]));
	return (-1);
      }
      
      is_array2D = TRUE;
      array_n1 = array_n;
      if (is_fixed) {
	array_n2 = fixed_len;
      } else {
	array_n2 = 1;
      }
      array_n = array_n1 * array_n;
      
      jtok = last_tok_used + 1;

    } /* tdrpFindArrayBrackets - 2D array */

  } /* tdrpFindArrayBrackets - 1D array */

  /*
   * check for closing ';'
   */

  if (strcmp(tokens[jtok].tok, ";")) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Expecting final ';', found '%s'\n",
	    tokens[jtok].tok);
    fprintf(stderr, "Param %s\n", param_name);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[jtok]));
    return (-1);
  }

  /*
   * fill out table entry as appropriate
   */

  tt->param_name = tdrpStrDup(param_name);
  tt->ptype = (tableEntry_t) ptype;

  if (ptype == ENUM_TYPE) {
    if (enum_name == NULL) {
      tt->enum_def.name = tdrpStrDup(param_name);
    } else {
      tt->enum_def.name = tdrpStrDup(enum_name);
    }
  } else if (ptype == STRUCT_TYPE) {
    if (struct_name == NULL) {
      tt->struct_def.name = tdrpStrDup(param_name);
    } else {
      tt->struct_def.name = tdrpStrDup(struct_name);
    }
  }

  if (is_array) {
    tt->is_array = TRUE;
    tt->array_len_fixed = array_len_fixed;
    tt->array_n = array_n;
  }
  if (is_array2D) {
    tt->is_array2D = TRUE;
    tt->array_n1 = array_n1;
    tt->array_n2 = array_n2;
  }

  if (Debug) {
    fprintf(stderr, "------------------------------------------\n");
    fprintf(stderr, "paramdef for '%s'\n", tt->param_name);
    fprintf(stderr, "  type %s\n", tdrpFieldType2Str(tt->ptype));
    if (tt->is_array) {
      fprintf(stderr, "is_array\n");
      if (tt->array_len_fixed) {
	fprintf(stderr, "  fixed length array, %d elems\n",
		tt->array_n);
      } else {
	fprintf(stderr, "  variable length array\n");
      }
    }
    if (tt->is_array2D) {
      fprintf(stderr, "is_array2D\n");
    }
    fprintf(stderr, "  Start line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
    fprintf(stderr, "  End line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tt->end_tok]));
  }

  return(0);

}

/*****************
  * parse_typedef()
  *
  * parse a type definition
  */

static int parse_typedef(token_handle_t *tok_handle,
			 typedef_handle_t *tdef_handle, int start_pos,
			 tdrpToken_t *tokens, int ntok)

{

  char *type_str;
  char *param_name = NULL;

  int ptype;
  int itok = start_pos;
  int jtok;
  int n_unpaired;
  int n_pairs;

  /*
   * initialize
   */

  tdef_handle->start_tok = start_pos;
  tdef_handle->end_tok = start_pos;

  /*
   * get type
   */

  type_str = tokens[itok++].tok;
  ptype = tdrpStr2TableEntry(type_str);

  if (ptype != ENUM_TYPE && ptype != STRUCT_TYPE) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Unknown typedef type %s\n", type_str);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_pos]));
    return (-1);
  }

  /*
   * check for starting '{'
   */

  if (strcmp(tokens[itok].tok, "{")) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "  Bad token '%s'.\n", tokens[itok].tok);
    fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    fprintf(stderr, "  Opening '{' for typedef must follow '%s'.\n",
	    type_str);
    fprintf(stderr, "  No %s name allowed.\n", type_str);
    return (-1);
  }

  /*
   * set start token to opening brace
   */

  tdef_handle->start_tok = itok;
  tdef_handle->end_tok = itok;

  /*
   * find end '}'
   */

  n_unpaired = 1;
  n_pairs = 1;
  for (jtok = itok + 1; jtok < ntok - 2; jtok++) {
    if (!strcmp(tokens[jtok].tok, "{")) {
      n_unpaired++;
      n_pairs++;
    } else if (!strcmp(tokens[jtok].tok, "}")) {
      n_unpaired--;
      if (n_unpaired == 0) {
	break;
      }
    }
  } /* jtok */

  if (n_unpaired != 0) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No closing '}', %s typedef\n", type_str);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  if (n_pairs != 1) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Nested braces, %s typedef\n", type_str);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  /*
   * set end token to closing brace
   */

  tdef_handle->end_tok = jtok;
  jtok++;

  /*
   * get typdef type
   */

  param_name = tokens[jtok].tok;
  jtok++;

  /*
   * check for closing ';'
   */

  if (strcmp(tokens[jtok].tok, ";")) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "Expecting final ';', found '%s'\n",
	    tokens[jtok].tok);
    fprintf(stderr, "Param %s\n", param_name);
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[jtok]));
    return (-1);
  }

  /*
   * fill out table entry as appropriate - this is temporary only,
   * the entry will be reused so we do not allocate strings;
   */
  
  tdef_handle->name = param_name;
  tdef_handle->type = ptype;

  if (Debug) {
    fprintf(stderr, "------------------------------------------\n");
    if (ptype == ENUM_TYPE) {
      fprintf(stderr, "typedef enum for '%s'\n", tdef_handle->name);
    } else if (ptype == STRUCT_TYPE) {
      fprintf(stderr, "typedef struct for '%s'\n", tdef_handle->name);
    }
    fprintf(stderr, "  Start line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tdef_handle->start_tok]));
    fprintf(stderr, "  End   line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tdef_handle->end_tok]));
  }

  return(0);

}

/********************
  * parse_commentdef()
  *
  * parse a comment definition
  */

static int parse_commentdef(token_handle_t *tok_handle,
			    TDRPtable *tt, int start_pos,
			    tdrpToken_t *tokens, int ntok)

{

  char name[256];
  static int count = 0;
  int itok = start_pos;
  int jtok;
  int n_unpaired;

  /*
   * initialize
   */

  tt->start_tok = start_pos;
  tt->end_tok = start_pos;

  /*
   * check for starting '{'
   */

  if (strcmp(tokens[itok].tok, "{")) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No opening '{'\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  /*
   * set start token to opening brace
   */

  tt->start_tok = itok;
  tt->end_tok = itok;

  /*
   * find end '}'
   */

  n_unpaired = 1;

  for (jtok = itok + 1; jtok < ntok - 2; jtok++) {

    if (!strcmp(tokens[jtok].tok, "{")) {
      n_unpaired++;
    } else if (!strcmp(tokens[jtok].tok, "}")) {
      n_unpaired--;
      if (n_unpaired == 0) {
	break;
      }
    }

  } /* jtok */

  if (n_unpaired != 0) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "No closing '}', commentdef\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
    return (-1);
  }

  /*
   * set end token to closing brace
   */

  tt->end_tok = jtok;

  /*
   * fill out table entry as appropriate
   */

  tt->ptype = COMMENT_TYPE;
  sprintf(name, "Comment %d", count);
  tt->param_name = tdrpStrDup(name);

  if (Debug) {
    fprintf(stderr, "------------------------------------------\n");
    fprintf(stderr, "commentdef number %d\n", count);
    fprintf(stderr, "  Start line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
    fprintf(stderr, "  End   line num: %s\n",
	    tdrpLineInfo(tok_handle, &tokens[tt->end_tok]));
  }

  count++;
  return(0);

}

/****************
  * set_paramdef()
  *
  * Set items in a parameter definition
  */

static int set_paramdef(token_handle_t *tok_handle,
			TDRPtable *tt,
			tdrpToken_t *tokens, int ntok)

{

  int iret = 0;

  /*
   * get description, help and private - these apply to
   * all types
   */

  if (set_descr(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  if (set_help(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  if (set_private(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }

  switch (tt->ptype) {

  case BOOL_TYPE:
    if (set_bool(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case INT_TYPE:
    if (set_int(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case LONG_TYPE:
    if (set_long(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case FLOAT_TYPE:
    if (set_float(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case DOUBLE_TYPE:
    if (set_double(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case STRING_TYPE:
    if (set_string(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case ENUM_TYPE:
    if (set_enum(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
    break;

  case STRUCT_TYPE:
    if (set_struct(tok_handle, tt, tokens, ntok)) {
      iret = -1;
    }
#ifdef NOTNOW
    if (tt->is_array && tt->n_struct_vals == 0) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load struct p_default.\n");
      fprintf(stderr,
	      "This is an array, but you have not included any entries.\n");
      fprintf(stderr, "Make sure you have nested the braces correctly.\n");
      fprintf(stderr,
	      "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
#endif

    break;

  case COMMENT_TYPE:
    break;

  } /* switch */

  return (iret);

}


/****************************
 * define_enum_from_typedef()
 *
 * Creates an enum definition from within a typedef
 */

static int define_enum_from_typedef(token_handle_t *tok_handle,
				    typedef_handle_t *tdef_handle,
				    tdrpToken_t *tokens, int ntok)
     
{
  
  int i, n_fields;
  enum_field_t *fields;
  tdrpBuf *field_buf;
  
  field_buf = tdrpBufCreate();
  
  if (set_enum_fields(tok_handle, tdef_handle->start_tok, tdef_handle->end_tok,
		    tokens, ntok, tdef_handle->name, NULL,
		    field_buf)) {
    tdrpBufDelete(field_buf);
    return (-1);
  }
  
  n_fields = tdrpBufLen(field_buf) / sizeof(enum_field_t);
  fields = (enum_field_t *) tdrpBufPtr(field_buf);
  
  /*
   * add to list
   */
  
  if (enum_def_add(tdef_handle->name, n_fields, fields)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< \n");
    fprintf(stderr, "  %s\n",
            tdrpLineInfo(tok_handle, &tokens[tdef_handle->start_tok]));
    tdrpBufDelete(field_buf);
    return (-1);
  }

  /*
   * free up
   */
  
  for (i = 0; i < n_fields; i++) {
    tdrpFree(fields[i].name);
  }
  tdrpBufDelete(field_buf);

  return (0);
    
}

/*****************************
 * define_enum_from_paramdef()
 *
 * Creates an enum definition from within a paramdef
 */

static int
define_enum_from_paramdef(token_handle_t *tok_handle,
			  char *enum_name,
			  int start_tok, int end_tok,
			  tdrpToken_t *tokens, int ntok)

{
  
  int i, nfields;
  int label_tok;
  enum_field_t *fields;
  enum_def_t *def;
  tdrpBuf *field_buf;
  
  if ((def = enum_def_by_name(enum_name)) != NULL) {

    /*
     * enum already defined - check that we are not trying to 
     * define it again.
     */
    
    if (tdrpFindLabel(start_tok, end_tok,
		      tokens, ntok, "p_options", &label_tok) == 0) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< enum '%s'\n", enum_name);
      fprintf(stderr, "Enum '%s' previously defined.\n", enum_name);
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
      return (-1);
    } else {
      return (0);
    }

  }
  
  /*
   * not defined yet - do so now
   */
  
  field_buf = tdrpBufCreate();
  
  if (set_enum_fields(tok_handle, start_tok, end_tok,
		    tokens, ntok, enum_name, "p_options",
		    field_buf)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< enum '%s'\n", enum_name);
    fprintf(stderr, "Cannot set fields, check syntax.\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    tdrpBufDelete(field_buf);
    return (-1);
  }
  
  nfields = tdrpBufLen(field_buf) / sizeof(enum_field_t);
  fields = (enum_field_t *) tdrpBufPtr(field_buf);
  
  /*
   * add to list
   */
  
  if (enum_def_add(enum_name, nfields, fields)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    tdrpBufDelete(field_buf);
    return (-1);
  }
  
  /*
   * free up
   */
  
  for (i = 0; i < nfields; i++) {
    tdrpFree(fields[i].name);
  }
  tdrpBufDelete(field_buf);

  return (0);
  
}

/******************************
 * define_struct_from_typedef()
 *
 * Creates a struct definition from within a typedef
 */

static int define_struct_from_typedef(token_handle_t *tok_handle,
				      typedef_handle_t *tdef_handle,
				      tdrpToken_t *tokens, int ntok)
     
{

  int i, n_fields;
  struct_field_t *fields;
  tdrpBuf *field_buf;
  
  field_buf = tdrpBufCreate();
  
  if (set_struct_fields(tok_handle, tdef_handle->start_tok, tdef_handle->end_tok,
		      tokens, ntok, tdef_handle->name, NULL,
		      field_buf)) {
    tdrpBufDelete(field_buf);
    return (-1);
  }
  
  n_fields = tdrpBufLen(field_buf) / sizeof(struct_field_t);
  fields = (struct_field_t *) tdrpBufPtr(field_buf);
  
  /*
   * add to list
   */
  
  if (struct_def_add(tdef_handle->name, n_fields, fields)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tdef_handle->start_tok]));
    tdrpBufDelete(field_buf);
    return (-1);
  }

  /*
   * free up
   */
  
  for (i = 0; i < n_fields; i++) {
    tdrpFree(fields[i].ftype);
    tdrpFree(fields[i].fname);
  }
  tdrpBufDelete(field_buf);

  return (0);
    
}

/*******************************
 * define_struct_from_paramdef()
 *
 * Create a struct definition from within a paramdef
 */

static int define_struct_from_paramdef(token_handle_t *tok_handle,
				       char *struct_name,
				       int start_tok, int end_tok,
				       tdrpToken_t *tokens, int ntok)
     
{

  int ifield;
  int nfields, nvars;
  int field_tok, var_tok, last_tok;
  int label_tok;
  tdrpToken_t *token_list;
  tdrpToken_t *token;
  struct_field_t *fields;
  struct_def_t *def;

  if ((def = struct_def_by_name(struct_name)) != NULL) {

    /*
     * struct already defined - check that we are not trying to 
     * define it again.
     */
    
    if (tdrpFindLabel(start_tok, end_tok,
		      tokens, ntok, "p_field_type", &label_tok) == 0) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< struct '%s'\n",
	      struct_name);
      fprintf(stderr,
	      "p_field_type illegal, struct '%s' previously defined.\n",
	      struct_name);
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
      return (-1);
    }
    
    if (tdrpFindLabel(start_tok, end_tok,
		      tokens, ntok, "p_field_name", &label_tok) == 0) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< struct '%s'\n",
	      struct_name);
      fprintf(stderr,
	      "p_field_name illegal, struct '%s' previously defined.\n",
	      struct_name);
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
      return (-1);
    }
    
    return (0);

  }
    
  /*
   * get field type list
   */
  
  tdrpTokenListReset(tok_handle);
  if (tdrpFindMultipleItem(tok_handle,
			   start_tok, end_tok, tokens, ntok,
			   "p_field_type", &field_tok, &last_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "  Cannot parse p_field_type in struct '%s'\n",
	    struct_name);
    fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    return (-1);
  }
  nfields = tdrpNTokenList(tok_handle);

  /*
   * allocate array
   */
  
  fields = (struct_field_t *)
    tdrpMalloc(nfields * sizeof(struct_field_t));

  /*
   * load fields types
   */
  
  token_list = tdrpTokenListPtr(tok_handle);
  for (ifield = 0; ifield < nfields; ifield++) {
    token = &token_list[ifield];
    fields[ifield].ftype = tdrpStrDup(token->tok);
  }

  /*
   * get var name list
   */
  
  tdrpTokenListReset(tok_handle);
  if (tdrpFindMultipleItem(tok_handle,
			   start_tok, end_tok, tokens, ntok,
			   "p_field_name", &var_tok, &last_tok)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "  Cannot parse p_field_name in struct '%s'\n",
	    struct_name);
    fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    for (ifield = 0; ifield < nfields; ifield++) {
      tdrpFree(fields[ifield].ftype);
    }
    tdrpFree(fields);
    return (-1);
  }
  nvars = tdrpNTokenList(tok_handle);
  
  /*
   * check nfields and nvars are equal
   */
  
  if (nvars != nfields) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "  Struct '%s'\n", struct_name);
    fprintf(stderr, "  Struct must have same number of names as types\n");
    fprintf(stderr, "  Found %d types and %d names\n", nfields, nvars);
    fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    for (ifield = 0; ifield < nfields; ifield++) {
      tdrpFree(fields[ifield].ftype);
    }
    tdrpFree(fields);
    return (-1);
  }

  /*
   * load field vars
   */
  
  token_list = tdrpTokenListPtr(tok_handle);
  for (ifield = 0; ifield < nfields; ifield++) {
    token = &token_list[ifield];
    fields[ifield].fname = tdrpStrDup(token->tok);
  }

  /*
   * add to struct list
   */
  
  if (struct_def_add(struct_name, nfields, fields)) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[start_tok]));
    for (ifield = 0; ifield < nfields; ifield++) {
      tdrpFree(fields[ifield].fname);
      tdrpFree(fields[ifield].ftype);
    }
    tdrpFree(fields);
    return (-1);
  }

  /*
   * free up
   */
  
  for (ifield = 0; ifield < nfields; ifield++) {
    tdrpFree(fields[ifield].fname);
    tdrpFree(fields[ifield].ftype);
  } /* ifield */
  tdrpFree(fields);
  
  return (0);
}

/******************
 * set_commentdef()
 *
 * Set items in a comment definition
 */

static int set_commentdef(token_handle_t *tok_handle,
			  TDRPtable *tt,
			  tdrpToken_t *tokens, int ntok)

{

  if (set_comment_text(tok_handle, tt, tokens, ntok)) {
    return (-1);
  } else {
    return (0);
  }

}

