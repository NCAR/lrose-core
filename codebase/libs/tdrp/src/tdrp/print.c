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

/**********************************************************
 * print.c
 *
 * TDRP print functions.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307, USA
 *
 * May 1997
 */

#include <tdrp/tdrp.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>
#include <float.h>
#define MIN_KEYWORD "MIN"
#define MAX_KEYWORD "MAX"

/*
 * file scope prototypes
 */

static void print_comment_lines(FILE *out, const char *str, int format_len);
static void print_comment(FILE *out, const char *line, int format_len);
static void print_entry(FILE *out, const TDRPtable *tt, tdrp_print_mode_t mode);
static void print_type(FILE *out, const TDRPtable *tt);
     
/*************
 * tdrpPrint()
 * 
 * Print params file
 *
 * The modes supported are:
 *
 *   PRINT_SHORT:   main comments only, no help or descriptions
 *                  structs and arrays on a single line
 *   PRINT_NORM:    short + descriptions and help
 *   PRINT_LONG:    norm  + arrays and structs expanded
 *   PRINT_VERBOSE: long  + private params included
 */

void tdrpPrint(FILE *out, const TDRPtable *table, const char *module,
	       tdrp_print_mode_t mode)
     
{

  const TDRPtable *tt = table;

  fprintf(out, "/***********************************"
	  "***********************************\n");
  fprintf(out, " * TDRP params for %s\n", module);
  fprintf(out, " ***********************************"
	 "***********************************/\n");
  fprintf(out, "\n");

  while (tt->param_name != NULL) {
    print_entry(out, tt, mode);
    tt++;
  }

  return;
  
}

/******************
 * print_entry()
 * 
 * Print params file entry
 */

static void print_entry(FILE *out, const TDRPtable *tt, tdrp_print_mode_t mode)

