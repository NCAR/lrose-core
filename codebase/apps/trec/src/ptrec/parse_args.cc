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
 * parse_args.c: parse command line args
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "trec.h"

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_brief_p,
		int *print_params_full_p,
		tdrp_override_t *override,
		char **params_file_path_p)


{

  int error_flag = FALSE;
  int i, j;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s%s",
	  "Usage: ", Glob->prog_name, "[options as below]\n",
	  "      [ --, -h, -help, -man ] produce this list.\n"
	  "      [ -check_params ] check parameter usage\n"
	  "      [ -debug ] print debug messages\n"
          "      [ -dir input_directory ] Input directory for REALTIME\n"
          "      [ -encode encode_type ]\n"
          "                None (unsigned char) = 0, URL8 8bit encoding = 1\n"
	  "      [ -header [n] ] print header each n records (n default 360)\n"
          "      [ -i  instance_name ] Instance string (no blanks)\n"
          "      [ -if input_file_list ] (for ARCHIVE mode)\n"
          "      [ -mode operational_mode ] ARCHIVE or REALTIME\n"
          "      [ -noupdate ] do not update existing trec output files\n"
          "      [ -of output_file_dir ] top directory for output files\n"
	  "      [ -params params_file ] set params file name\n"
	  "      [ -print_params_brief ] brief parameter listing\n"
	  "      [ -print_params_full ] commented parameter listing\n"
          "      [ -sf spdb_destination ] full pathname\n"
	  "      [ -summary [n] ] print summary each n records (n default 90)\n"
	  "\n");

  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_brief_p = FALSE;
  *print_params_full_p = FALSE;
  Glob->input_file_list = NULL;
  Glob->n_input_files = 0;
  TDRP_init_override(override);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      fprintf(stderr, "%s\n", usage);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      *check_params_p = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dir")) {
      if (i < argc - 1) {             
         sprintf(tmp_str, "input_dir = \"%s\";",argv[i+1]);
         TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-encode")) {
      if (i < argc - 1 ) {
        sprintf(tmp_str, "output_encoding_type = %d;",atoi(argv[i+1]));
        TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-header")) {
      sprintf(tmp_str, "print_header = TRUE;");
      TDRP_add_override(override, tmp_str);
      if (i < argc - 1) {
	sprintf(tmp_str, "header_interval = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-i")) {
      if (i < argc - 1) {
         sprintf(tmp_str, "instance = \"%s\";",argv[i+1]);
         TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-if")) {
      if (i < argc - 1) {
         for (j = i+1; j < argc; j++)
            if (argv[j][0] == '-')
               break;
         Glob->n_input_files = j - i - 1;
         Glob->input_file_list = argv + i + 1;
      }
      else 
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-mode")) {
      if (i < argc - 1) {
         sprintf(tmp_str, "mode = %s;",argv[i+1]);
         TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-noupdate")) {
      sprintf(tmp_str, "noupdate = TRUE;");
      TDRP_add_override(override, tmp_str);

    } else if (!strcmp(argv[i], "-of")) {
      if (i < argc - 1) {
         sprintf(tmp_str, "output_dir = \"%s\";",argv[i+1]);
         TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-params")) {
      if (i < argc - 1) {
	*params_file_path_p = argv[i+1];
      } 
      else
         error_flag = TRUE;
	
    } else if (!strcmp(argv[i], "-print_params_brief")) {
      *print_params_brief_p = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params_full")) {
      *print_params_full_p = TRUE;
      
    } else if (!strcmp(argv[i], "-sf")) {
      if (i < argc - 1) {
         sprintf(tmp_str, "spdb_destination = \"%s\";",argv[i+1]);
         TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } else if (!strcmp(argv[i], "-summary")) {
      sprintf(tmp_str, "print_summary = TRUE;");
      TDRP_add_override(override, tmp_str);
      if (i < argc - 1) {
	sprintf(tmp_str, "summary_interval = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }
      else
         error_flag = TRUE;

    } /* if */
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag) {
    fprintf(stderr, "%s\n", usage);
    exit(-1);
  }
}
