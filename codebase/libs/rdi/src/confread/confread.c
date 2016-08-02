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

Date:	8/16/94

Description: The configuration file read module.

************************************************************************/

/*
# ifndef    lint
static char RCSid[] = "$Id: confread.c,v 1.3 2016/03/03 18:47:03 dixon Exp $";
# endif    
*/

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/* System include files / Local include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <rdi/confread.h>

/* Constant definitions / Macro definitions / Type definitions */

#define CFRD_NAME_LEN		256	/* maximum name length */

#define MAX_KEY_LEN		32	/* maximum key word length */

#define LIST_INLINE		1	/* values for Key_word_struct.is_list */
#define LIST_NEXTLINE		2	
#define LIST_NO			0

#define USED			1	/* values for Key_word_struct.used */
#define UNUSED			0


#define TMP_BUF_SIZE	256
#define FILE_NAME_LEN	64		/* be sure this length is
					   save in generating msgs
					   of size TMP_BUF_SIZE */

/* structure for recording the key words */
typedef struct {
    char word[MAX_KEY_LEN];		/* key word */
    int offset;				/* file offset */
    int l_number;			/* current line number */
    int st_number;			/* start line number */
    signed char is_list;		/* LIST_INLINE/LIST_NEXTLINE/LIST_NO */
    signed char used;			/* USED/UNUSED */
} Key_word_struct;


/* Local variables */
static Key_word_struct *Key_words = NULL;/* key word list */
static int Key_word_buf_size = 0;	/* key word buffer size */
static int N_key_word = 0;		/* key word list size */

static char Conf_name [FILE_NAME_LEN];  /* conf file name */
static FILE *Conf_fl = NULL;		/* configuration file handler */
static int L_number;			/* current line number in the file */

static void (*Err_func)(char *msg) = NULL;	/* error function pointer */

/* local functions */


static char * Read_next_line (void);

static int Match_token (char *str,  char *token, int n);

static int Register_key_word (char *cpt,  int offset, int list, int st_number);

static char *Err_msg (int err);

/****************************************************************
			
	CFRD_read_check()			Date: 8/16/94
	
	This function checks that if all key words in the conf.
	file are read. It returns the first line number where the
	key word has not been touched so far.

	It returns CFRD_SUCCESS if all key words are read. 
	Otherwise it returns CFRD_FAILURE.
*/

int 
  CFRD_read_check 
(
    char **keyword		/* returning the unread keyword */
)
{
    int i;

    if (Conf_fl == NULL)
	return (CFRD_FAILURE);

    for (i = 0; i < N_key_word; i++) {
	if (Key_words[i].used == 0) {
	    if (keyword != NULL)
		*keyword = Key_words[i].word;
	    return (Key_words[i].l_number);
	}
    }
    return (CFRD_SUCCESS);
}

/****************************************************************
			
	CFRD_close()				Date: 8/16/94
	
	This function closes all resources.
*/

void 
  CFRD_close ()
{

    fclose (Conf_fl);
    Conf_fl = NULL;
    if (Key_words != NULL)
        free (Key_words);
    Key_words = NULL;
    Key_word_buf_size = 0;	
    N_key_word = 0;	
}


/****************************************************************
			
	CFRD_open()				Date: 8/16/94

	This function opens the configuration file and goes through 
	the file to check errors. The errors are reported
	through calling an external function err_func.

	This function returns CFRD_SUCCESS on success or 
	CFRD_FAILURE on failure.
*/


