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
/*****************************************************
 * struct_list.c
 *
 * Struct list manipulation routines for tdrp_gen.
 *
 * The struct list is a list of struct_field_t structs.
 *
 */

#include "tdrp_gen.h"
#include <assert.h>

static tdrpBuf *ListBuf = NULL;
static int Ndefs = 0;
static struct_def_t *Defs;

/******************
 * struct_list_init()
 *
 * Initialize the struct list.
 */

void struct_list_init(void)

{
  ListBuf = tdrpBufCreate();
}

/******************
 *  struct_def_add()
 *
 *  Adds definition to struct list.
 *
 *  Returns:
 *    0  on success
 *    -1 on failure which occurs if definition is already on the list.
 */

int struct_def_add(char *name, int nfields, struct_field_t *fields)

{

  int ifield;
  struct_def_t def;
  assert(ListBuf != NULL);
  
  /*
   * check if def is already registered
   */
  
  if (struct_def_by_name(name) != NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n struct '%s' already defined.\n",
	    name);
    return (-1);
  }

  if (enum_def_by_name(name) != NULL) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n proposed struct '%s' conflicts with enum.\n",
	    name);
    return (-1);
  }

  /*
   * add
   */

  ctype_def_add(name);
  def.name = tdrpStrDup(name);
  def.nfields = nfields;
  def.fields =
    (struct_field_t *) tdrpMalloc(nfields * sizeof(struct_field_t));

  for (ifield = 0; ifield < nfields; ifield++) {
    def.fields[ifield] = fields[ifield];
    def.fields[ifield].ftype = tdrpStrDup(fields[ifield].ftype);
    def.fields[ifield].fname = tdrpStrDup(fields[ifield].fname);
  }
  
  /*
   * set types for fields
   */
  
  for (ifield = 0; ifield < nfields; ifield++) {

    enum_def_t *edef;
    int ctype, ptype;
    
    if ((edef = enum_def_by_name(def.fields[ifield].ftype)) != NULL) {
      
      enum_def_t *s_edef;
      int j;

      /*
       * enum type is a special case - load up enum def for this field
       */
      
      ptype = ENUM_TYPE;
      s_edef = &def.fields[ifield].enum_def;
      s_edef->name = tdrpStrDup(edef->name);
      s_edef->nfields = edef->nfields;
      s_edef->fields = (enum_field_t *)
	tdrpMalloc (edef->nfields * sizeof(enum_field_t));
      for (j = 0; j < edef->nfields; j++) {
	s_edef->fields[j].name = tdrpStrDup(edef->fields[j].name);
	s_edef->fields[j].val = edef->fields[j].val;
      }
      
    } else {
      
      ptype = tdrpStr2TableEntry(fields[ifield].ftype);
      
    }
    
    ctype = ctype_index_by_iname(fields[ifield].ftype);

    if (ctype < 0 || ptype < 0 || ptype == STRUCT_TYPE) {
      fprintf(stderr, "\n>>> TDRP_ERROR <<<\n");
      fprintf(stderr, "  Illegal type in struct '%s'\n", name);
      fprintf(stderr, "  Illegal type is '%s' for field '%s'\n",
	      fields[ifield].ftype,
	      fields[ifield].fname);
      return (-1);
    }
    def.fields[ifield].ptype = ptype;
    def.fields[ifield].ctype = ctype;
    
  } /* ifield */
  
  Defs = tdrpBufAdd(ListBuf, &def, sizeof(struct_def_t));
  Ndefs = tdrpBufLen(ListBuf) / sizeof(struct_def_t);

  return (0);

}

/*******************************
 * n_struct_defs()
 *
 * return number of defs on list
 */

int n_struct_defs(void)

{
  assert(ListBuf != NULL);
  return (Ndefs);
}

/*******************************************
 * struct_def_by_index()
 *
 * Returns the struct def for the given index.  
 * Returns NULL on error.
 */

struct_def_t *struct_def_by_index(int index)

{

  assert(ListBuf != NULL);

  if (index >= Ndefs) {
    return (NULL);
  }

  return (Defs + index);

}

/*******************************************
 * struct_def_by_name()
 *
 * Returns the struct def for the given name.
 * Returns NULL on error.
 */

struct_def_t *struct_def_by_name(char *name)

{

  int i;
  assert(ListBuf != NULL);
  
  for (i = 0; i < Ndefs; i++) {
    if (!strcmp(Defs[i].name, name)) {
      return (Defs + i);
    }
  }

  return (NULL);

}

/******************
 * struct_list_free()
 *
 * Free the struct list.
 */

void struct_list_free(void)

{

  int i, j, k;
  enum_def_t *edef;
  struct_def_t *sdef;
  assert(ListBuf != NULL);

  sdef = Defs;
  for (j = 0; j < Ndefs; j++, sdef++) {
    for (i = 0; i < sdef->nfields; i++) {
      if (sdef->fields[i].ptype == ENUM_TYPE) {
	edef = &sdef->fields[i].enum_def;
	for (k = 0; k < edef->nfields; k++) {
	  tdrpFree(edef->fields[k].name);
	}
	tdrpFree(edef->fields);
	tdrpFree(edef->name);
      } /* if ((edef = ... */
      tdrpFree(sdef->fields[i].ftype);
      tdrpFree(sdef->fields[i].fname);
    }
    tdrpFree(sdef->name);
    tdrpFree(sdef->fields);
  }

  tdrpBufDelete(ListBuf);

}



