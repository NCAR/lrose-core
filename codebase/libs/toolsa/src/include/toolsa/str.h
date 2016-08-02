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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef STR_WAS_INCLUDED
#define STR_WAS_INCLUDED

#ifndef JAWS_FE_TECH_TRANSFER

/* CREATED: 9/23/91 JCaron
   CHANGES: 4/13/93: add str_parse and str_parse_double

    Note:
    The parameter maxs1 is often used as the maximum size of a string.
    this includes the zero terminator; i.e. it is the dimension of the
    string: char s1[ maxs1]; 
 */


#include <string.h>
#include <toolsa/globals.h>
#include <toolsa/ansi.h>

extern  char *STRbinary(int num, int nbits, char *s);
/* Format "nbits" number of bits from "num" into a printable string of zeros 
   and ones; bit 0 is in char 0, etc. s must be at least nbits+1 in size.
   place result in s and return s. 
 */

extern  char *STRbinaryRev(int num, int nbits, char *s);
/* Format "nbits" number of bits, in reverse, from "num" into a printable strin
   of zeros and ones; bit 0 is in char 0, etc. s must be at least nbits+1 in size.
   place result in s and return s. 
 */

extern  int  STRblnk(char *s);
/* Remove leading and trailing white space from the string s.
   Return the length of the resulting string .
 */

extern  char *STRbpad(char *s, int w);
/* Pad string with blanks, until string has width w (length w+1). 
   Positive w means pad on right (left justify, for strings)
   Negetive w means pad on left (right justify, for numerics) 
   Do nothing if s already has length >= w. 
   Return pointer to string.
 */

extern  char *STRconcat(char *s1, const char *s2,int maxs1);
/* Concat s2 to end of s1, check s1 maximum = maxs1.  return s1.
   Guarentee that s1 is zero terminated.
 */


extern char *STRcopy(char *d, const char *s, int n);
/* Copies the indicated number of characters from string s into
 * string d and ensures that the result is null-terminated.
 */

extern  char *STRdelete(char *s1,int ndelete);
/* Delete the first ndelete chars from s1 
 */

extern char *STRdup(const char *s);
/* Allocates space and duplicates the input string s.
 */

extern  int  STRequal(const char *s1, const char *s2);
/* Returns TRUE if s1 matches s2.
   leading and trailing blanks are not counted.
   upper/lower case ignored. 
   This is a reasonably efficient implementation.
 */

#define STRequal_exact(s1, s2) (strcmp(s1, s2) == 0)
/* Returns TRUE if s1 matches s2 exactly.
 */

extern void STRfree(char *s);
/* Frees space for a string allocated by one of the STR functions.
 * Does nothing if the string pointer is NULL.
 */

extern  int  STRgood(const char *s);
/* Returns TRUE if s is a printable string. 
   All chars must pass issprint(), or be = \n
 */

extern  char *STRinsert(char *s1, const char *s2, int maxs1);
/* Insert s2 into the start of s1; s1 cannot get bigger than maxs1.
   return s1
 */ 

#define STRmove(d,s,n) memmove((void *)(d), (void *)(s),(size_t)(n)) 
/* possible overlapping copy: void STRmove(void *d, void *s, size_t n) 
 */

extern  char *STRmax_copy(char *s1,const char *s2,int max_froms2,int maxs1);
/* Copy up to max_froms2 chars from s2 to s1. 
   Guarentee s1 is zero terminated, does not exceed maxs1 in size.
   Return s1. 
   Use this instead of STRncopy when chars you want to copy from s2 are 
   not zero terminated.
 */

extern  char *STRncopy(char *s1,const char *s2,int maxs1);
/* Copy up to maxs1-1 chars from s2 to s1. 
   Guarentee s1 is zero terminated, does not exceed maxs1 in size.
   Return s1. 
 */

extern int STRparse(const char *inpstr, char **outstr,int nchars, int max_tokens, 
		    int max_token_length);
/*
 * Parse inpstr into white-space delineated tokens. Look at only the first
 *  nchars characters of inpstr, or until a zero byte is found. Place found
 *  tokens into *outstr[]. The caller must allocate the pointer array
 *  outstr[ max_tokens], and allocate the storage that outstr[i] points to.
 * Return number of tokens found.
 *
 *	char	*inpstr;		* string to be parsed
 *	char	*outstr[];		* array for returned substrings   
 *	int	nchars;			* maximum number of chars in inpstr to look at
 *	int	max_tokens;		* max number of tokens 
 *	int	max_token_length;	* max length of each token
 */