int
CFRD_open
(
    char *file_name,			/* name of the configuration */
    void (*err_func)()			/* error function pointer */
)
{
    char msg[128], buf[TMP_BUF_SIZE];
    char *cpt;
    int in_list;
    char tmp_buf[LINE_BUF_SIZE];
    int err;

    if (Conf_fl != NULL)		/* can open only once */
	return (CFRD_FAILURE);

    Err_func = err_func;
    strncpy (Conf_name, file_name, FILE_NAME_LEN);
    Conf_name [FILE_NAME_LEN - 1] = 0;
    L_number = 0;

    /* open the configuration file */
    if (Conf_name [0] == '\0' ||
	(Conf_fl = fopen (Conf_name, "r")) == NULL) {
	strcpy (msg, "could not open file");
	goto err;
    }
    CFRD_get_next_line ("CFRD_BEGIN", NULL); /* we must reset the CFRD_get_next_line */

    /* go through the file to check errors */
    in_list = LIST_NO;
    fseek (Conf_fl, 0, 0);
    while (1) {
	int ret, off;
	int st_number;
        int cr_pt;

        cr_pt = ftell (Conf_fl);
	st_number = L_number;
        if ((cpt = CFRD_get_line (Conf_fl, &L_number, &err)) == NULL) {
	    if (err !=  CFRD_EOF) {
		strcpy (msg, Err_msg (err));
	        goto err_close;
	    }
	    break;
	}

	if (in_list == LIST_NO) {		/* not in a list */
	    /* can not be reserved words */
	    if (Match_token (cpt, "BEGIN", 1) == CFRD_SUCCESS ||
	        Match_token (cpt, "END", 1) == CFRD_SUCCESS) {
	        strcpy (msg, "unexpected reserved word");
		goto err_close;
	    }
	    
	    if (CFRD_find_token (cpt, 2, &off) > 1)	{ /* more than one word */
		ret = Match_token (cpt, "BEGIN", 2);
		if (ret == CFRD_SUCCESS) { 		/*  start a list */
		    if (CFRD_find_token (cpt, 3, &off) > 2) {	/* data after BEGIN */
		        strcpy (msg, "data on the same line after BEGIN");
			goto err_close;
		    }
		    in_list = LIST_INLINE;
		}
	    }
	    else {			/* we have to read next line */
		int f_pt, l_n;

		f_pt = ftell (Conf_fl);
		l_n = L_number;

		strcpy (tmp_buf, cpt);
		cpt = CFRD_get_line (Conf_fl, &L_number, &err);	/* read next line */
		if (cpt != NULL) {
		    ret = Match_token (cpt, "BEGIN", 1);
		    if (ret != CFRD_SUCCESS) {	/* the line needs to be returned */
		        fseek (Conf_fl, f_pt, 0);
		        L_number = l_n;
		    }
		    else {			/* BEGIN found */
		        if (CFRD_find_token (cpt, 3, &off) > 1) {/* data after BEGIN */
		            strcpy (msg, "data on the same line after BEGIN");
			    goto err_close;
		        }
		        in_list = LIST_NEXTLINE;
		    }
		}
		else {
		    if (err != CFRD_EOF) {
			strcpy (msg, Err_msg (err));
		        goto err_close;
		    }
		    L_number = l_n;
		}
		cpt = tmp_buf;
	    }

	    /* register this new keyword */
	    if (Register_key_word (cpt, cr_pt, in_list, st_number) != CFRD_SUCCESS) {
		fclose (Conf_fl);
	        return (CFRD_FAILURE);
	    }

	}
	else {				/* in a list */
	    /* check if there is a BEGIN */
	    ret = Match_token (cpt, "BEGIN", 1);
	    if (ret == CFRD_SUCCESS) {
	        strcpy (msg, "BEGIN inside a list");
		goto err_close;
	    }

	    /* Is this the END line? */
	    ret = Match_token (cpt, "END", 1);
	    if (ret == CFRD_SUCCESS) {
		if (CFRD_find_token (cpt, 3, &off) > 1) {/* data after END */
		    strcpy (msg, "data after END");
		    goto err_close;
		}
		in_list = LIST_NO;
	    }
	    continue;
	}
    }

    /* if all BEGINs are ended? */
    if (in_list != LIST_NO) {
        strcpy (msg, "BEGIN not closed");
	goto err_close;
    }

    fseek (Conf_fl, 0, 0);
    L_number = 0;

    return (N_key_word);

   err_close:
    fclose (Conf_fl);
   err:
    if (Err_func != NULL) {
        sprintf (buf, "Cfrd error (line %d, %s): %s\n", L_number, Conf_name, msg);
        Err_func (buf);
    }
    return (CFRD_FAILURE);

}


/*****************************************************************

*/

static char *Err_msg (int err)
{
    static char too_long[] = "Line is too long";
    static char not_ascii[] = "Not an ASCII file";
    static char bad_bs[] = "Unexpected back slash";
    static char bad_env[] = "Bad environment variable";

    if (err == CFRD_LINE_TOO_LONG)
	return (too_long);
    else if (err == CFRD_NON_ASCII)
	return (not_ascii);
    else if (err == CFRD_BAD_BACK_SLASH)
	return (bad_bs);
    else if (err == CFRD_BAD_ENV)
	return (bad_env);
    else 
	return ("End of file");
}

