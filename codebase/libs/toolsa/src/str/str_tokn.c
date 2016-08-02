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
#include <string.h>
#include <toolsa/str.h>

char  *
STRtokn( char **str_ptr, char *token, int max_toksiz, char *delim)
/* char  **str_ptr;     current point in the string to parse  */
/* char  *token;        put the found token here             */
/* int   max_toksiz;    maximum size of token (include zero byte) */
/* char  *delim;        delimiter for tokens                 */

/* Returns pointer to the original string where the token was found,
 *  and NULL if no token was found.
 *  str_ptr is updated past the token and delim(s), or set to NULL
 *  when theres nothing more to scan.
 */
{
    int   	delim_index;   /* index in string of the delimiter    */
    char 	*retptr;        /* return value */

    if (NULL == *str_ptr)
	return (NULL);

    /* skip leading delimiter characters */
    while (0 == (delim_index = STRpos( *str_ptr, delim)))
    {
	(*str_ptr)++;
    }

    if (delim_index == -1 )
    {
	/* didnt find the delimiter */
	if ((*str_ptr != NULL) && (*str_ptr[0] != EOS))
	{
	    /* copy nbytes or as much as caller has allocated for
	     * (which ever is less) to string, i.e. to EOS
	     */
	    STRmax_copy( token, *str_ptr, 
			(int) strlen( *str_ptr), max_toksiz);
	    
	    /* scanning pointer set to null - we're done */
	    retptr = (*str_ptr);
	    *str_ptr = NULL;
	    return (retptr);
	}
	else
	{
	    *str_ptr = NULL;
	    return NULL;
	}
    }

    /* copy this token out */
    STRmax_copy( token, *str_ptr, delim_index, max_toksiz);
    
    /* increment scanning pointer past the token and delimiter */
    retptr = (*str_ptr);
    (*str_ptr) += delim_index+1;

    return (retptr);
}
