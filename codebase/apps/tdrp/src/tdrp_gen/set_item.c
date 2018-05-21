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
 * set_item.c
 *
 * Sets items in table
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Jan 1998
 *
 ****************************************************************/

#include "tdrp_gen.h"
#include <assert.h>
#include <float.h>
#include <limits.h>

static int check_no_min_and_max(token_handle_t *tok_handle,
				TDRPtable *tt,
				tdrpToken_t *tokens, int ntok);
     
static void initialize_val(TDRPtable *tt, tdrpVal_t *init_val);

static int set_min_and_max(token_handle_t *tok_handle,
			   TDRPtable *tt,
			   tdrpToken_t *tokens, int ntok);
     
/************
 * set_bool()
 *
 * Set items relevant to boolean
 */

int set_bool(token_handle_t *tok_handle,
	     TDRPtable *tt, tdrpToken_t *tokens, int ntok)

{

  int deflt_tok;
  tdrpVal_t init_val;
  
  /*
   * initialize
   */

  init_val.b = pFALSE;
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("boolean");

  /*
   * check that there is no min and max set
   */
  
  if (check_no_min_and_max(tok_handle, tt, tokens, ntok)) {
    return (-1);
  }

  /*
   * read default if there is one
   */

  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadBool(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load enum p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      return (-1);
    }
  }

  return (0);

}

/***********
 * set_int()
 *
 * Set items relevant to int
 */

int set_int(token_handle_t *tok_handle,
	     TDRPtable *tt, tdrpToken_t *tokens, int ntok)
     