/****************************************************************

	Register_key_word ()

	This function records a key word and associated parameters.
	The function also checks any duplicated key word.

	It returns CFRD_FAILURE if it failed in malloc. Otherwise 
	it returns CFRD_SUCCESS.
*/

static int
  Register_key_word 
(
    char *cpt, 			/* the line containing the key word */
    int offset,			/* file offset */
    int list,			/* is_list flag */
    int st_number		/* start line number of this line */
)
{
    Key_word_struct *key;
    char *pt;
    int len, i;
    char msg[TMP_BUF_SIZE];

    /* alloc (realloc) the space */
    if (N_key_word >= Key_word_buf_size) {
	int size;

	Key_word_buf_size += 20;
	size = Key_word_buf_size * sizeof (Key_word_struct);
	if (Key_words == NULL)
	    Key_words = (Key_word_struct *)malloc (size);
	else 
	    Key_words = (Key_word_struct *)realloc (Key_words, size);
	if ((char *)Key_words == NULL) {
	    if (Err_func != NULL)
                Err_func ("Cfrd error: malloc (realloc) failed");
	    return (CFRD_FAILURE);
	}
    }

    /* find the key word length */
    pt = cpt;
    while (*pt != ' ' && *pt != '\t' && *pt != '\0' && *pt != '\n') 
	pt++;
    len = pt - cpt;
    if (len >= MAX_KEY_LEN - 1) {
        sprintf (msg, "Cfrd error (line %d, %s): Key word too long", L_number, Conf_name);
	if (Err_func != NULL)
	    Err_func (msg);
	return (CFRD_FAILURE);
    }
    key = Key_words + N_key_word;
    strncpy (key->word, cpt, len);
    key->word[len] = '\0';
    key->offset = offset;
    key->is_list = list;
    key->st_number = st_number;
    key->l_number = L_number;
    if (list == LIST_NEXTLINE)
	(key->l_number)--;
    key->used = 0;

    /* check duplicated keywords */
    for (i = 0; i < N_key_word; i++) {
	if (strcmp (Key_words[i].word, key->word) == 0) {
            sprintf (msg, "Cfrd error (line %d, %s): Duplicated key word", L_number, Conf_name);
	    if (Err_func != NULL)
	        Err_func (msg);
	    return (CFRD_FAILURE);
	}
    }
    N_key_word++;

    return (CFRD_SUCCESS);
}


/****************************************************************

	Match_token ()

	This function compares the n-th (counting from 1) token in 
	string "str" with the "token".

	It returns CFRD_SUCCESS on success or CFRD_FAILURE if they
	do not match of the n-th token does not exist.
*/

static int
    Match_token 
(
    char *str, 			/* the string */
    char *token,		/* the token */
    int n			/* which token to be compared */
)
{
    char *pt, c;
    int len;
    int off;

    if (CFRD_find_token (str, n, &off) != n)
	return (CFRD_FAILURE);

    pt = str + off;
    len = strlen (token);
    if (strncmp (pt, token, len) == 0 &&
	((c = pt[len]) == ' ' || c == '\t' || c == '\n' || c == '\0')) 
		return (CFRD_SUCCESS);

    return (CFRD_FAILURE);
}

/****************************************************************

	CFRD_find_token ()

	This function finds the n-th (starting from 1) token and
	returns the offset of the first character of that token in
	argument "off". If the number of tokens in the string is 
	less than n, it returns the number of tokens and offset 
	returned is 0.
*/

int 
  CFRD_find_token
(
    char *string,		/* the string */
    int n,			/* which token to look for */
    int *off			/* offset of the first char */
)
{
    int in, cnt;
    char *pt, c;
    int offset;

    cnt = 0;
    pt = string - 1;
    in = 0;
    offset = 0;
    while (*(pt + 1) != '\0') {
	pt++;
	if (in == 0 && (c = *pt) != ' ' && c != '\t' && c != '\n') {
	    cnt++;
	    in = 1;
	    if (cnt >= n) {
		offset = pt - string;
		break;
	    }
	    continue;
	}
	if ((c = *pt) == ' ' || c == '\t' || c == '\n')
	    in = 0;
    }
    *off = offset;
    return (cnt);
}	

/****************************************************************

*/

