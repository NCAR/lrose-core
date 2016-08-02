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

/***********************************
 * mem.c
 * 
 * simple memory allocation library.
 *
 * Causes exit on alloc error.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <tdrp/tdrp.h>

static void free_struct_str(void *fld, int ptype);
static void realloc_1D_array(TDRPtable *tt, void *params,
			     int new_array_n);
static void realloc_struct_array(TDRPtable *tt, void *params,
				 int new_array_n);

/**************
 * tdrpMalloc()
 *
 * Malloc wrapper - exits with message if malloc fails.
 * Guaranteed valid pointer if it returns.
 */

void *tdrpMalloc(size_t size)
     
{

  void *addr;
  
  if (size == 0) {
    return (NULL);
  }
  
  if ((addr = (void *) malloc(size)) == NULL) {
    fprintf(stderr, "ERROR - tdrpMalloc\n");
    fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
    exit(-1);
  }
  memset(addr, 0, size);
  
  return (addr);

}

/**************
 * tdrpCalloc()
 *
 * Calloc wrapper - exits with message if calloc fails.
 * Guaranteed valid pointer if it returns.
 */

void *tdrpCalloc (size_t nelem, size_t elsize)
     
{

  void *addr;

  int size = nelem * elsize;
  if (size == 0) {
    return (NULL);
  }
  
  if ((addr = (void *) calloc(nelem, elsize)) == NULL) {
    fprintf(stderr, "ERROR - tdrpCalloc\n");
    fprintf(stderr, "Cannot perform calloc, nelem, elsize = %d, %d\n",
    	    (int) nelem, (int) elsize);
    exit(-1);
  }
  
  return (addr);

}

/***************
 * tdrpRealloc()
 *
 * Realloc wrapper - exits with message if realloc fails.
 * Guaranteed valid pointer if it returns.
 */

void *tdrpRealloc (void *ptr, size_t size)

{

  void *addr;

  if (size == 0) {
    if (ptr != NULL) {
      free (ptr);
    }
    return (NULL);
  }

  if ((addr = (void *) realloc(ptr, size)) == NULL) {
    fprintf(stderr, "ERROR - tdrpRealloc\n");
    fprintf(stderr, "Cannot perform realloc, size = %d\n", (int) size);
    exit(-1);
  }
  
  return (addr);

}

/************
 * tdrpFree()
 *
 * Frees memory allocated with tdrp routines
 */

void tdrpFree (const void *ptr)
{
  if (ptr != (void *)NULL)
    free ((void *) ptr);
}

/**************
 * tdrpStrDup()
 *
 * String duplication using tdrp malloc routines.
 */

char *tdrpStrDup(const char *s1)
{
  char *s2;
  if (s1 == NULL) {
    return NULL;
  }
  s2 = (char *)  tdrpMalloc(strlen(s1) + 1);
  strcpy(s2, s1);
  return (s2);
}

/******************
 * tdrpStrReplace()
 *
 * Replacement of string created by tdrpStrDup()
 */

void tdrpStrReplace(char **s1, const char *s2)
{
  if (*s1 != NULL) {
    tdrpFree(*s1);
  }
  if (s2 == NULL) {
    *s1 = NULL;
  } else {
    *s1 = tdrpStrDup(s2);
  }
}

/****************
 * tdrpStrNCopy()
 *
 * Safe strncpy - guarantees a valid null-terminated string.
 */

char *tdrpStrNcopy(char *s1, const char *s2, int maxs1)
{
  if (!s1 || !s2)
    return NULL;

  if (maxs1 > 0) {
    strncpy(s1, s2, (size_t) (maxs1-1));
    s1[maxs1-1] = '\0';
  }
  return(s1);
}

/*****************
 * tdrpFreeTable()
 *
 * Free up entire table.
 */

void tdrpFreeTable(TDRPtable *table)

{
  TDRPtable *tt;
  tt = table;
  while (tt->param_name != NULL) {
    tdrpFreeEntry(tt);
    tt++;
  }
}

/*****************
 * tdrpFreeEntry()
 *
 * Free up single table entry
 */

void tdrpFreeEntry(TDRPtable *tt)

