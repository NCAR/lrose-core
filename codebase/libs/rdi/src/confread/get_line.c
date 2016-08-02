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
  
/****************************************************************
		
	File: get_line.c	
				
	8/16/94
	Purpose: This module contains the function CFRD_get_line. 
	Files used: confread.h
	See also: 
	Author: 
	Notes: This module calls an external function 
	    void LOG_send_msg (char *msg, int beep)
	for printing out error messages.

*****************************************************************/

/*
#define TEST
#include <string.h>
*/

#ifdef	   LINT
static char RCSid[] = "$Id: get_line.c,v 1.4 2016/03/03 18:47:03 dixon Exp $";
static char SCCSid[] = "%W% %D% %T%";
#endif /* not LINT */

/* 
 * System include files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rdi/confread.h>

static char *Process_string (char *buf);
static int substitute_env(char *line);


/****************************************************************
			
	CFRD_get_line()			Date: 8/16/94

	This function reads next non-empty and non-comment line in
	file Conf_fl and returns the pointer to the buffer containing 
	the line. Leading spaces and tabs are removed and the line 
	is always both '\n' and null terminated. It returns the current
	line number in argument "l_num". "#" is used for starting a 
	comment. "\#" can be used for character "#". A \n only line is not 
	returned. The char of "\" is interpreted as the to-be-continued 
	symbol. If there is data after "\", an error is generated. "\\" can 
	be used for character "\".

	The function returns the char pointer to the line on success,
	NULL on EOF or error conditions, in which cases the err is set to
	one of the following:

	CFRD_LINE_TOO_LONG
	CFRD_NON_ASCII
	CFRD_BAD_BACK_SLASH
	CFRD_EOF
	CFRD_BAD_ENV
*/

char *
  CFRD_get_line (
    FILE *fl,
    int *l_num,			/* input and output */
    int *err
)
{
    static char buf[LINE_BUF_SIZE + 2];		/* for added line return */	
    char tmp[4];
    char *pt, c;
    int len;
    int p_yes, bs_yes;

    len = LINE_BUF_SIZE;	
    pt = buf;
    while (fgets (pt, len, fl) != NULL) {
	int ll, i;
	int cont;

	if (substitute_env(pt)) {
	  *err = CFRD_BAD_ENV;
	  return (NULL);
	}

	*l_num = *l_num + 1;

	/* go through the string once for effciency reason */
	p_yes = bs_yes = 0;		/* # and back slash found */
	ll = 0;				/* string length */
	
	while ((c = pt [ll]) != '\0') {
	    if (c == '#') 
		p_yes = 1;
	    if (c == '\\')
		bs_yes = 1;
	    ll++;
	}

	if (ll == 0) {				/* leading char is null */
	    *err = CFRD_NON_ASCII;
	    return (NULL); 
	}
	if (pt [ll - 1] != '\n') { 		/* no line return */		
	    if (fgets (tmp, 1, fl) != NULL &&	/* line too long */
		tmp[0] != 0) {	
		*err = CFRD_LINE_TOO_LONG;
	        return (NULL);
	    }
	    pt [ll] = '\n';
	    pt [ll + 1] = '\0';
	    ll++;
	}

	/* process back slash and # and remove comments */
	if (p_yes == 1 || bs_yes == 1) {
	    if (pt [0] == '#') {
		pt [0] = '\n';
		pt [1] = '\0';
	    }
	    else {
		int k = 1;

		while ((c = pt [k]) != '\0') {
		    if (c == '#' || c == '\\') {
			if (pt [k - 1] != '\\') {
			    if (c == '#') {
				pt [k] = '\n';
				pt [k + 1] = '\0';
				break;
			    }
			    else
				k++;
			}
			else {
			    char *ccpt;

			    ccpt = pt + k - 1;
			    while (*ccpt != '\0') {
			        *ccpt = *(ccpt + 1);
			        ccpt++;
			    }
			    if (c == '\\') 		/* set to a tmp value */
				pt [k - 1] = 1;
			}
		    }
		    else
			k++;
		}
	    }
	}

	cont = -1;
	if (bs_yes == 1) {
	    for (i = 0; i < (int)strlen (pt); i++) {
		c = pt[i];
	        if (cont >= 0 && c != ' ' && c != '\t' && c != '\n' ) {
		    *err = CFRD_BAD_BACK_SLASH;
		    return (NULL);
		}
		if (c == '\\') {
		    cont = i;
		    continue;
	        }
		if (c == 1)			/* recover normal \ */
		    pt[i] = '\\';
	    }
	}
	if (cont >= 0) {
	    pt += cont;
	    len -= cont;
	}

	/* empty cont. line should be removed */
	if (cont >= 0 || (pt > buf && Process_string (pt) == NULL))
	    continue;

	if ((pt = Process_string (buf)) != NULL)
	    return (pt);
	pt = buf;			
    }

    /* unfinished to-be-continued line */
    if (pt > buf && (pt = Process_string (buf)) != NULL) 	
	    return (pt);

    *err = CFRD_EOF;
    return (NULL);
}

