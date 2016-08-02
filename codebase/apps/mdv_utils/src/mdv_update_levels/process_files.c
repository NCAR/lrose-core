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
 * process_files.c
 *
 * Gets a list of mdv files in a directory
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * February 1999
 *
 **********************************************************************/

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <toolsa/str.h>
#include <toolsa/dlm.h>

#include "mdv_update_levels.h"


/*
 * prototypes
 */
static void update_files(DLMlist *file_list, char *subdir);
static int alpha_compare(void *str1, void *str2);

void process_files()
{
  DIR *dir_ptr;
  DIR *subdir_ptr;

  DLMlist *subdir_list;
  DLMlist *file_list;

  struct dirent *dir_info;
  struct dirent *subdir_info;
  struct stat subdir_stat;

  char subdir_name[SUBDIR_LEN];
  char *subdir;

  if ((dir_ptr = opendir(Glob->params.input_dir)) == (DIR *)NULL)
  {
     fprintf(stderr, "ERROR:  Couldn't open directory %s\n", 
             Glob->params.input_dir);
     tidy_and_exit(-1);
  }

  subdir_list = DLMcreateList(SUBDIR_LEN);

  
  while((dir_info = readdir(dir_ptr)) != NULL)
  {

     if (STRequal_exact(dir_info->d_name, ".") ||
	 STRequal_exact(dir_info->d_name, ".."))
	 continue;

     sprintf(subdir_name, "%s/%s", Glob->params.input_dir, dir_info->d_name);

     if (stat(subdir_name, &subdir_stat) != 0)
     {
	fprintf(stderr, "ERROR:  process_files\n");
	fprintf(stderr, "Could not stat subdirectory %s\n", subdir_name);
	tidy_and_exit(-1);
     }

     if (S_ISDIR(subdir_stat.st_mode) && strlen(dir_info->d_name) == 8)
     {
	DLMinsert(subdir_list, (void *) dir_info->d_name, alpha_compare);
     }
  }

  closedir(dir_ptr);

  subdir = (char *)DLMfirst(subdir_list);
  while (subdir != NULL)
  {
     sprintf( subdir_name, "%s/%s", Glob->params.input_dir, subdir );

     if ((subdir_ptr = opendir(subdir_name)) == (DIR *)NULL)
     {
       fprintf(stderr, "ERROR:  Couldn't open directory %s\n", subdir_name);
       tidy_and_exit(-1);
     }

     file_list = DLMcreateList(FILE_LEN);

     while((subdir_info = readdir(subdir_ptr)) != NULL)
     {
	
       if (strstr(subdir_info->d_name, Glob->params.file_ext) == NULL)
          continue;
   
       DLMinsert(file_list, (void *) subdir_info->d_name, alpha_compare);
     }

     closedir(subdir_ptr);

     update_files(file_list, subdir);

     DLMdestroyList(file_list);

     subdir = (char *)DLMnext(subdir_list);
       
  }
}

/*******************************************
 * sample_files()
 *
 * Processes files in a file list.
 ******************************************/

void update_files(DLMlist *file_list, char *subdir)
{
   char *file_name;

   file_name = (char *)DLMfirst(file_list);
   if(Glob->params.debug)
      fprintf(stderr, "processing file %s/%s/%s\n", 
              Glob->params.input_dir, subdir, file_name);

   while (file_name != NULL)
   {
      create_file(file_name, subdir);

      file_name = (char *)DLMnext(file_list);
      if(Glob->params.debug && file_name != NULL)
         fprintf(stderr, "processing file %s/%s/%s\n",
                 Glob->params.input_dir, subdir, file_name);
   }

   
}

/**********************************************
 * alpha_compare()
 * 
 * Comparison function used by DLM_insert.
 * Returns true if the first string comes
 * before the second string alphabetically.
 * Otherwise it returns false.
 **********************************************/
int alpha_compare(void *str1, void *str2)
{
  if(strcmp((char *)str1, (char *)str2) < 0)
    return(TRUE);
  return(FALSE);
}
