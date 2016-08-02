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
/***************************************************************************
 * reutil.c
 *
 * Utilities for using UNIX regular expressions.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1995
 *
 ****************************************************************************/

#include <toolsa/os_config.h>
#include <toolsa/reutil.h>

/***************************************************************************
 * REUTIL_get_file_list
 *
 * Gets a list of files in a directory that match the given list of
 * regular expressions.  The returned list will either have the files
 * in alphabetical order or in the order returned by the readdir()
 * function.  This function allocates space for the file name list
 * returned.  This space must be freed by the calling process.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1995
 *
 ****************************************************************************/

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/param.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_ERR_LENGTH 512
#define EXP_BUF_SIZE  1024

/*
 * Local typedefs.
 */

typedef struct file_llist_t
{
  char *filename;
  struct file_llist_t *next, *prev;
} file_llist_t;


char **REUTIL_get_file_list(const char *dir_path,
			    char **reg_exp_string,
			    int  num_reg_exp,
			    int  sort_flag,
			    int  *num_files)
{

  regex_t *exps;

  char **file_list = NULL;
  file_llist_t *file_llist = NULL;
  file_llist_t *last_llist = NULL;
  file_llist_t *new_llist;
  file_llist_t *check_llist;
  
  DIR *dirp;
  struct dirent *dp;
  int file_matches;
  int i;

  int err;
  char err_msg[MAX_ERR_LENGTH];

  /*
   * Initialize return values.
   */

  *num_files = 0;
  
  /*
   * Compile the regular expressions.
   */

  exps = (regex_t *) malloc(num_reg_exp * sizeof(regex_t));
  
  for (i = 0; i < num_reg_exp; i++)  {
    
    /*
     * compile the RE. If this step fails, reveals what's wrong with the RE
     */
    
    if ((err = regcomp(&exps[i], reg_exp_string[i], REG_EXTENDED)) != 0) {
      regerror(err, &exps[i], err_msg, MAX_ERR_LENGTH);
      fprintf(stderr, "ERROR - REUTIL_get_file_list\n");
      fprintf(stderr, "  Cannot compile expression '%s': %s\n",
              reg_exp_string[i], err_msg);
    }
    
  } /* endfor - i */
  
  /*
   * Open the directory entry.
   */

  if ((dirp = opendir(dir_path)) == (DIR *)NULL)
    return(file_list);
  
  /*
   * Loop through the files in the directory, saving the names of
   * the desired files.
   */

  while ((dp = readdir(dirp)) != NULL)
  {
    file_matches = FALSE;
    for (i = 0; i < num_reg_exp; i++) {
      if ((err = regexec(&exps[i], dp->d_name, 0, NULL, 0)) == 0)
      {
	file_matches = TRUE;
	break;
      }
    } /* endfor = i */
    
    if (file_matches)
    {
      new_llist = (file_llist_t *)malloc(sizeof(file_llist_t));
      new_llist->filename = (char *)malloc(MAXPATHLEN);
      new_llist->next = NULL;
      new_llist->prev = NULL;
      
      strcpy(new_llist->filename, dir_path);
      strcat(new_llist->filename, "/");
      strcat(new_llist->filename, dp->d_name);
      
      if (file_llist == NULL)
      {
	file_llist = new_llist;
	last_llist = new_llist;
      }
      else if (sort_flag)
      {
	for (check_llist = file_llist;
	     check_llist != NULL;
	     check_llist = check_llist->next)
	{
	  if (strcmp(check_llist->filename, new_llist->filename) >= 0)
	    break;
	}
	
	if (check_llist == NULL)
	{
	  last_llist->next = new_llist;
	  new_llist->prev = last_llist;
	  
	  last_llist = new_llist;
	}
	else if (check_llist == file_llist)
	{
	  new_llist->next = file_llist;
	  file_llist->prev = new_llist;
	  
	  file_llist = new_llist;
	}
	else
	{
	  new_llist->prev = check_llist->prev;
	  new_llist->next = check_llist;
	  
	  check_llist->prev->next = new_llist;
	  check_llist->prev = new_llist;
	}
      }
      else
      {
	last_llist->next = new_llist;
	new_llist->prev = last_llist;

	last_llist = new_llist;
      }
      
      (*num_files)++;
	
    } /* endif - file matches */
    
  } /* endwhile - readdir not NULL */

  /*
   * Fill the array to be returned to the user.
   */
  
  file_list = (char **)malloc(*num_files * sizeof(char *));
  for (i = 0, new_llist = file_llist;
       new_llist != NULL;
       new_llist = new_llist->next)
    file_list[i++] = new_llist->filename;
  
  /*
   * Free malloced memory.
   */

  free(exps);
  
  while (file_llist != NULL)
  {
    new_llist = file_llist;
    file_llist = file_llist->next;

    free(new_llist);

    /* Don't free the string pointers, they are returned to the caller. */

  } /* endwhile */
  
  return(file_list);
}



/***************************************************************************
 * REUTIL_shell_to_regexp
 *
 * Converts a shell-format regular expression to the format expected by
 * regexp.  Allocates space for the returned string, which must be freed
 * by the calling process.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1995
 *
 ****************************************************************************/

#include <stdlib.h>

char *REUTIL_shell_to_regexp(const char *shell_string)
{
  char *regexp_string;
  
  int  i, j;
  
  int  star_count = 0;
  int  dot_count = 0;
  
  for (i = 0; i < strlen(shell_string); i++)
  {
    if (shell_string[i] == '*')
      star_count++;
    else if (shell_string[i] == '.')
      dot_count++;
  }
  
  regexp_string = (char *)malloc(strlen(shell_string) +
				 star_count +
				 dot_count +
				 3);          /* EOS, '^', '$' */
  
  regexp_string[0] = '^';
  j = 1;
  for (i = 0; i < strlen(shell_string); i++)
  {
    if (shell_string[i] == '*')
    {
      regexp_string[j++] = '.';
      regexp_string[j++] = '*';
    }
    else if (shell_string[i] == '.')
    {
      regexp_string[j++] = '\\';
      regexp_string[j++] = '.';
    }
    else
      regexp_string[j++] = shell_string[i];
  }
  regexp_string[j++] = '$';
  regexp_string[j] = '\0';
  
  return(regexp_string);
}