{

  int deflt_tok;
  int iret = 0;
  tdrpVal_t init_val;

  /*
   * load up max and min
   */
  
  if (set_min_and_max(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  
  /*
   * initialize with mean
   */

  init_val.i = (tt->min_val.i + tt->max_val.i) / 2;
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("int");

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadInt(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load int p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
  }

  if (tdrpCheckValRange(tt)) {
    iret = -1;
  }

  return (iret);

}
     
/************
 * set_long()
 *
 * Set items relevant to long
 */

int set_long(token_handle_t *tok_handle,
	     TDRPtable *tt, tdrpToken_t *tokens, int ntok)
     
{

  int deflt_tok;
  int iret = 0;
  tdrpVal_t init_val;

  /*
   * load up max and min
   */
  
  if (set_min_and_max(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  
  /*
   * initialize with mean
   */

  init_val.l = (tt->min_val.l + tt->max_val.l) / 2;
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("long");

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadLong(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load long p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
  }
  
  if (tdrpCheckValRange(tt)) {
    iret = -1;
  }
  
  return (iret);

}
     
/*************
 * set_float()
 *
 * Set items relevant to float
 */

int set_float(token_handle_t *tok_handle,
	      TDRPtable *tt, tdrpToken_t *tokens, int ntok)
     
{

  int deflt_tok;
  int iret = 0;
  tdrpVal_t init_val;

  /*
   * load up max and min
   */
  
  if (set_min_and_max(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  
  /*
   * initialize with mean
   */

  init_val.f = (tt->min_val.f + tt->max_val.f) / 2.0;
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("float");

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadFloat(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load float p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
  }
  
  if (tdrpCheckValRange(tt)) {
    iret = -1;
  }
  
  return (iret);

}
     
/**************
 * set_double()
 *
 * Set items relevant to double
 */

int set_double(token_handle_t *tok_handle,
	       TDRPtable *tt, tdrpToken_t *tokens, int ntok)
     
{

  int deflt_tok;
  int iret = 0;
  tdrpVal_t init_val;

  /*
   * load up max and min
   */
  
  if (set_min_and_max(tok_handle, tt, tokens, ntok)) {
    iret = -1;
  }
  
  /*
   * initialize with mean
   */

  init_val.d = (tt->min_val.d + tt->max_val.d) / 2.0;
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("double");

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadDouble(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load double p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
  }
  
  if (tdrpCheckValRange(tt)) {
    iret = -1;
  }
  
  return (iret);
  
}
     
/**************
 * set_string()
 *
 * Set items relevant to string
 */

int set_string(token_handle_t *tok_handle,
	       TDRPtable *tt,
	       tdrpToken_t *tokens, int ntok)

{

  int deflt_tok;
  int iret = 0;
  tdrpVal_t init_val;

  /*
   * initialize
   */

  init_val.s = "not_set";
  initialize_val(tt, &init_val);

  /*
   * set c type
   */
  
  tt->ctype = ctype_index_by_iname("char*");

  /*
   * check that there is no min and max set
   */
  
  if (check_no_min_and_max(tok_handle, tt, tokens, ntok)) {
    return (-1);
  }

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadString(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load string p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      iret = -1;
    }
  }
  
  return (iret);

}

/************
 * set_enum()
 *
 * Set items relevant to enum
 */

int set_enum(token_handle_t *tok_handle,
	     TDRPtable *tt,
	     tdrpToken_t *tokens, int ntok)

{

  int i;
  enum_def_t *def;
  tdrpVal_t init_val;

  if ((def = enum_def_by_name(tt->enum_def.name)) == NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< enum '%s'\n",
	    tt->enum_def.name);
    fprintf(stderr,
	    "Enum not properly defined either using typedef or paramdef.\n");
    fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
    return (-1);
  }

  /*
   * load entry
   */
  
  tt->enum_def.nfields = def->nfields;
  tt->enum_def.fields =
    (enum_field_t *) tdrpMalloc(def->nfields * sizeof(enum_field_t));
  for (i = 0; i < def->nfields; i++) {
    tt->enum_def.fields[i].name = tdrpStrDup(def->fields[i].name);
    tt->enum_def.fields[i].val = def->fields[i].val;
  }
  
  /*
   * set ctype
   */
  
  tt->ctype = ctype_index_by_iname(tt->enum_def.name);

  /*
   * check that there is no min and max set
   */
  
  if (check_no_min_and_max(tok_handle, tt, tokens, ntok)) {
    return (-1);
  }

  /*
   * initialize
   */

  init_val.e = tt->enum_def.fields[0].val;
  initialize_val(tt, &init_val);
  
  /*
   * get default
   */
  
  if (tdrpReadEnum(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
    return (-1);
  }

  return (0);

}

/*******************
 * set_enum_fields()
 *
 * Sets fields and vals for an enum.
 * If field_label is non-null, it searches for the label
 * first before finding the enum list.
 *
 * Legal syntax is:
 *   [field_label = ] { item, item, item ... }
 * or
 *   [field_label = ] { item = val, item = val, item = val ... }
 *
 * Returns 0 on success, -1 on failure.
 */

int set_enum_fields(token_handle_t *tok_handle,
		    int start_tok, int end_tok,
		    tdrpToken_t *tokens, int ntok,
		    char *enum_name, char *field_label,
		    tdrpBuf *field_buf)

{

  char message_label[256];
  char *name;
  int field_label_tok;
  int itok, stok;
  int braces_start, braces_end;
  int n_gap, n_prs_enclosed;
  int val, prev_val = -1;
  enum_field_t field;

  /*
   * search for item label if required
   */

  if (field_label != NULL) {

    if (tdrpFindLabel(start_tok, end_tok, tokens, ntok,
		      field_label, &field_label_tok)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
      fprintf(stderr, "    enum name %s\n", enum_name);
      fprintf(stderr, "  Failed search for label '%s'\n", field_label);
      fprintf(stderr, "    between %s\n",
	      tdrpLineInfo(tok_handle, &tokens[start_tok]));
      fprintf(stderr, "    and     %s\n",
	      tdrpLineInfo(tok_handle, &tokens[end_tok]));
      return (-1);
    }

    if (strcmp(tokens[field_label_tok + 1].tok, "=")) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
      fprintf(stderr, "    enum name %s\n", enum_name);
      fprintf(stderr, "    no '=' found for item %s\n", field_label);
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[field_label_tok]));
      return (-1);
    }
    
    stok = field_label_tok;

  } else {

    stok = start_tok;

  }

  /*
   * find the braces pair surrounding the enum
   */
  
  sprintf(message_label, "%s: enum name '%s'",
	  "set_enum_fields", enum_name);
  
  if (tdrpFindBracesPair(tok_handle, message_label, stok, end_tok,
			 tokens, ntok,
			 &n_gap, &n_prs_enclosed,
			 &braces_start, &braces_end)) {
    return (-1);
  }

  if (n_prs_enclosed > 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
    fprintf(stderr, "    enum name %s\n", enum_name);
    fprintf(stderr, "    No enclosed braces allowed\n");
    fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[stok]));
    return (-1);
  }
  
  /*
   * check through the list between the braces, finding how many elements
   * there are and checking the syntax
   */

  itok = braces_start + 1;
  while (itok < braces_end) {

    /*
     * check enum field is not reserved string
     */

    name = tokens[itok].tok;
    if (tdrpReservedStr(name)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
      fprintf(stderr, "    enum name %s\n", enum_name);
      fprintf(stderr, "    Invalid enum field.\n");
      fprintf(stderr, "    '%s' out of place\n", name);
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
      return (-1);
    }
    itok++;
    
    /*
     * if val is supplied use it, else create one
     */
    
    if (!strcmp(tokens[itok].tok, "=")) {
      if (sscanf(tokens[itok+1].tok, "%d", &val) != 1) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
	fprintf(stderr, "    enum name %s\n", enum_name);
	fprintf(stderr, "    Invalid enum val '%s'\n",
		tokens[itok+1].tok);
	fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok+1]));
	return (-1);
      }
      itok += 2;
    } else {
      val = prev_val + 1;
    }
    prev_val = val;
    
    /*
     * add field to buffer
     */
    
    field.val = val;
    field.name = tdrpStrDup(name);
    tdrpBufAdd(field_buf, &field, sizeof(enum_field_t));

    /*
     * check for trailing comma
     */

    if (!strcmp(tokens[itok].tok, ",")) {
      itok++;
    } else {
      if (itok != braces_end) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< set_enum_fields\n");
	fprintf(stderr, "    enum name %s\n", enum_name);
	fprintf(stderr, "    Invalid syntax\n");
	fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
	return (-1);
      }
    }

  } /* while */

  return (0);

}

