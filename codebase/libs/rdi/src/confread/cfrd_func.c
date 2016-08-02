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

/************************************************************************

Module:	confread.c

Author:	Z. Jing

Date:	1/16/95

Description: Additional functions for the configuration file read module.

************************************************************************/

/* System include files / Local include files */

#include <rdi/confread.h>


/************************************************************************

Function Name: 	CFRD_read_array

Description: This function reads num elements of an array of numbers from
	the configuration file. Refer to confread.doc for a complete
	description.

************************************************************************/

int CFRD_read_array (
    char *key_word, 		/* key word */
    int type,			/* data type */
    int num, 			/* number of elements to read */
    void *array, 		/* array to put returning the numbers */
    int *err			/* returns an error number */
)
{
    int cnt, int_type;

    if (type == CFRD_FLOAT || type == CFRD_DOUBLE) 
	int_type = 0;
    else 
	int_type = 1;

    cnt = 0;			/* number of values read */
    if (err != NULL)
	*err = CFRD_SUCCESS;
    while (1) {			/* for each line */
	char *line;
	int ln, n, off;

	if (cnt >= num)
	    return (cnt);

	/* read next line */
	if ((ln = CFRD_get_next_line ("Elevation_list", &line)) == CFRD_FAILURE) {
				/* key word not found */
	    if (err != NULL)
		*err = CFRD_KEY_NOT_FOUND;
	    return (cnt);	/* return 0 */
	}

	if (line[0] == '\0') 	/* all data are read */
	    return (cnt);

	n = 1;			/* from the first token */
	while (CFRD_find_token (line, n, &off) == n) {	/* for each token */
	    long itmp;
	    double ftmp;
	    int ret;

	    if (cnt >= num) {	/* token left on the line */
		*err = ln;
		return (cnt);
	    }

	    if (int_type == 1)
		ret = sscanf (line + off, "%ld", &itmp);
	    else 
		ret = sscanf (line + off, "%lf", &ftmp);

	    if (ret != 1) {
		if (err != NULL)	/* bad item encountered */
		    *err = ln;	
		return (cnt);
	    }

	    switch (type) {
		case CFRD_CHAR:
		    *((char *)array + cnt) = itmp;
		    break;

		case CFRD_SHORT:
		    *((short *)array + cnt) = itmp;
		    break;

		case CFRD_INT:
		    *((int *)array + cnt) = itmp;
		    break;

		case CFRD_LONG:
		    *((long *)array + cnt) = itmp;
		    break;

		case CFRD_FLOAT:
		    *((float *)array + cnt) = ftmp;
		    break;

		case CFRD_DOUBLE:
		    *((double *)array + cnt) = ftmp;
		    break;

		default:
		    if (err != NULL)
			*err = CFRD_KEY_NOT_FOUND;
		    return (0);

	    }

	    cnt++;
	    n++;
	}
    }

    return (cnt);
}