{

  int i, j;
  int ifield;

  tdrpFree(tt->param_name);

  if (tt->ptype == COMMENT_TYPE) {
    tdrpFree(tt->comment_hdr);
    tdrpFree(tt->comment_text);
  } else {
    tdrpFree(tt->help);
    tdrpFree(tt->descr);
  }

  if (tt->ptype == STRING_TYPE) {
    if (tt->is_array) {
      for (i = 0; i < tt->array_n; i++) {
	tdrpFree(tt->array_vals[i].s);
      }
    } else {
      tdrpFree(tt->single_val.s);
    }
  }

  if (tt->ptype == ENUM_TYPE) {
    for (i = 0; i < tt->enum_def.nfields; i++) {
      tdrpFree(tt->enum_def.fields[i].name);
    }
    tdrpFree(tt->enum_def.fields);
    tdrpFree(tt->enum_def.name);
  }
  
  if (tt->ptype == STRUCT_TYPE) {

    for (i = 0; i < tt->n_struct_vals; i++) {
      ifield = i % tt->struct_def.nfields;
      if (tt->struct_def.fields[ifield].ptype == STRING_TYPE) {
	tdrpFree(tt->struct_vals[i].s);
      }
    }
    tdrpFree(tt->struct_vals);

    for (i = 0; i < tt->struct_def.nfields; i++) {
      if (tt->struct_def.fields[i].ptype == ENUM_TYPE) {
	for (j = 0; j < tt->struct_def.fields[i].enum_def.nfields; j++) {
	  tdrpFree(tt->struct_def.fields[i].enum_def.fields[j].name);
	}
	tdrpFree(tt->struct_def.fields[i].enum_def.name);
	tdrpFree(tt->struct_def.fields[i].enum_def.fields);
      }
      tdrpFree(tt->struct_def.fields[i].ftype);
      tdrpFree(tt->struct_def.fields[i].fname);
    }

    tdrpFree(tt->struct_def.name);
    tdrpFree(tt->struct_def.fields);

  } else {
    
    if ( tt->is_array) {
      tdrpFree(tt->array_vals);
    }
    
  } /* if (tt->ptype == STRUCT_TYPE) */

  memset(tt, 0, sizeof(TDRPtable));

}

/****************
 * tdrpFreeUser()
 *
 * Free up the user struct fields.
 */

void tdrpFreeUser(TDRPtable *table, void *params)

{

  int i, j;
  TDRPtable *tt = table;
  void **array_p;
  void ***array2D_p;
  void **val_ptr;
  char **cptr;

  while (tt->param_name != NULL) {

    if (tt->ptype == STRING_TYPE) {

      /*
       * free up strings
       */

      if (tt->is_array) {
	array_p = (void **) ((char *) params + tt->array_offset);
	cptr = (char **) *array_p;
	if (cptr) {
	  for (i = 0; i < tt->array_n; i++) {
	    tdrpFree(cptr[i]);
	    cptr[i] = NULL;
	  }
	}
      } else {
	cptr = (char **) ((char *) params + tt->val_offset);
	tdrpFree(*cptr);
	*cptr = NULL;
      }

    } else if (tt->ptype == STRUCT_TYPE) {
      
      /*
       * free up string fields in structs
       */

      if (tt->is_array) {
	
	val_ptr = (void **) ((char *) params + tt->array_offset);
	
	if (*val_ptr) {
	  for (i = 0; i < tt->array_n; i++) {
	    for (j = 0; j < tt->struct_def.nfields; j++) {
	      int field_offset = i * tt->array_elem_size +
		tt->struct_def.fields[j].rel_offset;
	      free_struct_str((char *) (*val_ptr) + field_offset,
			      tt->struct_def.fields[j].ptype);
	    } /* j */
	  } /* i */
	}

      } else {
	
	for (j = 0; j < tt->struct_def.nfields; j++) {
	  int field_offset = tt->val_offset +
	    tt->struct_def.fields[j].rel_offset;
	  free_struct_str((char *) params + field_offset,
			  tt->struct_def.fields[j].ptype);
	}

      }

    } /* if (tt->ptype == STRUCT_TYPE) */

    /*
     * free up arrays
     */
    
    if (tt->is_array) {
      if (tt->is_array2D) {
	array2D_p = (void ***) ((char *) params + tt->array2D_offset);
	if (*array2D_p != NULL) {
	  tdrpFree(*array2D_p);
	  *array2D_p = NULL;
	}
      }
      array_p = (void **) ((char *) params + tt->array_offset);
      if (*array_p != NULL) {
	tdrpFree(*array_p);
	*array_p = NULL;
      }
    }

    tt++;
  
  } /* while */

}

