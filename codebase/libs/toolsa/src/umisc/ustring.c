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
/**********************************************************************
 * ustring.c
 *
 * String utilities
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1992
 *
 **********************************************************************/

#include <toolsa/umisc.h>

#include <errno.h>

/**********************************************************************
 * uinsert_crs()
 *
 * inserts carriage returns into a string at the
 * blank space immediately preceding the end of line. Formatting for
 * output.
 *
 **********************************************************************/

char *uinsert_crs(char *string, int line_length)
{
  int i, posn, slength;


  urem_wspace(string);
  slength = (int) strlen(string);
  posn = 0;
 
  while (posn + line_length < slength) {
    
    for (i = posn + line_length -1; i >= posn; i--) {
      
      if (string[i] == ' ') {
	string[i] = '\n';
	posn = i + 1;
	break;
      }

    }

    if (i == posn) posn += line_length;

  }

  return string;

}

/**********************************************************************
 * urem_wspace()
 *
 * removes extra tabs and spaces from a string.
 * leaving only one space as a delimiter.
 *
 * returns pointer to the string if successful, NULL if only spaces and
 * tabs in the string.
 *
 **********************************************************************/

char *urem_wspace(char *string)
{
  char *tmpstr1, *tmpstr2, *token;


  /*
   * allocate space for temp strings
   */

  tmpstr1 = (char *) umalloc ((unsigned) (strlen(string) + 1));
  tmpstr2 = (char *) umalloc ((unsigned) (strlen(string) + 1));
  strcpy(tmpstr1, string);
  tmpstr2[0] = '\0';

  token = strtok(tmpstr1, " \t\n");

  if (token == NULL){

    ufree(tmpstr1);
    ufree(tmpstr2);
    return string;

  } else {

    strcat(tmpstr2,token);
    
    while ((token= (strtok((char *) NULL, " \t\n"))) != NULL) {
      strcat(tmpstr2," ");
      strcat(tmpstr2,token);
    }
    strcpy(string, tmpstr2);
    ufree(tmpstr2);
    ufree(tmpstr1);
    return string;
  }

}

/**********************************************************************
 * ustrdelim()
 *
 *  returns a pointer to a string delimited by the delimiting
 *  constant
 *
 * (char *) strdelim( char *nptr, char **endptr, int delim);
 *
 * strdelim searches the string pointed to by nptr for the first
 * occurrence of the delimiter. It returns a pointer to the character
 * after the delimiter.
 * The second occurrence of the delimiter is overwritten by a NULL.
 * A pointer to the first character after the string is assigned to
 * endptr
 *
 * Returns NULL if delimited string is not found
 *
 **********************************************************************/

char *ustrdelim(char *nptr, char **endptr, int delim)
{

  char *start, *end;

  if ((start = strchr(nptr, delim)) == NULL) {
    *endptr = nptr;
    return NULL;
  }

  start++;

  if ((end = strchr(start, delim)) == NULL) {
    *endptr = nptr;
    return NULL;
  }

  *end = '\0';
  *endptr = end + 1;

  return start;

}

/**********************************************************************
 * ustrstr()
 *
 * this is an implementation of the ANSI standard strstr
 * function which is missing from the Unix package
 *
 * char *strstr(char *s1, char *s2)
 *
 * strstr locates the first occurrence of s2 (not including the terminating
 * null character) in the string s1. It returns a pointer to the string
 * located in s1, or a null pointer if no match is found.
 *
 **********************************************************************/

char *ustrstr(char *s1, char *s2)
{

  char *start;
  int i, len_s2;

  start = s1;
  len_s2 = strlen(s2);

  for (i = 0; i < (int) (strlen(s1) - len_s2 + 1); i++) {

    if (!strncmp(s2, start, len_s2)) 
      return start;

    start++;

  }

  return NULL;

}

/**********************************************************************
 * ustr_token()
 *
 * Code from John Caron
 *
 * char  **str_ptr;     current point in the string to parse
 * char  *token;        put the found token here
 * int   max_toksiz;    maximum size of token (include zero byte)
 * char  *delim;        delimiter for tokens
 *
 * Returns pointer to the original string where the token was found,
 * and NULL if no token was found.
 * str_ptr is updated past the token, or set to NULL
 * when theres nothing more to scan.
 */

char *ustr_token(char **str_ptr,
		 char *token,
		 int max_toksiz,
		 char *delim)

