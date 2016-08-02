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
 * convert.c
 *
 * Convert and test data types.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Jan 1998
 *
 ****************************************************************/

#include <tdrp/tdrp.h>
#include <string.h>
#include <ctype.h>

#ifdef SUNOS4
#define	toupper(C) (((C) >= 'a' && (C) <= 'z')? (C) - 'a' + 'A': (C))
#endif

char *tdrpTableEntry2Str(int ptype)

{
  switch (ptype) {
    
  case BOOL_TYPE:
    return ("BOOL_TYPE");
    break;
  case INT_TYPE:
    return ("INT_TYPE");
    break;
  case LONG_TYPE:
    return ("LONG_TYPE");
    break;
  case FLOAT_TYPE:
    return ("FLOAT_TYPE");
    break;
  case DOUBLE_TYPE:
    return ("DOUBLE_TYPE");
    break;
  case ENUM_TYPE:
    return ("ENUM_TYPE");
    break;
  case STRING_TYPE:
    return ("STRING_TYPE");
    break;
  case STRUCT_TYPE:
    return ("STRUCT_TYPE");
    break;
  case COMMENT_TYPE:
    return ("COMMENT_TYPE");
    break;
  default:
    return ("UNKNOWN");

  }

}


/*********************
 * tdrpStr2TableEntry()
 *
 * Decode type from string.
 *
 * Returns type if successful, -1 on error
 */

int tdrpStr2TableEntry(const char *type_str)

{

  if (!strcmp(type_str, "comment")) {
    return (COMMENT_TYPE);
  } else if (!strcmp(type_str, "boolean")) {
    return (BOOL_TYPE);
  } else if (!strcmp(type_str, "int")) {
    return (INT_TYPE);
  } else if (!strcmp(type_str, "long")) {
    return (LONG_TYPE);
  } else if (!strcmp(type_str, "float")) {
    return (FLOAT_TYPE);
  } else if (!strcmp(type_str, "double")) {
    return (DOUBLE_TYPE);
  } else if (!strcmp(type_str, "enum")) {
    return (ENUM_TYPE);
  } else if (!strcmp(type_str, "char*")) {
    return (STRING_TYPE);
  } else if (!strcmp(type_str, "string")) {
    return (STRING_TYPE);
  } else if (!strcmp(type_str, "struct")) {
    return (STRUCT_TYPE);
  } else {
    return (-1);
  }

}

/*******************************************
 * tdrpFieldType2Str
 *
 * return ascii string representation of field
 * variable type
 */

char *tdrpFieldType2Str(int field_type)

{

  switch (field_type) {
    case BOOL_TYPE:
      return ("tdrp_bool_t");
      break;
    case INT_TYPE:
      return ("int");
      break;
    case LONG_TYPE:
      return ("long");
      break;
    case FLOAT_TYPE:
      return ("float");
      break;
    case DOUBLE_TYPE:
      return ("double");
      break;
    case ENUM_TYPE:
      return ("enum");
      break;
    case STRING_TYPE:
      return ("char*");
      break;
    case STRUCT_TYPE:
      return ("struct");
      break;
    case COMMENT_TYPE:
      return ("comment");
      break;
  } /* switch */

  return ("unknown");

}

/*******************************************
 * tdrpEntryType2Str
 *
 * fill out_str with ascii representation of param type
 * given the entry
 */

void tdrpEntryType2Str(const char *module, const TDRPtable *tt, char *out_str)

{

  if (tt->ptype == STRUCT_TYPE) {
    sprintf(out_str, "%s_%s", module, tt->struct_def.name);
  } else if (tt->ptype == ENUM_TYPE) {
    sprintf(out_str, "%s_%s", module, tt->enum_def.name);
  } else {
    sprintf(out_str, "%s", tdrpFieldType2Str(tt->ptype));
  }

}
      
/*******************************************
 * tdrpType2Str
 *
 * return ascii representation of param type
 * given the entry type
 */

char *tdrpType2Str(const TDRPtable *tt)

{
  if (tt->ptype == STRUCT_TYPE) {
    return (tt->struct_def.name);
  } else if (tt->ptype == ENUM_TYPE) {
    return (tt->enum_def.name);
  } else {
    return (tdrpFieldType2Str(tt->ptype));
  }
}
      
/***************
 * tdrpLoadVal()
 *
 * Load value from string, given type
 *
 * Returns type if successful, -1 on error
 */

int tdrpLoadVal(const char *val_str, int type, tdrpVal_t *val)

{

  int ii;
  long ll;
  double dd;
  
  switch (type) {
    
  case STRING_TYPE:
    val->s = tdrpStrDup(val_str);
    return (0);
    break;

  case INT_TYPE:
    if (sscanf(val_str, "%d", &ii) != 1) {
      return (-1);
    }
    val->i = ii;
    return (0);
    break;

  case LONG_TYPE:
    if (sscanf(val_str, "%ld", &ll) != 1) {
      return (-1);
    }
    val->l = ll;
    return (0);
    break;

  case FLOAT_TYPE:
    if (sscanf(val_str, "%lg", &dd) != 1) {
      return (-1);
    }
    val->f = (float) dd;
    return (0);
    break;

  case DOUBLE_TYPE:
    if (sscanf(val_str, "%lg", &dd) != 1) {
      return (-1);
    }
    val->d = dd;
    return (0);
    break;

  default:
    return (-1);
    break;

  } /* switch */
  
  return (0);

}

/*****************
 * tdrpBoolStrTrue()
 *
 * Checks if bool string is set to TRUE.
 * Converts all characters to upper case before test, therefore
 * test in not case sensitive.
 *
 * Sets is_true to TRUE or FALSE.
 *
 * Returns 0 on success, -1 on error.
 */

int tdrpBoolStrTrue(const char *bool_str, int *is_true_p)
     
{

  unsigned int i;
  char local_str[32];

  /*
   * make local copy
   */
  
  strncpy(local_str, bool_str, 32);
  local_str[31] = '\0';

  for (i = 0; i < strlen(local_str); i++) {
    local_str[i] = toupper(local_str[i]);
  }

  if (!strcmp(local_str, "TRUE")) {
    *is_true_p = TRUE;
    return (0);
  } else if (!strcmp(local_str, "FALSE")) {
    *is_true_p = FALSE;
    return (0);
  } else {
    return (-1);
  }

}

/********************************
 * tdrpBool2Str()
 *
 * Return string for boolean
 */

char *tdrpBool2Str(int b)

{

  if (b) {
    return ("pTRUE");
  } else {
    return ("pFALSE");
  }

}

/********************************
 * tdrpEnum2Str()
 *
 * Return string for enum field
 */

char *tdrpEnum2Str(int val, const TDRPtable *tt)

{

  int i;

  for (i = 0; i < tt->enum_def.nfields; i++) {
    if (val == tt->enum_def.fields[i].val) {
      return (tt->enum_def.fields[i].name);
    }
  }
  return ("unknown");

}