/********************************************************************

	This function removes leading empty characters and adds an '\n'
	to the end if there is not one. It returns the pointer to
	the processed string or NULL if the string is empty.
*/

static 
char *Process_string (char *buf)
{
    char *pt;

    pt = buf;
    while (pt[0] == ' ' || pt[0] == '\t') 
	pt++;

    if (*pt == '\0' || *pt == '\n') 
	return (NULL);

    return (pt);
}

/******************************************************************
 * substitute_env()
 *
 * Substitute environment variables into the line. The env variables
 * must be in the $(ENV_VAR) format
 *
 * Returns 0 on success, -1 on failure
 */

static int substitute_env(char *line)
{

  char tmp_line[LINE_BUF_SIZE];
  char env_cpy[LINE_BUF_SIZE];

  char *dollar_bracket;
  char *closing_bracket;
  char *pre_str;
  char *env_str;
  char *env_val;
  char *post_str;

  int pre_len, env_len, post_len, tot_len;

  memset ((void *)  env_cpy,
          (int) 0, (size_t)  LINE_BUF_SIZE);

  /*
   * look for opening '$(' sequence
   */

  while ((dollar_bracket = strstr(line, "$(")) != NULL) {

    pre_str = line;
    env_str = dollar_bracket + 2;

    if ((closing_bracket = strchr(env_str, ')')) == NULL) {

      /*
       * no closing bracket
       */

      fprintf(stderr, "WARNING - cfrd:substitute_env\n");
      fprintf(stderr, "No closing bracket for env variable\n");
      fprintf(stderr, "Reading '%s'", line);
      return (-1);

    } /* if ((closing_bracket = ... */

    post_str = closing_bracket + 1;
    
    /*
     * load up env string
     */

    strncpy(env_cpy, env_str, (int) (closing_bracket - env_str));

    /*
     * get env val
     */

    if ((env_val = getenv(env_cpy)) == NULL) {

      /*
       * no env variable set 
       */

      fprintf(stderr, "WARNING - cfrd:substitute_env\n");
      fprintf(stderr, "Env variable '%s' not set\n", env_cpy);
      return (-1);

    }

    /*
     * compute total length after substitution
     */

    pre_len = (int) (dollar_bracket - pre_str);
    env_len = strlen(env_val);
    post_len = strlen(post_str);

    tot_len = pre_len + env_len + post_len + 1;

    if (tot_len > LINE_BUF_SIZE) {

      /*
       * substituted string too long
       */

      fprintf(stderr, "WARNING - cfrd:substitute_env\n");
      fprintf(stderr, "Env str too long.\n");
      fprintf(stderr, "Reading '%s'", line);
      
      return (-1);

    } /* if (tot_len > LINE_BUF_SIZE) */

    /*
     * set tmp line and copy over
     */

    *dollar_bracket = '\0';
    sprintf(tmp_line, "%s%s%s", pre_str, env_val, post_str);
    strncpy(line, tmp_line, LINE_BUF_SIZE);

  } /* while */

  return (0);

}



