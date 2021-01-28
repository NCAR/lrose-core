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
 * STRING_PARSE : This routine sepatates a long string into substrings.
 * 			String fields are delimited by white space.
 *
 * int STRparse(inpstr,outstr,nchars,max_fields,max_f_len)
 *	char	*inpstr;		* string to be parsed		*
 *	char	*outstr[];		* array for returned substrings      *
 *	int	nchars;			* maximum number of chars in inpstr to look at *
 *	int	max_fields;		* max number of character fileds allowed to fill 
 *	int	max_f_len;		* max_length of character fields to fill
 *
 *
 * For the AWPG Display 
 * Frank Hage   1991 NCAR, Research Applications Program
 * Modified: 4/13/93 JCaron; include into toolsa/str
 */

#include <string.h>
#include <toolsa/str.h>

#define MAX_LINE 4096		/* max chars on one line */
#define BLANK '\040'
#define TAB	'\t'

#ifndef __STDC__
int STRparse(inpstr,outstr,nchars,max_fields,max_f_len)
	const char *inpstr;
	char **outstr;
	int nchars;
	int max_fields;
	int max_f_len;
#endif	/*  __STDC__ */
 
#ifdef __STDC__
int STRparse(const char *inpstr, char **outstr,int nchars, int max_fields, int max_f_len)
#endif /*  __STDC__ */
 
{

	const char * instr;
	char * tbuf;
	int i,end_flag,in_progress;
	int	k = 0;
	char	tmpbuf[MAX_LINE];

	instr = inpstr;
	tbuf = tmpbuf;
	end_flag = 0;
	in_progress = 0;
	i = 0;

	while(*instr == BLANK || *instr == TAB) { /* skip leading white space */
		instr++; 
		i++;
	}

	while(*instr != '\0' && *instr != '\n' && i < nchars ) {
		if(*instr == BLANK || *instr == TAB ){
			if(in_progress) end_flag = 1;	/* signal end of field */
			instr++;
		} else {
			in_progress = 1;	
			*tbuf++ = *instr++;	/* move chars */
		}
		i++;

		if(end_flag & in_progress) {	/* tmpbuf is filled */
			*tbuf = '\0';	/* terminate */
			strncpy(outstr[k++],tmpbuf,max_f_len -1);
			if(k == max_fields) return(k);
			tbuf = tmpbuf;			/* reset temp buffer */
			end_flag = 0;
			in_progress = 0;
		}
	}

	if(in_progress) {	/* still something in temp buffer */
		*tbuf = '\0';
		strncpy(outstr[k++],tmpbuf,max_f_len -1);
	}


	return(k);	/* return # of fields found */
}
