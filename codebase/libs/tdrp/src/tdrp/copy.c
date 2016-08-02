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
 * copy.c
 *
 * TDRP copy functions.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80303
 *
 * December 1998
 *
 */

#include <tdrp/tdrp.h>
#include <string.h>
#include <stdlib.h>

/***********************************************************
 * tdrpCopyTable()
 *
 * Copy one table to another.
 * The target table must be at least as large as the source
 * table.
 */

void tdrpCopyTable(const TDRPtable *source, TDRPtable *target)

{

  int i, j;
  int field_num;
  const TDRPtable *ss = source;
  TDRPtable *tt = target;
  const enum_def_t *ss_edef;
  const struct_def_t *ss_sdef;
  enum_def_t *tt_edef;
  struct_def_t *tt_sdef;

  while (ss->param_name != NULL) {

    /*
     * shallow copy first
     */

    *tt = *ss;

    /*
     * strings
     */

    if (ss->param_name) {
      tt->param_name = tdrpStrDup(ss->param_name);
    }
    if (ss->comment_hdr) {
      tt->comment_hdr = tdrpStrDup(ss->comment_hdr);
    }
    if (ss->comment_text) {
      tt->comment_text = tdrpStrDup(ss->comment_text);
    }
    if (ss->descr) {
      tt->descr = tdrpStrDup(ss->descr);
    }
    if (ss->help) {
      tt->help = tdrpStrDup(ss->help);
    }

    /*
     * enum definition
     */

    ss_edef = &ss->enum_def;
    tt_edef = &tt->enum_def;

    if (ss->ptype == ENUM_TYPE) {
      tt_edef->name = tdrpStrDup(ss_edef->name);
      tt_edef->fields = (enum_field_t *)
	tdrpMalloc(ss_edef->nfields * sizeof(enum_field_t));
      for (i = 0; i < ss_edef->nfields; i++) {
	tt_edef->fields[i] = ss_edef->fields[i];
	tt_edef->fields[i].name =
	  tdrpStrDup(ss_edef->fields[i].name);
      }
    }

    /*
     * struct definition
     */

    ss_sdef = &ss->struct_def;
    tt_sdef = &tt->struct_def;

    if (ss->ptype == STRUCT_TYPE) {

      tt_sdef->name = tdrpStrDup(ss_sdef->name);
      tt_sdef->fields = (struct_field_t *)
	tdrpMalloc(ss_sdef->nfields * sizeof(struct_field_t));

      for (i = 0; i < ss_sdef->nfields; i++) {

	tt_sdef->fields[i] = ss_sdef->fields[i];
	tt_sdef->fields[i].ftype =
	  tdrpStrDup(ss_sdef->fields[i].ftype);
	tt_sdef->fields[i].fname =
	  tdrpStrDup(ss_sdef->fields[i].fname);

	if (ss_sdef->fields[i].ptype == ENUM_TYPE) {
	  ss_edef = &ss_sdef->fields[i].enum_def;
	  tt_edef = &tt_sdef->fields[i].enum_def;
	  tt_edef->name = tdrpStrDup(ss_edef->name);
	  tt_edef->fields = (enum_field_t *)
	    tdrpMalloc(ss_edef->nfields * sizeof(enum_field_t));
	  for (j = 0; j < ss_edef->nfields; j++) {
	    tt_edef->fields[j] = ss_edef->fields[j];
	    tt_edef->fields[j].name =
	      tdrpStrDup(ss_edef->fields[j].name);
	  } /* j */
	}

      } /* i */

    }

    /*
     * non-struct arrays
     */
    
    if (ss->ptype != STRUCT_TYPE && ss->is_array) {
      tt->array_vals =
	(tdrpVal_t *) tdrpCalloc(ss->array_n, sizeof(tdrpVal_t));
      if (ss->ptype == STRING_TYPE) {
	for (i = 0; i < ss->array_n; i++) {
	  tt->array_vals[i].s = tdrpStrDup(ss->array_vals[i].s);
	}
      } else {
	memcpy(tt->array_vals, ss->array_vals,
	       ss->array_n * sizeof(tdrpVal_t));
      }
    }

    /*
     * single string val
     */

    if (ss->ptype == STRING_TYPE && !ss->is_array) {
      tt->single_val.s = tdrpStrDup(ss->single_val.s);
    }
    
    /*
     * structs
     */
    
    if (ss->ptype == STRUCT_TYPE) {
      tt->struct_vals =	(tdrpVal_t *)
	tdrpCalloc(ss->n_struct_vals, sizeof(tdrpVal_t));
      memcpy(tt->struct_vals, ss->struct_vals,
	     ss->n_struct_vals * sizeof(tdrpVal_t));
      for (i = 0; i < ss->n_struct_vals; i++) {
	field_num = i % ss->struct_def.nfields;
	if (ss->struct_def.fields[field_num].ptype == STRING_TYPE) {
	  tt->struct_vals[i].s = tdrpStrDup(ss->struct_vals[i].s);
	}
      }
    }
    ss++;
    tt++;
  
  } /* while */

  /*
   * copy over the last (null) entry
   */
  
  *tt = *ss;
      
}