/***************
 * tdrpFreeAll()
 *
 * Free up the table and user struct fields.
 */

void tdrpFreeAll(TDRPtable *table, void *params)

{
  tdrpFreeUser(table, params);
  tdrpFreeTable(table);
}

/********************
 * tdrpArrayRealloc()
 *
 * Realloc 1D array.
 *
 * If size is increased, the values from the last array entry is
 * copied into the new space.
 *
 * Returns 0 on success, -1 on error.
 */

int tdrpArrayRealloc(TDRPtable *table, void *params,
		     const char *param_name, int new_array_n)

{

  int param_found = FALSE;
  TDRPtable *tt = table;

  while (tt->param_name != NULL) {
    
    if (!strcmp(tt->param_name, param_name)) {

      if (!tt->is_array || tt->is_array2D) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
	fprintf(stderr, "Parameter '%s' - illegal operation\n", param_name);
	fprintf(stderr, "Must be 1D array.\n");
	fprintf(stderr, "Realloc not performed.\n");
	return (-1);
      }

      if (tt->array_len_fixed) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
	fprintf(stderr, "Parameter '%s' - illegal operation\n", param_name);
	fprintf(stderr,
		"Array length is fixed at [%d], may not be changed.\n",
		tt->array_n);
	fprintf(stderr, "Realloc not performed.\n");
	return (-1);
      }

      if (tt->ptype == STRUCT_TYPE) {

	realloc_struct_array(tt, params, new_array_n);

      } else {

	realloc_1D_array(tt, params, new_array_n);
	
      }

      /*
       * set new size
       */
      
      tt->array_n  = new_array_n;
      *((int *) ((char *) params + tt->len_offset)) = tt->array_n;
      *((int *) ((char *) params + tt->array_n_offset)) = tt->array_n;
      
      param_found = TRUE;
      break;
      
    } /* if (!strcmp ... */
    
    tt++;
    
  } /* while */

  if (!param_found) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
    fprintf(stderr, "No parameter named '%s'\n", param_name);
    fprintf(stderr, "Realloc not performed.\n");
    return (-1);
  }

  return (0);

}

/**********************
 * tdrpArray2DRealloc()
 *
 * Realloc 2D array.
 *
 * If size is increased, the values from the last array entry is
 * copied into the new space.
 *
 * Returns 0 on success, -1 on error.
 */

int tdrpArray2DRealloc(TDRPtable *table, void *params,
		       const char *param_name,
		       int new_array_n1, int new_array_n2)

{

  int j;
  int param_found = FALSE;
  TDRPtable *tt = table;
  int new_array_n = new_array_n1 * new_array_n2;
  void **array_p;
  void ***array2D_p;

  while (tt->param_name != NULL) {
    
    if (!strcmp(tt->param_name, param_name)) {

      if (!tt->is_array2D) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
	fprintf(stderr, "Parameter '%s' - illegal operation\n", param_name);
	fprintf(stderr, "Must be 2D array.\n");
	fprintf(stderr, "Realloc not performed.\n");
	return (-1);
      }
      
      if (tt->array_len_fixed) {
	fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
	fprintf(stderr, "Parameter '%s' - illegal operation\n", param_name);
	fprintf(stderr,
		"Array length is fixed at [%d], may not be changed.\n",
		tt->array_n);
	fprintf(stderr, "Realloc not performed.\n");
	return (-1);
      }

      /*
       * do the 1D part of the operation
       */
      
      realloc_1D_array(tt, params, new_array_n);

      /*
       * set new sizes
       */
      
      tt->array_n  = new_array_n;
      tt->array_n1 = new_array_n1;
      tt->array_n2 = new_array_n2;
      *((int *) ((char *) params + tt->len_offset)) = tt->array_n;
      *((int *) ((char *) params + tt->array_n_offset)) = tt->array_n;
      *((int *) ((char *) params + tt->array_n1_offset)) = tt->array_n1;
      *((int *) ((char *) params + tt->array_n2_offset)) = tt->array_n2;
      
      /*
       * realloc the 2D pointers
       */
	
      array_p = (void **) ((char *) params + tt->array_offset);
      array2D_p = (void ***) ((char *) params + tt->array2D_offset);
      *array2D_p = (void **) tdrpRealloc(*array2D_p,
					 tt->array_n1 * sizeof(void *));
      for (j = 0; j < tt->array_n1; j++) {
	(*array2D_p)[j] = (char *) *array_p +
	  j * tt->array_n2 * tt->array_elem_size;
      }

      param_found = TRUE;
      break;
      
    } /* if (!strcmp ... */
    
    tt++;
    
  } /* while */

  if (!param_found) {
    fprintf(stderr, "\n>>> TDRP_ERROR <<< - tdrpArrayRealloc\n");
    fprintf(stderr, "No parameter named '%s'\n", param_name);
    fprintf(stderr, "Realloc not performed.\n");
    return (-1);
  }

  return (0);

}