{

  int i, j, index;
  char *val_str;

  if (tt->ptype == COMMENT_TYPE) {
    if (mode >= PRINT_NORM || strlen(tt->comment_hdr) > 0) {
      fprintf(out, "//=================================="
	      "====================================\n");
      fprintf(out, "//\n");
      if (strlen(tt->comment_hdr) > 0) {
	print_comment_lines(out, tt->comment_hdr, 70);
	fprintf(out, "//\n");
      }
      if (mode >= PRINT_NORM && strlen(tt->comment_text) > 0) {
	print_comment_lines(out, tt->comment_text, 70);
	fprintf(out, "//\n");
      }
      fprintf(out, "//=================================="
	      "====================================\n");
      fprintf(out, " \n");
    }
    return;
  }

  if (tt->is_private && mode != PRINT_VERBOSE) {
    return;
  }
  
  /*
   * help, description and type
   */

  if (mode >= PRINT_NORM) {
    
    int nameLen = strlen(tt->param_name);
    fprintf(out, "///////////// %s ",
	    tt->param_name);
    if (nameLen < 40) {
      for (i = 0; i < (40 - nameLen); i++) {
	fprintf(out, "/");
      }
    }
    fprintf(out, "\n");
    fprintf(out, "//\n");

    if (strlen(tt->descr) > 0) {
      print_comment_lines(out, tt->descr, 70);
      if (mode >= PRINT_LONG) {
        fprintf(out, "//\n");
      }
    }

    if (strlen(tt->help) > 0) {
      print_comment_lines(out, tt->help, 70);
      if (mode >= PRINT_LONG) {
        fprintf(out, "//\n");
      }
    }

    if (tt->has_min) {
      val_str = sprintf_val(tt->ptype, &tt->enum_def, &tt->min_val);
      fprintf(out, "// Minimum val: %s\n", val_str);
      tdrpFree(val_str);
    }

    if (tt->has_max) {
      val_str = sprintf_val(tt->ptype, &tt->enum_def, &tt->max_val);
      fprintf(out, "// Maximum val: %s\n", val_str);
      tdrpFree(val_str);
    }

    print_type(out, tt);
    if (tt->is_private) {
      fprintf(out, "// Private - cannot be set in param file.\n");
    }

    if (tt->is_array2D) {
      if (tt->array_len_fixed) {
	fprintf(out, "// 2D array - fixed size (%d x %d).\n",
		tt->array_n1, tt->array_n2);
      } else {
	fprintf(out, "// 2D array - variable size\n");
      }
    } else if (tt->is_array) {
      if (tt->array_len_fixed) {
	fprintf(out, "// 1D array - fixed length - %d elements.\n",
		tt->array_n);
      } else {
	fprintf(out, "// 1D array - variable length.\n");
      }
    }
    fprintf(out, "//\n");
    fprintf(out, "\n");
    
  }

  /*
   * values
   */

  if (tt->ptype == STRUCT_TYPE) {

    if (tt->is_array) {

      if (mode >= PRINT_LONG) {
	
	fprintf(out, "%s = {\n", tt->param_name);
	for (j = 0; j < tt->array_n; j++) {
	  fprintf(out, "  {\n");
	  for (i = 0; i < tt->struct_def.nfields; i++) {
	    index = j * tt->struct_def.nfields + i;
	    val_str = sprintf_val(tt->struct_def.fields[i].ptype,
				  &tt->struct_def.fields[i].enum_def,
				  tt->struct_vals + index);
	    fprintf(out, "    %s = %s",
		    tt->struct_def.fields[i].fname, val_str);
	    tdrpFree(val_str);
	    if (i < tt->struct_def.nfields - 1) {
	      fprintf(out, ",\n");
	    } else {
	      fprintf(out, "\n  }\n");
	    }
	  } /* i */
	  if (j < tt->array_n - 1) {
	    fprintf(out, "  ,\n");
	  }
	} /* j */
	fprintf(out, "};\n");

      } else {

	fprintf(out, "%s = {\n", tt->param_name);
	for (j = 0; j < tt->array_n; j++) {
	  fprintf(out, "  {");
	  for (i = 0; i < tt->struct_def.nfields; i++) {
	    index = j * tt->struct_def.nfields + i;
	    val_str = sprintf_val(tt->struct_def.fields[i].ptype,
				  &tt->struct_def.fields[i].enum_def,
				  tt->struct_vals + index);
	    fprintf(out, " %s", val_str);
	    tdrpFree(val_str);
	    if (i < tt->struct_def.nfields - 1) {
	      fprintf(out, ",");
	    } else {
	      fprintf(out, "}");
	    }
	  } /* i */
	  if (j < tt->array_n - 1) {
	    fprintf(out, ",\n");
	  } else {
	    fprintf(out, "\n");
	  }
	} /* j */
	fprintf(out, "};\n");

      }
      
    } else {

      if (mode >= PRINT_LONG) {
	
	fprintf(out, "%s = {\n", tt->param_name);
	for (i = 0; i < tt->struct_def.nfields; i++) {
	  val_str = sprintf_val(tt->struct_def.fields[i].ptype,
				&tt->struct_def.fields[i].enum_def,
				tt->struct_vals + i);
	  fprintf(out, "    %s = %s",
		  tt->struct_def.fields[i].fname, val_str);
	  tdrpFree(val_str);
	  if (i < tt->struct_def.nfields - 1) {
	    fprintf(out, ",\n");
	  } else {
	    fprintf(out, "\n");
	  }
	} /* i */
	fprintf(out, "};\n");

      } else {

	fprintf(out, "%s = {", tt->param_name);
	for (i = 0; i < tt->struct_def.nfields; i++) {
	  val_str = sprintf_val(tt->struct_def.fields[i].ptype,
				&tt->struct_def.fields[i].enum_def,
				tt->struct_vals + i);
	  fprintf(out, " %s", val_str);
	  if (i < tt->struct_def.nfields - 1) {
	    fprintf(out, ",");
	  }
	  tdrpFree(val_str);
	} /* i */
	fprintf(out, " };\n");

      }
      
    }

  } else {

    if (tt->is_array2D) {

      fprintf(out, "%s = {\n", tt->param_name);
      for (j = 0; j < tt->array_n1; j++) {
	fprintf(out, "  {");
	for (i = 0; i < tt->array_n2; i++) {
	  index = j * tt->array_n2 + i;
	  val_str = sprintf_val(tt->ptype,
				&tt->enum_def,
				tt->array_vals + index);
	  fprintf(out, " %s", val_str);
	  tdrpFree(val_str);
	  if (i < tt->array_n2 - 1) {
	    fprintf(out, ",");
	  }
	} /* i */
	fprintf(out, " }");
	if (j < tt->array_n1 - 1) {
	  fprintf(out, ",\n");
	} else {
	  fprintf(out, "\n");
	}
      } /* j */
      fprintf(out, "};\n");
      
    } else if (tt->is_array) {

      fprintf(out, "%s = {", tt->param_name);
      if (mode >= PRINT_LONG) {
	fprintf(out, "\n");
      }
      for (i = 0; i < tt->array_n; i++) {
	val_str = sprintf_val(tt->ptype,
			      &tt->enum_def, tt->array_vals + i);
	fprintf(out, " %s", val_str);
	tdrpFree(val_str);
	if (i < tt->array_n - 1) {
	  fprintf(out, ",");
	}
	if (mode >= PRINT_LONG) {
	  fprintf(out, "\n");
	}
      } /* i */
      if (mode >= PRINT_LONG) {
	fprintf(out, "};\n");
      } else {
	fprintf(out, " };\n");
      }
      
    } else {

      val_str = sprintf_val(tt->ptype, &tt->enum_def, &tt->single_val);
      fprintf(out, "%s = %s;\n", tt->param_name, val_str);
      tdrpFree(val_str);

    }

  }

  fprintf(out, "\n");
  fflush(stdout);

}