/**************
 * set_struct()
 *
 * Set items relevant to struct
 */

int set_struct(token_handle_t *tok_handle,
	       TDRPtable *tt,
	       tdrpToken_t *tokens, int ntok)
     
{

  int ifield;
  int j;
  int deflt_tok;
  enum_def_t *edef1, *edef2;
  struct_def_t *def;

  if ((def = struct_def_by_name(tt->struct_def.name)) == NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< struct '%s'\n",
	    tt->struct_def.name);
    fprintf(stderr,
	    "Struct not properly defined either using typedef or paramdef.\n");
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
    return (-1);
  }

  tt->struct_def.nfields = def->nfields;

  /*
   * allocate fields array
   */
  
  tt->struct_def.fields =
    (struct_field_t *) tdrpMalloc(def->nfields * sizeof(struct_field_t));
  
  /*
   * load fields
   */
  
  for (ifield = 0; ifield < def->nfields; ifield++) {
    tt->struct_def.fields[ifield] = def->fields[ifield];
    tt->struct_def.fields[ifield].ftype =
      tdrpStrDup(def->fields[ifield].ftype);
    tt->struct_def.fields[ifield].fname =
      tdrpStrDup(def->fields[ifield].fname);
    if (tt->struct_def.fields[ifield].ptype == ENUM_TYPE) {
      edef1 = &def->fields[ifield].enum_def;
      edef2 = &tt->struct_def.fields[ifield].enum_def;
      edef2->nfields = edef1->nfields;
      edef2->name = tdrpStrDup(edef1->name);
      edef2->fields = (enum_field_t *)
	tdrpMalloc (edef2->nfields * sizeof(enum_field_t));
      for (j = 0; j < edef2->nfields; j++) {
	edef2->fields[j].name = tdrpStrDup(edef1->fields[j].name);
	edef2->fields[j].val = edef1->fields[j].val;
      }
    } /* if */
  } /* ifield */

  /*
   * set ctype for struct
   */
  
  tt->ctype = ctype_index_by_iname(tt->struct_def.name);

  /*
   * check that there is no min and max set
   */
  
  if (check_no_min_and_max(tok_handle, tt, tokens, ntok)) {
    return (-1);
  }

  /*
   * initialize fields
   */

  tt->n_struct_vals = tt->struct_def.nfields;

  tt->struct_vals =(tdrpVal_t *) tdrpMalloc
    (tt->n_struct_vals * sizeof(tdrpVal_t));

  for (ifield = 0; ifield < tt->struct_def.nfields; ifield++) {

    switch (tt->struct_def.fields[ifield].ptype) {
    case BOOL_TYPE:
      tt->struct_vals[ifield].b = pFALSE;
      break;
    case STRING_TYPE:
      tt->struct_vals[ifield].s = tdrpStrDup("not_set");
      break;
    case INT_TYPE:
      tt->struct_vals[ifield].i = 0;
      break;
    case LONG_TYPE:
      tt->struct_vals[ifield].l = 0;
      break;
    case FLOAT_TYPE:
      tt->struct_vals[ifield].f = 0.0;
      break;
    case DOUBLE_TYPE:
      tt->struct_vals[ifield].d = 0.0;
      break;
    case ENUM_TYPE:
      tt->struct_vals[ifield].e =
	tt->struct_def.fields[ifield].enum_def.fields[0].val;
      break;
    default:
      break;
    } /* switch */

  } /* ifield */

  /*
   * read default if applicable
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok,
		    tokens, ntok,
		    "p_default", &deflt_tok) == 0) {
    if (tdrpReadStruct(tok_handle, tt, "p_default", tokens, ntok, pFALSE)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
      fprintf(stderr, "Cannot load struct p_default.\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[tt->start_tok]));
      return (-1);
    }
  }
  
  return (0);

}

/*********************
 * set_struct_fields()
 *
 * Sets fields and vars for a struct.
 * If field_label is non-null, it searches for the label
 * first before finding the struct list.
 *
 * Legal syntax is:
 *   [field_label = ] { item var; item var; item var; ... }
 *
 * Returns 0 on success, -1 on failure.
 */