int CFRD_read_next_line (char *key_word, int max_len, char **line_buf)
{
    int ret; 
    char msg [128];

    ret = CFRD_get_next_line (key_word, line_buf);
    if (max_len > 0 && ret != CFRD_FAILURE && (int)strlen (*line_buf) > max_len) {
	if (Err_func != NULL) {
            sprintf (msg, "Cfrd error (line %d, %s): Line is longer than expected", 
			L_number, Conf_name);
            Err_func (msg);
	}
	return (CFRD_FAILURE);
    }
    return (ret);
}

	
/****************************************************************
			
	CFRD_get_next_line()			Date: 8/16/94

	This function reads next configuration line specified by 
	"key_word". The pointer to the buffer containing the line 
	data is returned in "line_buf". The returned string can be 
	of length zero if there is nothing except the key word in 
	the line.

	The argument "key_word" can take the value of "CFRD_BEGIN"
	which means the search is started over again.

	It returns the current line number on success. It returns 
	CFRD_FAILURE if the key word is not found or the file is not 
	opened. When it fails, the argument line_buf may not 
	contain useful value.

	Note: This function returns a zero length string if the 
	data is exhausted. The next read with the same key word will
	read the data from beginning of a list. 
*/

int
  CFRD_get_next_line 
(
    char *key_word,			/* the key word */
    char **line_buf			/* buffer pointer for retuning the line */
)	
{
    static int in_list = LIST_NO;
    static char last_key [MAX_KEY_LEN];
    int i;
    char *line;
    int err;

    if (key_word[0] == '\0' || Conf_fl == NULL)
	return (CFRD_FAILURE);

    if (strncmp (key_word, "CFRD_BEGIN", 10) == 0) {
	in_list = LIST_NO;
	return (0);
    }

    /* directly read next line */
    if (in_list != LIST_NO && strncmp (key_word, last_key, MAX_KEY_LEN) == 0) {
	*line_buf = Read_next_line ();
	if (*line_buf[0] == '\0')
	    in_list = LIST_NO;
	return (L_number);
    }

    /* search for the new key word */
    for (i = 0; i < N_key_word; i++) {
	if (strncmp (key_word, Key_words[i].word, MAX_KEY_LEN) == 0)
	    break;
    }
    if (i >= N_key_word)
	return (CFRD_FAILURE);

    /* read the first line */
    fseek (Conf_fl, Key_words[i].offset, 0);
    Key_words[i].used = 1;
    L_number = Key_words[i].st_number;
    in_list = LIST_NO;

    line = CFRD_get_line (Conf_fl, &L_number, &err);
    if (line == NULL && err != CFRD_EOF) {
	if (Err_func != NULL) 
            Err_func ("Fatal error - file is corrupted");
	return (CFRD_FAILURE);
    }

    if (Key_words[i].is_list == LIST_NO) {		/* single line data */
	line += strlen (key_word);
	while (*line != '\0' && (*line == ' ' || *line == '\t'))
	    line++;
	*line_buf = line;
	return (L_number);
    }
    
    /* start a list */
    in_list = LIST_INLINE;
    strcpy (last_key, key_word);
    if (Key_words[i].is_list == LIST_NEXTLINE) { 		/* BEGIN in the next line */
	line = CFRD_get_line (Conf_fl, &L_number, &err);	/* read a line and discard it */
	if (line == NULL && err != CFRD_EOF) {
	    if (Err_func != NULL) 
        	Err_func ("Fatal error - file is corrupted");
	    return (CFRD_FAILURE);
	}
    }

    *line_buf = Read_next_line ();
    if (*line_buf[0] == '\0')				/* END is reached */
	in_list = LIST_NO;

    return (L_number);
}

/****************************************************************
			
	Read_next_line()			Date: 8/16/94

	This function reads the next line and checks if it is
	an "END" line. It returns the pointer to the data buffer.
	If there is no more data or the next line is an "END"
	line it returns a pointer to an empty string.
*/

static char *
   Read_next_line ()
{
    char *line;
    int ret, err;
    static char empty[] = "";

    line = CFRD_get_line (Conf_fl, &L_number, &err);		
    if (line == NULL) {
	if (err != CFRD_EOF) {
	    if (Err_func != NULL) 
        	Err_func ("Fatal error - file is corrupted");
	}
	return (empty);
    }

    ret = Match_token (line, "END", 1);
    if (ret == CFRD_SUCCESS)			/* lists terminated */
	return (empty);

    return (line);
}