/********************************************
 * print_type()
 *
 * Print parameter type
 */

static void print_type(FILE *out, const TDRPtable *tt)

{

  int i, j;

  fprintf(out, "//\n");

  switch (tt->ptype) {
    
  case BOOL_TYPE:
    fprintf(out, "// Type: boolean\n");
    return;

  case INT_TYPE:
    fprintf(out, "// Type: int\n");
    return;

  case LONG_TYPE:
    fprintf(out, "// Type: long\n");
    return;

  case FLOAT_TYPE:
    fprintf(out, "// Type: float\n");
    return;

  case DOUBLE_TYPE:
    fprintf(out, "// Type: double\n");
    return;

  case STRING_TYPE:
    fprintf(out, "// Type: string\n");
    return;

  case ENUM_TYPE:
    fprintf(out, "// Type: enum\n");
    fprintf(out, "// Options:\n");
    for (i = 0; i < tt->enum_def.nfields; i++) {
      fprintf(out, "//     %s\n", tt->enum_def.fields[i].name);
    } /* i */
    return;

  case STRUCT_TYPE:
    fprintf(out, "// Type: struct\n");
    fprintf(out, "//   typedef struct {\n");
    for (i = 0; i < tt->struct_def.nfields; i++) {
      fprintf(out, "//      %s %s;\n",
	      tt->struct_def.fields[i].ftype,
	      tt->struct_def.fields[i].fname);
      if (tt->struct_def.fields[i].ptype == ENUM_TYPE) {
	fprintf(out, "//        Options:\n");
	for (j = 0; j < tt->struct_def.fields[i].enum_def.nfields; j++) {
	  fprintf(out, "//          %s\n",
		  tt->struct_def.fields[i].enum_def.fields[j].name);
	} /* j */
      }
    }
    fprintf(out, "//   }\n");
    fprintf(out, "//\n");
    return;

  default:
    return;

  }

}
     
/********************************************
 * print_comment_lines()
 *
 * Print comments, formatting the line breaks.
 */

static void print_comment_lines(FILE *out, const char *str, int format_len)
     
{

  char *buf;
  char *buf2;
  char *next;
  int has_period = 0;

  buf = (char *) tdrpMalloc(strlen(str) + 1);
  strcpy(buf, str);
  buf2 = buf;

  /*
   * strip any trailing new-lines
   */

  if (buf2) {
    while (buf2[strlen(buf2) - 1] == '\n') {
      buf2[strlen(buf2) - 1] = '\0';
    }
  }
  next = strchr(buf2, '\n');

  while (buf2 != NULL) {

    if (next != NULL) {
      *next = '\0';
      next++;
    }
    print_comment(out, buf2, format_len);
    if (buf2[strlen(buf2) - 1] == '.') {
      has_period = TRUE;
    } else {
      has_period = FALSE;
    }
    buf2 = next;
    if (buf2 != NULL) {
      fprintf(out, "\n");
      next = strchr(buf2, '\n');
    }
    if (buf2 != NULL) {
      if (*buf2 == '\n') {
        fprintf(out, "//");
      }
    }
  }

  if (!has_period) {
    fprintf(out, ".");
  }
  fprintf(out, "\n");
  fflush(stdout);

  tdrpFree(buf);

}