extern int STRparse_delim(const char *inpstr, char **outstr, const int nchars,
			  const char *delim_string,
			  const int max_tokens, const int max_token_length);
/*
 * Parse inpstr into tokens. The tokens are deliminated by any character
 * in the given delim_string.  A space in delim_string will match and white
 * space (as defined by isspace()) in the input string. Look at only the first
 * nchars characters of inpstr, or until a zero byte is found. Place found
 * tokens into *outstr[]. The caller must allocate the pointer array
 * outstr[ max_tokens], and allocate the storage that outstr[i] points to.
 *
 * Note that for this routine consecutive delimiters denote empty tokens
 * and are returned as empty strings ("\0").
 *
 * Return number of tokens found.
 *
 *	char	*inpstr;	    * string to be parsed
 *	char	*outstr[];	    * array for returned substrings   
 *	int	nchars;		    * max number of chars in inpstr to look at
 *      char    *delim_string;      * string of delimitting characters
 *	int	max_tokens;	    * max number of tokens 
 *	int	max_token_length;   * max length of each token
 */

extern int STRparse_double(const char *inpstr, double *outfields, int maxchr, int max_fields);
/* 
 * Parse inpstr into tokens, then convert to doubles. The tokens may consist of digits,
 *  a decimal point or + or -. Any other character will be a delimiter.
 *  Look at only the first maxchr characters of inpstr, or until a zero byte is found. 
 *  Place a maximum of max_fields fields into outfields[].
 * Return number of doubles found.
 *
 *	char	inpstr[];	* input string	
 *	double	outfields[];	* array of found numeric fields (output)  
 *	int	maxchr;		* maximum characters to examine in string 
 *	int	max_fields;	* maximum number of fields to fill 
 */


extern  void STRpastoc(char *s, int maxs);
/* Convert pascal string (len in byte 0) to c string (zero terminated). 
 */

extern  int  STRpos(const char *s, const char *c);
/* Find the (0 relative) position of the first occurence of any of  
   the characters in string c in the string s. 
   A space character (' ') in c will match any "isspace()" space character.
   Return -1 if not found.
 */

extern  char *STRremove( char *s, char c);
/*   Remove all chars 'c' from s 
 */

extern char *STRreplace(char *string,
			char *old_substring, char *new_substring);
/*  Replace the given substring with the new substring.  This routine
 *  allocates space for the returned string.  Returns NULL if old_substring
 *  is not a substring of string.
 */

#define STRset(d,c,n)  memset((void *)d,(int)(c),(size_t)(n))       
/* set first n chars of d to c:	void STRset(void *d, int c, size_t n)
 */

extern  void *STRswap(void *dst, void *src, int nbytes);
/*  Swap nbytes from src to dst; guarenteed to work even
    if overlapping regions 
 */

extern  char *STRtokn(char **str_ptr,char *token,int max_toksiz,char *delim);
/*
   Find tokens in string, delimited by any char in the string "delim".
   A space character (' ') in delim will match any "isspace()" space character.
   Return found token in "token".
   Return pointer to the original spot in the string where the token 
   was found, and NULL if no token was found.
   str_ptr is updated past the token and delim(s), or set to NULL
   when theres nothing more to scan.

   Example:
      char  token[ MAX_TOKN];
      char  *ptr;

      ptr = original_string;
      while (NULL != STRtokn( &ptr, token, MAX_TOKN, " ")
         {
         if (STRequal( token, "some"))
            do_something;
         else if (STRequal( token, "any"))
            do_anything;
         };
*/

#else /* specialized subset for JAWS FrontEnd Tech Transfer */

#include <string.h>

#ifndef EOS
#define EOS ((char) 0)
#endif

#define STRequal_exact(s1, s2) (strcmp(s1, s2) == 0)
/* Returns TRUE if s1 matches s2 exactly.
 */

extern  char *STRncopy(char *s1,const char *s2,int maxs1);
/* Copy up to maxs1-1 chars from s2 to s1. 
   Guarentee s1 is zero terminated, does not exceed maxs1 in size.
   Return s1. 
 */

#endif /* JAWS_FE_TECH_TRANSFER */

#endif
#ifdef __cplusplus
}
#endif
