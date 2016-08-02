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
/*******************************************************************************
 * PARSE DOUBLE : A Routine that returns floating point fields contained in
 *		a given character array separated by any non-digit chars
 *
 * int STRparse_double(inpstr,outfields,maxchr,max_fields);
 *	char	inpstr[];	* input string	*
 *	double	outfields[];	* array of found numeric fields (output)  *
 *	int	maxchr;		* maximum characters to examine in string *
 *	int	max_fields;	* maximum number of fields to fill *
 *
 *	Returns the number of numeric fields found
 *
 * Written by Frank Hage -UNC  7/86
 *
 *
 * For the AWPG Displays 
 * Frank Hage   1991,1992 NCAR, Research Applications Program
 * Modified: 4/13/93 JCaron; include into toolsa/str
 */

#include <stdlib.h>
#include <string.h>
#include <toolsa/str.h>


#ifdef __STDC__
int	STRparse_double(const char *inpstr, double *outfields,int maxchr, int max_fields )
#endif /* __STDC__ */
 
#ifndef __STDC__
int	STRparse_double(inpstr,outfields,maxchr,max_fields)
	const char *inpstr;
	double *outfields;
	int maxchr;
	int max_fields;
#endif /* __STDC__ */
 
{
	char	tmpbuf[1024];	/* temporary buffer for conversion	*/
	char	blanks[1024];
	int	ii = 0;		/* input string counter		*/
	int	jj = 0;		/* temp buffer counter		*/
	int	kk = 0;		/* numeric field counter	*/
	int	fstat = 0;	/* field status: 0=empty, 1=in progress */

	memset(blanks,0,1024);

	while(ii < maxchr && inpstr[ii] != '\000'){ /* not the end */
		/* not a digit, decimal point, plus or minus */
		if(inpstr[ii] <= '\052' || inpstr[ii] == '\057' ||
			inpstr[ii] == '\054' || inpstr[ii] >= '\072' ){  
			if(inpstr[ii]=='\105' || (inpstr[ii]=='\145' && jj != 0)) {
				/* is an 'e' - part of exponential number */
				tmpbuf[jj++] = inpstr[ii++];
			} else {
				tmpbuf[jj] = '\0'; /* terminate conv buffer */
				jj = 0;	/* start a new conversion	*/
				ii++;	/* look at next input character */
			}
		} else { 	/* is part of a numerical field */
			tmpbuf[jj++] = inpstr[ii++];
			fstat = 1;	/* conversion buffer is not empty */
			if(inpstr[ii] == '\055') { /* next char/num is - */
				tmpbuf[jj] = '\0'; /* terminate */
				jj = 0;	/* signal end of current conv */
			}
		}

		if( fstat == 1 && jj == 0 ) { /* field is complete */
			outfields[kk++] = atof(tmpbuf);
			if(kk == max_fields) return(kk);
			fstat = 0;
			strncpy(tmpbuf,blanks,256); /* clear out buffer */
		}
	}

	if(fstat) {	/* something still in conversion buffer */
		tmpbuf[ii] = '\000';	/* add terminator	*/
		outfields[kk++] = atof(tmpbuf);	/* do conversion	*/
	}
	return(kk);
}