{

  char *orig_ptr;
  char *tmp_str;
  char *tok;
  int nfwd;

  orig_ptr = *str_ptr;

  if (*str_ptr == (char *) NULL) {
    return ((char *) NULL);
  }

  tmp_str = (char *) umalloc (max_toksiz + 1);

  strncpy(tmp_str, *str_ptr, max_toksiz);
  tmp_str[max_toksiz] = '\0';

  tok = strtok(tmp_str, delim);

  if (tok == (char *) NULL) {
    *str_ptr = (char *) NULL;
    ufree(tmp_str);
    return ((char *) NULL);
  }

  strncpy(token, tok, max_toksiz);
  token[max_toksiz - 1] = '\0';

  nfwd = (tok - tmp_str) + strlen(tok) + 1;

  *str_ptr += nfwd;

  ufree(tmp_str);

  return (orig_ptr);

}

/********************************************************************
 * ustr_concat()
 *
 * Concats two strings to a given max length
 * 
 * Code from John Caron
 */

char *ustr_concat(char *s1,
		  const char *s2,
		  int maxs1)

{
  int maxc; /* maximum chars to append */
  
  maxc = maxs1 - (int) strlen(s1) - 1;
  
  return strncat(s1, s2,  (size_t) maxc);
}

/********************************************************************
 * ustr_clear_to_end()
 *
 * Clears chars after the first null char, to given limit.
 * Forces a null termination
 */

void ustr_clear_to_end(char *str, int maxlen)

{

  int len, extras;

  *(str + maxlen - 1) = '\0';
  len = strlen(str) + 1;
  extras = maxlen - len;
  
  if (extras > 0) {
    memset(str + len, 0, extras);
  }

  return;

}

/*******************************************************************
 * ustrncpy()
 * 
 * copies string to max length, and ensures NULL termination
 */

char *ustrncpy(char *s1, const char *s2, int maxs1)

{
  if (maxs1 > 0) {
    strncpy(s1, s2, (size_t) (maxs1-1));
    s1[maxs1-1] = '\0';
  }
  return(s1);
}

/***************************************************************************
 *
 * int ustrtola (char *lngstr, si32 *lngarr, int size)
 *
 * takes
 *  lngstr, which is assumed to be a string consisting of digits
 *  separated by whitespace (newlines, spaces, etc.), and
 *  converts it into an array of long ints (whose size is
 *  given by "size") with a starting address at lngarr.
 *
 *  strtola returns size if it executes without errors, -1 otherwise.
 *
 *  Preconditions: "lngstr" has already been initialized.
 *    "size" is equal to the number of integers to be
 *    contained in the array, or 0 if that number is
 *    unknown.
 *
 *  Postconditions: "lngarr" points to the first element of the si32
 *    array.
 *
 ****************************************************************************/

int ustrtola (char *lngstr, long **lngarr_P, int size)

{
  
  char *bad = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,./<>?;:'[]{}\\|`~!@#$%^&*()_=";
  char *lcopy, *token;
  char *startptr, *endptr;
  long *lngarr;
  int index = 0;

  if (!lngstr)			/* exits if lngstr is NULL */
    return (0);

  if (strpbrk(lngstr, bad))
    return (-1);

  /*
   * determine number of tokens
   */


  if (!size) {			/* if size == 0, calculate the number of
				 * integers within the string */
    /*
     * copy string
     */

    lcopy = (char *) umalloc (strlen(lngstr) + 1);
    strcpy(lcopy, lngstr);
    
    token = strtok(lcopy, " \n\t\r");

    while (token != NULL) {
      size++;
      token = strtok((char *) NULL, " \n\t\r");
    }

    free(lcopy);

  } /* if (!size) */

  lngarr = (long *)ucalloc(size, sizeof(long));
  index = 0;
  startptr = lngstr;

  while (index < size) {
    lngarr[index++] = strtol(startptr, &endptr, 10);
    startptr = endptr;
  } /* while */
  
  *lngarr_P = lngarr;
  return (size);

}

/***************************************************************
 * uGetLong()
 *
 * Decodes the next long off a string, advancing the pointer
 * to the next token
 *
 * Returns 0 on success, -1 on failure
 *
 * Code from John Caron
 */

long uGetLong(char **ptr, long *lval)

{

  char token[128];
  char *end_pt;
  
  if (ustr_token(ptr, token, 128, " \n") == NULL) {

    *lval = 0;
    return (-1);

  } else {

    errno = 0;
    *lval = strtol(token, &end_pt, 10);

    if (errno) {
      *lval = 0;
      return (-1);
    } else {
      return (0);
    }

  }

}

/***************************************************************
 * uGetDouble()
 *
 * Decodes the next double off a string, advancing the pointer
 * to the next token
 *
 * Returns 0 on success, -1 on failure
 *
 * Code from John Caron
 */

int uGetDouble(char **ptr, double *dval)
     
{

  char token[128];
  char *end_pt;
  
  if (ustr_token(ptr, token, 128, " \n") == NULL) {

    *dval = 0.0;
    return (-1);

  } else {

    errno = 0;
    *dval = strtod(token, &end_pt);

    if (errno) {
      *dval = 0.0;
      return (-1);
    } else {
      return (0);
    }

  }

}

