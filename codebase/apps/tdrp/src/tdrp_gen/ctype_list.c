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
 * ctype_list.c
 *
 * Ctype list manipulation routines for tdrp_gen.
 *
 * The ctype list is a list of ctype_t structs.
 *
 */

#include "tdrp_gen.h"
#include <assert.h>

static tdrpBuf *ListBuf = NULL;
static int Ndefs = 0;
static ctype_def_t *Defs;
static char *Module;
static void local_add(char *iname, char *cname);

/******************
 * ctype_list_init()
 *
 * Initialize the ctype list.
 */

void ctype_list_init(char *module)

{

  Module = tdrpStrDup(module);
  ListBuf = tdrpBufCreate();

  /*
   * add the names of the known types
   */
  
  local_add("boolean", "tdrp_bool_t");
  local_add("char*", "char*");
  local_add("string", "char*");
  local_add("int", "int");
  local_add("long", "long");
  local_add("float", "float");
  local_add("double", "double");
  
}

/******************
 *  local_add()
 *
 *  Adds definition from local call
 *
 */

static void local_add(char *iname, char *cname)
     
{
  
  ctype_def_t def;
  
  def.iname = tdrpStrDup(iname);
  def.cname = tdrpStrDup(cname);
  Defs = tdrpBufAdd(ListBuf, &def, sizeof(ctype_def_t));
  Ndefs = tdrpBufLen(ListBuf) / sizeof(ctype_def_t);

}

/******************
 *  ctype_def_add()
 *
 *  Adds definition to ctype list.
 *
 *  Returns:
 *    0  on success
 *    -1 on failure which occurs if definition is already on the list.
 */

int ctype_def_add(char *iname)

{

  ctype_def_t def;
  assert(ListBuf != NULL);
  
  /*
   * check if def is on list
   */
  
  if (ctype_index_by_iname(iname) != -1) {
    return (-1);
  }

  /*
   * set the names - the C name is a concatenation of the
   * module and input name
   */
  
  def.iname = tdrpStrDup(iname);
  def.cname = (char *) tdrpMalloc (strlen(Module) + strlen(iname) + 2);
  sprintf(def.cname, "%s_%s", Module, iname);

  /*
   * add
   */

  Defs = tdrpBufAdd(ListBuf, &def, sizeof(ctype_def_t));
  Ndefs = tdrpBufLen(ListBuf) / sizeof(ctype_def_t);

  return (0);

}

/*******************************
 * n_ctype_defs()
 *
 * return number of defs on list
 */

int n_ctype_defs(void)

{
  assert(ListBuf != NULL);
  return (Ndefs);
}

/*******************************************
 * ctype_def_by_index()
 *
 * Returns the ctype def for the given index.  
 * Returns NULL on error.
 */

ctype_def_t *ctype_def_by_index(int index)

{

  assert(ListBuf != NULL);

  if (index >= Ndefs) {
    return (NULL);
  }

  return (Defs + index);

}

/*******************************************
 * ctype_iname_by_index()
 *
 * Returns the ctype input name for the given index.  
 * Returns NULL on error.
 */

char *ctype_iname_by_index(int index)

{

  assert(ListBuf != NULL);

  if (index < 0 || index >= Ndefs) {
    return ("unknown");
  }

  return (Defs[index].iname);

}

/*******************************************
 * ctype_cname_by_index()
 *
 * Returns the ctype C name for the given index.  
 * Returns NULL on error.
 */

char *ctype_cname_by_index(int index)

{

  assert(ListBuf != NULL);

  if (index < 0 || index >= Ndefs) {
    return ("unknown");
  }

  return (Defs[index].cname);

}

/*******************************************
 * ctype_cname_by_iname()
 *
 * Returns the ctype C name for the given input name.
 * Returns "unknown" on error.
 */

char *ctype_cname_by_iname(char *iname)

{

  int index;
  assert(ListBuf != NULL);

  index = ctype_index_by_iname(iname);
  if (index < 0) {
    return ("unknown");
  }

  return (Defs[index].cname);

}

/*******************************************
 * ctype_index_by_iname()
 *
 * Returns the ctype index for the given input name.
 * Returns -1 on error.
 */

int ctype_index_by_iname(char *iname)

{

  int i;
  assert(ListBuf != NULL);
  
  for (i = 0; i < Ndefs; i++) {
    if (!strcmp(Defs[i].iname, iname)) {
      return (i);
    }
  }

  return (-1);

}

/*******************************************
 * ctype_index_by_cname()
 *
 * Returns the ctype index for the given C name.
 * Returns -1 on error.
 */

int ctype_index_by_cname(char *cname)

{

  int i;
  assert(ListBuf != NULL);
  
  for (i = 0; i < Ndefs; i++) {
    if (!strcmp(Defs[i].cname, cname)) {
      return (i);
    }
  }

  return (-1);

}

/******************
 * ctype_list_free()
 *
 * Free the ctype list.
 */

void ctype_list_free(void)

{

  int j;
  ctype_def_t *def;
  assert(ListBuf != NULL);

  tdrpFree(Module);

  def = Defs;
  for (j = 0; j < Ndefs; j++, def++) {
    tdrpFree(def->iname);
    tdrpFree(def->cname);
  }

  tdrpBufDelete(ListBuf);

}

/******************
 * ptype2ctypeStr()
 *
 * Convert an entry type to its corresponding ctype string.
 */

char *ptype2ctypeStr(TDRPtable *tt)

{

  switch (tt->ptype) {

  case BOOL_TYPE:
    return ("int");
    break;
  case INT_TYPE:
    return("int");
    break;
  case LONG_TYPE:
    return("long");
    break;
  case FLOAT_TYPE:
    return("float");
    break;
  case DOUBLE_TYPE:
    return("double");
    break;
  case STRING_TYPE:
    return("char*");
    break;
  case ENUM_TYPE:
    return(tt->enum_def.name);
    break;
  case STRUCT_TYPE:
    return(tt->struct_def.name);
    break;
  case COMMENT_TYPE:
    return ("");
    break;

  } /* switch */

  return ("unknown");

}

