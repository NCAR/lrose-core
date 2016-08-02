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
 * enum_list.c
 *
 * Enum list manipulation routines for tdrp_gen.
 *
 * The enum list is a list of enum_field_t structs.
 *
 */

#include "tdrp_gen.h"
#include <assert.h>

static tdrpBuf *ListBuf = NULL;
static int Ndefs = 0;
static enum_def_t *Defs;

/******************
 * enum_list_init()
 *
 * Initialize the enum list.
 */

void enum_list_init(void)

{
  ListBuf = tdrpBufCreate();
}

/******************
 *  enum_def_add()
 *
 *  Adds definition to enum list.
 *
 *  Returns:
 *    0  on success
 *    -1 on failure which occurs if definition is already on the list.
 */

int enum_def_add(char *name, int nfields, enum_field_t *fields)

{

  int i;
  enum_def_t def;
  assert(ListBuf != NULL);
  
  /*
   * check if def is already registered
   */
  
  if (enum_def_by_name(name) != NULL) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<<\n enum '%s' already defined.\n",
	    name);
    return (-1);
  }

  if (struct_def_by_name(name) != NULL) {
    fprintf(stderr,
	    "\n>>> TDRP_ERROR <<<\n proposed enum '%s' conflicts with struct.\n",
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
    (enum_field_t *) tdrpMalloc(nfields * sizeof(enum_field_t));

  for (i = 0; i < nfields; i++) {
    def.fields[i].name = tdrpStrDup(fields[i].name);
    def.fields[i].val = fields[i].val;
  }
  
  Defs = tdrpBufAdd(ListBuf, &def, sizeof(enum_def_t));
  Ndefs = tdrpBufLen(ListBuf) / sizeof(enum_def_t);

  return (0);

}

/*******************************
 * n_enum_defs()
 *
 * return number of defs on list
 */

int n_enum_defs(void)

{
  assert(ListBuf != NULL);
  return (Ndefs);
}

/*******************************************
 * enum_def_by_index()
 *
 * Returns the enum def for the given index.  
 * Returns NULL on error.
 */

enum_def_t *enum_def_by_index(int index)

{

  assert(ListBuf != NULL);

  if (index >= Ndefs) {
    return (NULL);
  }

  return (Defs + index);

}

/*******************************************
 * enum_def_by_name()
 *
 * Returns the enum def for the given name.
 * Returns NULL on error.
 */

enum_def_t *enum_def_by_name(char *name)

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
 * enum_list_free()
 *
 * Free the enum list.
 */

void enum_list_free(void)

{

  int i, j;
  enum_def_t *def;
  assert(ListBuf != NULL);

  def = Defs;
  for (j = 0; j < Ndefs; j++, def++) {
    for (i = 0; i < def->nfields; i++) {
      tdrpFree(def->fields[i].name);
    }
    tdrpFree(def->name);
    tdrpFree(def->fields);
  }

  tdrpBufDelete(ListBuf);

}