int set_struct_fields(token_handle_t *tok_handle,
		      int start_tok, int end_tok,
		      tdrpToken_t *tokens, int ntok,
		      char *struct_name, char *field_label,
		      tdrpBuf *field_buf)

{

  char message_label[256];
  char *fname;
  char *ftype;
  int field_label_tok;
  int itok, stok;
  int braces_start, braces_end;
  int n_gap, n_prs_enclosed;
  struct_field_t field;

  /*
   * search for item label if required
   */

  if (field_label != NULL) {
    
    if (tdrpFindLabel(start_tok, end_tok, tokens, ntok,
		      field_label, &field_label_tok)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
      fprintf(stderr, "    struct name %s\n", struct_name);
      fprintf(stderr, "  Failed search for label '%s'\n", field_label);
      fprintf(stderr, "    between %s\n",
	      tdrpLineInfo(tok_handle, &tokens[start_tok]));
      fprintf(stderr, "    and     %s\n",
	      tdrpLineInfo(tok_handle, &tokens[end_tok]));
      return (-1);
    }

    if (strcmp(tokens[field_label_tok + 1].tok, "=")) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
      fprintf(stderr, "    struct name %s\n", struct_name);
      fprintf(stderr, "    no '=' found for item %s\n", field_label);
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[field_label_tok]));
      return (-1);
    }
    
    stok = field_label_tok;

  } else {

    stok = start_tok;

  }

  /*
   * find the braces pair surrounding the struct
   */
  
  sprintf(message_label, "%s: struct name '%s'",
	  "set_struct_fields", struct_name);
  
  if (tdrpFindBracesPair(tok_handle, message_label, stok, end_tok,
			 tokens, ntok,
			 &n_gap, &n_prs_enclosed,
			 &braces_start, &braces_end)) {
    return (-1);
  }
  
  if (n_prs_enclosed > 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
    fprintf(stderr, "    struct name %s\n", struct_name);
    fprintf(stderr, "    No enclosed braces allowed\n");
    fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[stok]));
    return (-1);
  }
  
  /*
   * check through the list between the braces, finding how many elements
   * there are and checking the syntax
   */
  
  itok = braces_start + 1;
  while (itok < braces_end) {

    /*
     * check name and var are not reserved string
     */
    
    ftype = tokens[itok].tok;
    if (tdrpReservedStr(ftype)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
      fprintf(stderr, "    struct name %s\n", struct_name);
      fprintf(stderr, "    Invalid struct field\n");
      fprintf(stderr, "    '%s' out of place\n", ftype);
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
      return (-1);
    }
    itok++;
    
    fname = tokens[itok].tok;
    if (tdrpReservedStr(fname)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
      fprintf(stderr, "    struct name %s\n", struct_name);
      fprintf(stderr, "    Invalid struct var\n");
      fprintf(stderr, "    '%s' out of place\n", fname);
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
      return (-1);
    }
    itok++;
    
    /*
     * check for trailing colon
     */

    if (strcmp(tokens[itok].tok, ";")) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<< set_struct_fields\n");
      fprintf(stderr, "    struct name %s\n", struct_name);
      fprintf(stderr, "    Invalid syntax, no ';'\n");
      fprintf(stderr, "    %s\n", tdrpLineInfo(tok_handle, &tokens[itok]));
      return (-1);
    }
    itok++;
    
    /*
     * add field to buffer
     */

    field.ctype = ctype_index_by_iname(ftype);
    
    if (field.ctype < 0) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "  Illegal type in struct '%s'\n", struct_name);
      fprintf(stderr, "  Illegal type is '%s' for field '%s'\n",
	      ftype, fname);
      fprintf(stderr, "  %s\n", tdrpLineInfo(tok_handle, &tokens[stok]));
      return (-1);
    }

    field.ftype = tdrpStrDup(ftype);
    field.fname = tdrpStrDup(fname);
    
    tdrpBufAdd(field_buf, &field, sizeof(struct_field_t));
    
  } /* while */

  return (0);

}