/********************
 * realloc_1D_array()
 *
 * Realloc for 1D array for all types except structs
 */

static void realloc_1D_array(TDRPtable *tt, void *params,
			     int new_array_n)

{

  int i;
  int nnew, offset, len;
  void **array_p;
  char **cptr;

  if (new_array_n == tt->array_n) {
    return;
  }

  /*
   * if size is decreasing, free up any strings in the 
   * area to be released in both the params and table
   */

  if (new_array_n < tt->array_n) {
    if (tt->ptype == STRING_TYPE) {
      array_p = (void **) ((char *) params + tt->array_offset);
      cptr = (char **) *array_p;
      for (i = new_array_n; i < tt->array_n; i++) {
	tdrpFree(cptr[i]);
	cptr[i] = NULL;
	tdrpFree(tt->array_vals[i].s);
	tt->array_vals[i].s = NULL;
      }
    }
  }

  /*
   * realloc the array in the user struct
   */
  
  array_p = (void **) ((char *) params + tt->array_offset);
  *array_p = (void *) tdrpRealloc(*array_p,
				  new_array_n * tt->array_elem_size);
  *((void **) ((char *) params + tt->val_offset))= *array_p;
  
  /*
   * realloc space in table
   */
  
  tt->array_vals = (tdrpVal_t*)
    tdrpRealloc(tt->array_vals, new_array_n * sizeof(tdrpVal_t));

  if (new_array_n > tt->array_n) {

    /*
     * initialize new memory
     */
    
    nnew = new_array_n - tt->array_n;
    
    offset = tt->array_n * tt->array_elem_size;
    len = nnew * tt->array_elem_size;
    memset((char *) *array_p + offset, 0, len);

    offset = tt->array_n * sizeof(tdrpVal_t);
    len = nnew * sizeof(tdrpVal_t);
    memset((char *) tt->array_vals + offset, 0, len);
    
  } /* if (new_array_n > tt->array_n) */

#ifdef NOTNAYMORE
  
  /*
   * if array increased in size, copy last entry to all locations
   * in new area, unless the original size is 0 in which case we
   * clear the new area
   */
  
  if (new_array_n > tt->array_n) {

    if (tt->array_n > 0) {

      /*
       * shallow copy
       */
      
      for (i = tt->array_n; i < new_array_n; i++) {
	memcpy((char *) *array_p + i * tt->array_elem_size,
	       (char *) *array_p + (tt->array_n - 1) * tt->array_elem_size,
	       tt->array_elem_size);
	
      }
      
      /*
       * deep copy for strings
       */
      
      if (tt->ptype == STRING_TYPE) {
	array_p = (void **) ((char *) params + tt->array_offset);
	cptr = (char **) *array_p;
	for (i = tt->array_n; i < new_array_n; i++) {
	  cptr[i] = tdrpStrDup(cptr[i]);
	  tt->array_vals[i].s = tdrpStrDup(tt->array_vals[i].s);
	}
      }

    } else {

      /*
       * no original data, clear area
       */
      
      memset(*array_p, 0, new_array_n * tt->array_elem_size);

    }

  } /* if (new_array_n > tt->array_n) */

#endif

}

/************************
 * realloc_struct_array()
 *
 * Realloc for 1D array of structs
 */

