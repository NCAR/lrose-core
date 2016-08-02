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
 * parse_args.cc
 *
 * Parse command line arguments.  Create a TDRP override list from
 * the command line arguments, as needed.  Note that not all the
 * parameters are modifiable from the command line argument list.
 * User may opt for params file.
 * See paramdef.mdv2gint for complete parameter listing.
 *
 * Rachel Ames, RAP, NCAR, Boulder CO, January, 1996.
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "cartsim2mdv.h"
using namespace std;

void parse_args(int argc, char **argv,
                tdrp_override_t *override,
                char **params_file_name_p,
                int *n_input_files_p, 
                char ***input_file_list)

{
   int i;
   int j;
   int error_flag = 0;
   char usage[BUFSIZ];
   char tmp_str[BUFSIZ];
  
/* set usage */

   sprintf(usage, "%s%s%s",
          "Usage: ", argv[0], "\n"
          "      [--, -help, -man] produce this list.\n"
          "      [-alt altitude] Altitude at origin of sensor \n"
          "      [-check_params] check parameter usage\n"
          "      [-ct data_collection_type] MEASURED = 0, EXTRAPOLTAED = 1, FORECAST = 3\n"
          "      [-debug] print debug messages\n"
          "      [-dir input_directory (for REALTIME mode) \n"
          "      [-encode encode_type] None (unsigned char) = 0 , URL8 8bit encoding = 1 \n"
          "      [-if input_file_list (for ARCHIVE mode) \n"
          "      [-lat latitude] Latitude at origin of sensor \n"
          "      [-lon longitude] Longitude at origin of sensor \n"
          "      [-mode operational_mode] ARCHIVE or REALTIME \n"
          "      [-of output_file_dir] top directory for output (mdv) files \n"
          "      [-params params_file]  set parameters file name\n"
          "      [-print_params ] print parameter usage \n"
          "      [-proj projection_type] NATIVE = -1, LAT/LON = 0 , FLAT = 8 \n"
          "\n");

/* initialize */

   TDRP_init_override(override);
   
/* process for command options */

   for (i =  1; i < argc; i++) {

      if (!strcmp(argv[i], "--")    || !strcmp(argv[i], "-h") ||
          !strcmp(argv[i], "-help") || !strcmp(argv[i], "-usage") ||
          !strcmp(argv[i], "-man")) {
            fprintf(stderr, "%s", usage);
            exit(0);
      }  /* endif usage */


      else if (!strcmp(argv[i], "-alt")) {

         sprintf(tmp_str, "sensor_altitude = %f;",atof(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif altitude */

      else if (!strcmp(argv[i], "-check_params")) {

         sprintf(tmp_str, "check_params = TRUE;");
         TDRP_add_override(override, tmp_str);

      }  /* endif check_params */

      else if (!strcmp(argv[i], "-ct")) {

         sprintf(tmp_str, "collection_type = %d;",atoi(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif collection_type */

      else if (!strcmp(argv[i], "-debug")) {

         sprintf(tmp_str, "debug = TRUE;");
         TDRP_add_override(override, tmp_str);

      }  /* endif debug */

      else if (!strcmp(argv[i], "-dir")) {
 
         if (i < argc - 1) {
            sprintf(tmp_str, "input_dir = \"%s\";",argv[i+1]);
            TDRP_add_override(override, tmp_str);
         }  /* endif enough args */
         else {
            fprintf(stderr,"\nError. User must provide directory name for input");
            error_flag = TRUE;
         }
 
      }  /* endif input_dir */

      else if (!strcmp(argv[i], "-encode")) {

         sprintf(tmp_str, "output_encoding_type = %d;",atoi(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif output_encoding_type */

      else if (!strcmp(argv[i], "-if")) {

         if (i < argc - 1) {

            for (j = i+1; j < argc; j++)
               if (argv[j][0] == '-')
                  break;

            *n_input_files_p = j - i - 1;

            *input_file_list = argv + i + 1;

         }

         else {
            fprintf(stderr,"\nError. User must provide input (gint) file name");
            error_flag = TRUE;
         }

      }  /* endif input_file */

      else if (!strcmp(argv[i], "-lat")) {

         sprintf(tmp_str, "sensor_latitude = %f;",atof(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif latitude */

      else if (!strcmp(argv[i], "-lon")) {

         sprintf(tmp_str, "sensor_longitude = %f;",atof(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif longitude */

      else if (!strcmp(argv[i], "-mode")) {

         if (i < argc - 1) {   
            sprintf(tmp_str, "mode = %s;",argv[i+1]);
            TDRP_add_override(override, tmp_str);
         }  /* endif enough args */
         else {
            fprintf(stderr,"\nError. User must specify ARCIHVE or REALTIME mode");
            error_flag = TRUE;
         }

      }  /* endif mode */

      else if (!strcmp(argv[i], "-of")) {

         if (i < argc - 1) {   
            sprintf(tmp_str, "output_dir = \"%s\";",argv[i+1]);
            TDRP_add_override(override, tmp_str);
         }  /* endif enough args */
         else {
            fprintf(stderr,"\nError. User must provide to directory name for output (mdv) files");
            error_flag = TRUE;
         }

      }  /* endif output_file_dir */

      else if (!strcmp(argv[i], "-params"))  {

         if (i < argc - 1)  {
            *params_file_name_p = argv[i+1];
         }
         else  {
            fprintf(stderr,"\nError. User must provide parameter file name");
            error_flag = TRUE;
         }

      }  /* endif params */

      else if (!strcmp(argv[i], "-print_params")) {

         sprintf(tmp_str, "print_params = TRUE;");
         TDRP_add_override(override, tmp_str);

      }  /* endif  print_params */

      else if (!strcmp(argv[i], "-proj")) {

         sprintf(tmp_str, "projection_type = %d;",atoi(argv[i+1]));
         TDRP_add_override(override, tmp_str);

      }  /* endif projection_type */

     
   }   /* end processing arguments loop */

/* print message if warning or error flag set */

   if(error_flag)  {
      fprintf(stderr, "%s", usage);
      fprintf(stderr, "Check the parameters file '%s'.\n\n", *params_file_name_p);
      fprintf(stderr, "something went wrong.  \n");
   }

   return;
}

#ifdef __cplusplus
}
#endif