/*************
 * set_descr()
 *
 * set description string
 */

int set_descr(token_handle_t *tok_handle,
	      TDRPtable *tt,
	      tdrpToken_t *tokens, int ntok)

{

  int descr_tok;
  char *descr;
  
  if (tdrpFindSingleItem(tok_handle, tt->start_tok, tt->end_tok, tokens, ntok,
			 "p_descr", &descr_tok) == 0) {

    /*
     * OK - set description
     */
    
    descr = tokens[descr_tok].tok;
    tt->descr = tdrpStrDup(descr);

  } else {

    tt->descr = tdrpStrDup("");

  }

  return (0);

}

/************
 * set_help()
 *
 * set help string
 */

int set_help(token_handle_t *tok_handle,
	     TDRPtable *tt,
	     tdrpToken_t *tokens, int ntok)

{

  int help_tok;
  char *help;
  
  if (tdrpFindSingleItem(tok_handle, tt->start_tok, tt->end_tok, tokens, ntok,
			 "p_help", &help_tok) == 0) {

    /*
     * OK - set help
     */
    
    help = tokens[help_tok].tok;
    tt->help = tdrpStrDup(help);

  } else {

    tt->help = tdrpStrDup("");

  }

  return (0);

}

/********************
 * set_comment_text()
 *
 * set comment text string
 */

int set_comment_text(token_handle_t *tok_handle,
		     TDRPtable *tt,
		     tdrpToken_t *tokens, int ntok)

{

  int text_tok;
  char *text;
  
  if (tdrpFindSingleItem(tok_handle, tt->start_tok, tt->end_tok, tokens, ntok,
			 "p_header", &text_tok) == 0) {
    
    /*
     * OK - set text
     */
    
    text = tokens[text_tok].tok;
    tt->comment_hdr = tdrpStrDup(text);
    
  } else {
    
    tt->comment_hdr = tdrpStrDup("");

  }

  if (tdrpFindSingleItem(tok_handle, tt->start_tok, tt->end_tok, tokens, ntok,
			 "p_text", &text_tok) == 0) {
    
    /*
     * OK - set text
     */
    
    text = tokens[text_tok].tok;
    tt->comment_text = tdrpStrDup(text);

  } else {

    tt->comment_text = tdrpStrDup("");

  }

  return (0);

}

/***************
 * set_private()
 *
 * set whether or not param is private
 */

int set_private(token_handle_t *tok_handle,
		TDRPtable *tt,
		tdrpToken_t *tokens, int ntok)