static void realloc_struct_array(TDRPtable *tt, void *params,
				 int new_array_n)

{

  int i, j;
  int nnew, offset, len;
  void **val_ptr;
  void **array_p;

  if (new_array_n == tt->array_n) {
    return;
  }

  /*
   * if size is decreasing, free up any strings in the 
   * area to be freed
   */
  
  if (new_array_n < tt->array_n) {
    
    val_ptr = (void **) ((char *) params + tt->array_offset);
    for (i = new_array_n; i < tt->array_n; i++) {
      for (j = 0; j < tt->struct_def.nfields; j++) {
	if (tt->struct_def.fields[j].ptype == STRING_TYPE) {
	  int index = i * tt->struct_def.nfields + j;
	  int field_offset = i * tt->array_elem_size +
	    tt->struct_def.fields[j].rel_offset;
	  free_struct_str((char *) (*val_ptr) + field_offset,
			  tt->struct_def.fields[j].ptype);
	  tdrpFree(tt->struct_vals[index].s);
	  tt->struct_vals[index].s = NULL;
	}
      } /* j */
    } /* i */

  } /* if (new_array_n < tt->array_n) */
      
  /*
   * realloc in user struct
   */
      
  array_p = (void **) ((char *) params + tt->array_offset);
  *array_p = (void *) tdrpRealloc(*array_p,
				  new_array_n * tt->array_elem_size);
  *((void **) ((char *) params + tt->val_offset))= *array_p;

  /*
   * realloc in table
   */
  
  tt->n_struct_vals = new_array_n * tt->struct_def.nfields;
  tt->struct_vals = (tdrpVal_t*)
    tdrpRealloc(tt->struct_vals, tt->n_struct_vals * sizeof(tdrpVal_t));

  if (new_array_n > tt->array_n) {

    /*
     * initialize new memory
     */
    
    nnew = new_array_n - tt->array_n;
    
    offset = tt->array_n * tt->array_elem_size;
    len = nnew * tt->array_elem_size;
    memset((char *) *array_p + offset, 0, len);

    offset = tt->array_n * tt->struct_def.nfields * sizeof(tdrpVal_t);
    len = nnew * tt->struct_def.nfields * sizeof(tdrpVal_t);
    memset((char *) tt->struct_vals + offset, 0, len);
    
  } /* if (new_array_n > tt->array_n) */

#ifdef NOTANYMORE

  /*
   * if array increased in size, copy last entry to all locations
   * in new area, unless the original size is 0 in which case we
   * clear the new area
   */
  
  if (new_array_n > tt->array_n) {

    char **cptr;

    if (tt->array_n > 0) {

      /*
       * shallow copy
       */
      
      for (i = tt->array_n; i < new_array_n; i++) {
	memcpy((char *) *array_p + i * tt->array_elem_size,
	       (char *) *array_p + (tt->array_n - 1) * tt->array_elem_size,
	       tt->array_elem_size);
	
      }
      
      /*
       * deep copy for strings
       */
      
      val_ptr = (void **) ((char *) params + tt->array_offset);
      for (i = tt->array_n; i < new_array_n; i++) {
	for (j = 0; j < tt->struct_def.nfields; j++) {
	  if (tt->struct_def.fields[j].ptype == STRING_TYPE) {
	    int index = i * tt->struct_def.nfields + j;
	    int field_offset = i * tt->array_elem_size +
	      tt->struct_def.fields[j].rel_offset;
	    cptr = (char **) ((char *) (*val_ptr) + field_offset);
	    *cptr = tdrpStrDup(*cptr);
	    tt->struct_vals[index].s = tdrpStrDup(*cptr);
	  }
	} /* j */
      } /* i */

    } else {

      /*
       * no original data, clear area
       */
      
      memset(*array_p, 0, new_array_n * tt->array_elem_size);

    }

  } /* if (new_array_n > tt->array_n) */

#endif

}

/*******************
 * free_struct_str()
 *
 * Free up a string type field in a struct
 */

static void free_struct_str(void *fld, int ptype)

{
  char **cptr;
  if (ptype == STRING_TYPE) {
    cptr = (char **) fld;
    tdrpFree(*cptr);
    *cptr = NULL;
  }
}