/********************************************
 * print_comment()
 *
 * Print comment line,
 * formatting the line breaks at format_len.
 */

static void print_comment(FILE *out, const char *line, int format_len)
     
{

  char *prnstr;
  char *last_space;
  int line_num = 0;

  while (strlen(line) > 0) {
    
    if(line_num == 0) {
      fprintf(out, "// ");
    } else {
      fprintf(out, "//   ");
    }
    line_num++;
    
    /*
     * if line is shorter than format_len, print it and return
     */
    
    if ((int)strlen(line) < format_len) {
      fprintf(out, "%s", line);
      return;
    }

    /*
     * allocate local print string
     */
    
    prnstr = (char *) tdrpMalloc(format_len + 10);

    /*
     * copy formattable segment into print string, ensure
     * null termination.
     */
    
    strncpy(prnstr, line, format_len);
    prnstr[format_len] = '\0';

    /*
     * If a space is found, replace char after last space with NULL
     * to end the string.
     */
    
    last_space = strrchr(prnstr, ' ');
    if (last_space != NULL) {
      last_space[1] = '\0';
    }

    /*
     * print string
     */
    
    fprintf(out, "%s\n", prnstr);

    /*
     * advance line ptr
     */
    
    line += strlen(prnstr);

    /*
     * free up
     */

    tdrpFree(prnstr);

  } /* while */

}

/************************************************************
 * sprintf_val
 *
 * sprintfs a value to a string.
 *
 * NOTE: the string is allocated here, so free it after use.
 */

char *sprintf_val(int ptype, const enum_def_t *enum_def, const tdrpVal_t *val)
     
{

  char *str = NULL;
  int nalloc = 0;
  int i;
  int needed;
  
  str = (char *) tdrpMalloc(256);
  nalloc = 256;
  str[0] = '\0';
  
  /*
   * if tok is non-null, use it. tok is used to store
   * strings which do not decode because of an
   * unexpanded environment variable
   */

  if (val->print.tok) {
    sprintf(str, "%s", val->print.tok);
    return str;
  }

  switch (ptype) {
    
  case BOOL_TYPE:
    sprintf(str, "%s", val->b? "TRUE" : "FALSE");
    return (str);
    break;
    
  case ENUM_TYPE:
    sprintf(str, "UNKNOWN");
    for (i = 0; i < enum_def->nfields; i++) {
      if (val->e == enum_def->fields[i].val) {
	needed = strlen(enum_def->fields[i].name) + 1;
	if (needed > nalloc) {
	  str = (char *) tdrpRealloc(str, needed);
	  nalloc = needed;
	}
	sprintf(str, "%s", enum_def->fields[i].name);
	break;
      }
    }
    return (str);
    break;
    
  case STRING_TYPE:
    if (val->s == NULL) {
      sprintf(str, "%s", "NULL");
    } else {
      needed = strlen(val->s) + 3;
      if (needed > nalloc) {
	str = (char *) tdrpRealloc(str, needed);
	nalloc = needed;
      }
      sprintf(str, "\"%s\"", val->s);
    }
    return (str);
    break;
    
  case INT_TYPE:
    if ( val->i == INT_MIN )
       strcpy(str, MIN_KEYWORD);
    else if ( val->i == INT_MAX )
       strcpy(str, MAX_KEYWORD);
    else
       sprintf(str, "%d", val->i);
    return (str);
    break;
    
  case LONG_TYPE:
    if ( val->l == LONG_MIN )
       strcpy(str, MIN_KEYWORD);
    else if ( val->l == LONG_MAX )
       strcpy(str, MAX_KEYWORD);
    else  
       sprintf(str, "%ld", val->l);
    return (str);
    break;
    
  case FLOAT_TYPE:
    if ( val->f == FLT_MIN )
       strcpy(str, MIN_KEYWORD);
    else if ( val->f == FLT_MAX )
       strcpy(str, MAX_KEYWORD);
    else  
       sprintf(str, "%g", val->f);
    return (str);
    break;
    
  case DOUBLE_TYPE:
    if ( val->d == DBL_MIN )
       strcpy(str, MIN_KEYWORD);
    else if ( val->d == DBL_MAX )
       strcpy(str, MAX_KEYWORD);
    else  
       sprintf(str, "%g", val->d);
    return (str);
    break;
    
  default:
    break;
    
  } /* switch */

  return (str);

}  