{

  int is_private;
  int private_tok;
  char *private;
  
  if (tdrpFindSingleItem(tok_handle, tt->start_tok, tt->end_tok, tokens, ntok,
			 "p_private", &private_tok) == 0) {
    
    /*
     * OK - set private
     */
    
    private = tokens[private_tok].tok;
    
    if (tdrpBoolStrTrue(private, &is_private)) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "In p_private boolean value\n");
      fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[private_tok]));
      return (-1);
    }
    
    if (is_private == TRUE) {
      tt->is_private = TRUE;
    }
    
  }

  return (0);

}

/*******************
 * set_min_and_max()
 *
 * Set min and max values, check limits
 */

static int set_min_and_max(token_handle_t *tok_handle,
			   TDRPtable *tt,
			   tdrpToken_t *tokens, int ntok)
     
{

  int min_tok, max_tok;
  int iret = 0;

  /*
   * load up max and min
   */
  
  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    "p_min", &min_tok) == 0) {
    if (tdrpReadMin(tok_handle, tt, tokens, ntok, min_tok, pFALSE)) {
      iret = -1;
    }
    tt->has_min = TRUE;
  } else {
    switch (tt->ptype) {
    case INT_TYPE:
      tt->min_val.i = INT_MIN;
      break;
    case LONG_TYPE:
      tt->min_val.l = LONG_MIN;
      break;
    case FLOAT_TYPE:
      tt->min_val.f = FLT_MIN;
      break;
    case DOUBLE_TYPE:
      tt->min_val.d = DBL_MIN;
      break;
    default:
      break;
    }
  }

  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    "p_max", &max_tok) == 0) {
    if (tdrpReadMax(tok_handle, tt, tokens, ntok, max_tok, pFALSE)) {
      iret = -1;
    }
    tt->has_max = TRUE;
  } else {
    switch (tt->ptype) {
    case INT_TYPE:
      tt->max_val.i = INT_MAX;
      break;
    case LONG_TYPE:
      tt->max_val.l = LONG_MAX;
      break;
    case FLOAT_TYPE:
      tt->max_val.f = FLT_MAX;
      break;
    case DOUBLE_TYPE:
      tt->max_val.d = DBL_MAX;
      break;
    default:
      break;
    }
  }

  return (iret);

}

/************************
 * check_no_min_and_max()
 *
 * Check that min and max are not set
 */

static int check_no_min_and_max(token_handle_t *tok_handle,
				TDRPtable *tt,
				tdrpToken_t *tokens, int ntok)
     
{

  int label_tok;

  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    "p_min", &label_tok) == 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
    fprintf(stderr, "%s cannot have min value.\n",
	    tdrpTableEntry2Str(tt->ptype));
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[label_tok]));
    return (-1);
  }

  if (tdrpFindLabel(tt->start_tok, tt->end_tok, tokens, ntok,
		    "p_max", &label_tok) == 0) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<, param '%s'\n", tt->param_name);
    fprintf(stderr, "%s cannot have max value.\n",
	    tdrpTableEntry2Str(tt->ptype));
    fprintf(stderr, "%s\n", tdrpLineInfo(tok_handle, &tokens[label_tok]));
    return (-1);
  }

  return (0);

}

/******************
 * initialize_val()
 *
 * Initialize the value - single or array - prior to reading defaults
 */

static void initialize_val(TDRPtable *tt, tdrpVal_t *init_val)

{

  int i;
  int nvals;

  if (tt->is_array2D) {
    nvals = tt->array_n1 * tt->array_n2;
    tt->array_vals =
      (tdrpVal_t *) tdrpMalloc(nvals * sizeof(tdrpVal_t));
    for (i = 0; i < nvals; i++) {
      if (tt->ptype == STRING_TYPE) {
	tt->array_vals[i].s = tdrpStrDup(init_val->s);
      } else {
	tt->array_vals[i] = *init_val;
      }
    }
  } else if (tt->is_array) {
    tt->array_vals =
      (tdrpVal_t *) tdrpMalloc(tt->array_n * sizeof(tdrpVal_t));
    for (i = 0; i < tt->array_n; i++) {
      if (tt->ptype == STRING_TYPE) {
	tt->array_vals[i].s = tdrpStrDup(init_val->s);
      } else {
	tt->array_vals[i] = *init_val;
      }
    }
  } else {
    if (tt->ptype == STRING_TYPE) {
      tt->single_val.s = tdrpStrDup(init_val->s);
    } else {
      tt->single_val = *init_val;
    }
  }

}
