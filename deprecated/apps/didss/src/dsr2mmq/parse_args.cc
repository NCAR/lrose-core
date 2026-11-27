// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * parse_args.c: parse command line args, open files as required
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1997
 *
 * Mike Dixon/Jaimi Yee
 *
 *********************************************************************/

#include "dsr2mmq.h"
using namespace std;

void parse_args(int argc,
		char **argv,
                int *check_params_p,
                int *print_params_p,
                tdrp_override_t *override,
                char **params_file_path_p)

{

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  int error_flag = 0;
  int i;
 
  /*
   * set defaults
   */
  
  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);

  /*
   * set usage
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", argv[0], "\n",
	  "       [--, -h, -help, -man] produce this list.\n"
          "       [-debug] print debug messages\n"
          "       [-check_params] check parameter usage\n"
	  "       [-params ?] set parameters file name\n"
          "       [-print_params] print parameter usage\n"
	  "\n");

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") || !strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") || !strcmp(argv[i], "-man")) {

      fprintf(stderr, "%s", usage);
      tidy_and_exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
       
       sprintf(tmp_str, "debug = TRUE;");
       TDRP_add_override(override, tmp_str);
       
    } else if (!strcmp(argv[i], "-check_params")){

      *check_params_p = TRUE;

    }  else if (!strcmp(argv[i], "-params")) {

      if (i < argc - 1) {
        *params_file_path_p = argv[i+1];
      } else {
        error_flag = TRUE;
      }
	
    } else if (!strcmp(argv[i], "-print_params")) {

       *print_params_p = TRUE;
      
    } /* if */

  } /* i */


  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s", usage);
  }

  if (error_flag)
    tidy_and_exit(-1);

}
