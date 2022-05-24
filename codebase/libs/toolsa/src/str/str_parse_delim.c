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
/****************************************************************************
 * STRING_PARSE_DELIM : This routine sepatates a long string into substrings.
 * 			String fields are delimited by delimiters specified
 *                      by the user. A space in the delimiter string matches
 *                      any white space (as defined by isspace()) in the
 *                      input string.  Note that for this routine consecutive
 *                      delimiters denote empty tokens and are returned as
 *                      empty strings ("\0").
 *
 * int STRparse_delim(inpstr,outstr,nchars,delim_string,max_fields,max_f_len)
 *	char	*inpstr;        * string to be parsed
 *	char	*outstr[];      * array for returned substrings
 *	int	nchars;	        * maximum number of chars in inpstr to look at
 *      char    *delim_string;  * string of delimitting characters
 *	int	max_fields;     * max number of char fileds allowed to fill 
 *	int	max_f_len;      * max_length of character fields to fill
 *
 *********************************************************************/

#include <ctype.h>
#include <string.h>
#include <toolsa/str.h>

#define MAX_LINE 4096		/* max chars on one line */


#ifndef __STDC__
int STRparse_delim(inpstr,outstr,nchars,delim_string,max_fields,max_f_len)
	const char *inpstr;
	char **outstr;
	const int nchars;
        const char *delim_string;
	const int max_fields;
	const int max_f_len;
#endif	/*  __STDC__ */
 
#ifdef __STDC__
int STRparse_delim(const char *inpstr, char **outstr, const int nchars,
		   const char *delim_string,
		   const int max_fields, const int max_f_len)
#endif /*  __STDC__ */
 
{
  
  const char * instr;
  char * tbuf;
  int i, end_flag, check_spaces;
  int	num_fields = 0;
  char	tmpbuf[MAX_LINE];

  /*
   * Initialize local variables.
   */

  instr = inpstr;
  tbuf = tmpbuf;
  end_flag = 0;
  i = 0;

  /*
   * See if we are using white space as part of the delimiters.
   */

  if (strchr(delim_string, ' ') == NULL)
    check_spaces = 0;
  else
    check_spaces = 1;
  
  /*
   * Pull off all of the tokens.
   */
	    
  while (*instr != '\0' && *instr != '\n' && i < nchars)
  {
    /*
     * Have we found a delimiter?
     */

    if (strchr(delim_string, *instr) != NULL ||
	(check_spaces && isspace(*instr)))
    {
      end_flag = 1;	/* signal end of field */
      instr++;
    }
    else
    {
      *tbuf++ = *instr++;	/* move chars */
    }
    i++;

    /*
     * Process the end of the token.
     */

    if (end_flag)	/* tmpbuf is filled */
    {
      *tbuf = '\0';	/* terminate */
      strncpy(outstr[num_fields++], tmpbuf, max_f_len -1);
      if (num_fields == max_fields)
	return(num_fields);
      tbuf = tmpbuf;			/* reset temp buffer */
      end_flag = 0;
    }
  }

  /*
   * Make sure we finish processing the last token.
   */

  *tbuf = '\0';
  strncpy(outstr[num_fields++], tmpbuf, max_f_len -1);

  return(num_fields);	/* return # of fields found */
}
