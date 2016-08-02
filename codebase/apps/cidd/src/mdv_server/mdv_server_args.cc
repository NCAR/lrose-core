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
 * MDV_SERVER_ARGS
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_ARGS

#include "mdv_server.h"

#define ARG_OPTION_STRING   "o:p:s:i:n:d:f:h:H:F:rclub"
/*****************************************************************
 * PROCESS_ARGS: Progess command line arguments. Set option flags
 *     And print usage info if necessary
 */

void process_args(int argc, char *argv[])
{
  int err_flag =0;
  int  c;
  extern  char *optarg;   /* option argument string */

  /*
   * Initialize the application name first since I don't know
   * if the getopt stuff corrupts the argv pointers and I don't
   * feel like testing it.
   */

  STRcopy(gd.app_name, argv[0], PROCMAP_NAME_MAX);

  /*
   * Initialize the global fields that could be changed by
   * the command line arguments.
   */

  gd.port = MDV_SERVER_PORT;
  STRcopy(gd.app_instance, MDV_SERVICE_INSTANCE, SERVMAP_INSTANCE_MAX);
  STRcopy(gd.servmap_host1, SERVMAP_HOST1, SERVMAP_HOST_MAX);
  STRcopy(gd.servmap_host2, SERVMAP_HOST2, SERVMAP_HOST_MAX);

  STRcopy(gd.data_file_suffix, DATA_FILE_SUFFIX, MAX_SUFFIX_LEN);
  gd.reg= FALSE;
  gd.compress_flag= FALSE;
  gd.daemonize_flag = TRUE;
  
  gd.override_origin = 0;  
  gd.num_children = 0;  
  gd.live_update = 0;   
  gd.num_top_dirs = 0;
  gd.input_filename = NULL;
  
  while ((c = getopt(argc, argv,ARG_OPTION_STRING)) != EOF)
  {

    switch(c)
    {
    case 'b':    /* Run as a "background" process */
      gd.daemonize_flag = FALSE;
      printf("%s: Running as background process - NOT daemonizing\n",
	     gd.app_name);
      break;
      
    case 'c':    /* Compress the data before transfer to client */
      gd.compress_flag = TRUE;
      printf("%s: Compressing Outgoing data\n", gd.app_name);
      break;

    case 'd' :  /* directory storing data */
      gd.top_dir[gd.num_top_dirs] = STRdup(optarg);
      gd.top_dir_url[gd.num_top_dirs] = (char *) 
         calloc(1,strlen(gd.top_dir[gd.num_top_dirs]) + 1024);

      printf("%s: Using additional directory: %s\n",
	     gd.app_name, gd.top_dir[gd.num_top_dirs]);
      gd.num_top_dirs++;
      break;

    case 'f' :    /* input file - used when input is always from the */
                  /* same file name */
      gd.input_filename = STRdup(optarg);
      printf("%s: Using input file: %s\n",
	     gd.app_name, gd.input_filename);
      break;
			 
    case 'h' :  /* alternate servermapper host1 */
      STRcopy(gd.servmap_host1, optarg, SERVMAP_HOST_MAX);
      break;

    case 'H' :  /* alternate service name */
      STRcopy(gd.servmap_host2, optarg, SERVMAP_HOST_MAX);
      break;

    case 'F' :  /* Number of children to fork */
      gd.num_children = atoi(optarg);
      break;

    case 'i' :  /* alternate service instance name */
      STRcopy(gd.app_instance, optarg, SERVMAP_INSTANCE_MAX);
      break;

    case 'l':    /* register this server as "live" with the servermapper */
      gd.live_update = TRUE;
      break;

    case 'n' :  /* alternate service subtype name */
      STRcopy(gd.service_subtype, optarg, SERVMAP_NAME_MAX);
      break;

    case 'o' :  /* override file's data origin */
      if (sscanf(optarg, "%lf,%lf,%lf",
		 &gd.origin_lat,
		 &gd.origin_lon,
		 &gd.origin_alt) == 3)
      {
	gd.override_origin = TRUE;
      }
      break;

    case 'p' :  /* alternate parameter database file */
      gd.port = atoi(optarg);
      break;

    case 'r':   /* register this server with the servermapper and proc mapper */
      gd.reg = TRUE;
      break;

    case 's' :  /* alternate data file extension */
      STRcopy(gd.data_file_suffix, optarg, MAX_SUFFIX_LEN);
      break;

    case 'u':   /* indicate live/ updating data with the servermapper */
      gd.live_update = TRUE;
      break;

    case '?':   /* error in options */
    default:
      err_flag++;
      break;
    }

  };    

  if(gd.num_top_dirs <= 0) {
      err_flag++;
      fprintf(stderr,"No Top level directory specified \n");
  }

  /*
   * Inform the user of any command line errors.
   */

  if (err_flag)
  {
    fprintf(stderr,"Usage:%s [options]\n",argv[0]);
    fprintf(stderr,"                   [-b ] (Run as background process - DON'T daemonize)\n");
    fprintf(stderr,"                   [-F N] Fork N children to handle load\n");
    fprintf(stderr,"                   [-c ] (Run Length Encode the data for Xfer across the net)\n");
    fprintf(stderr,"                   [-d alternate data directory] \n");
    fprintf(stderr,"                   [-f input filename, overrides -d] \n");
    fprintf(stderr,"                   [-h alternate server mapper primary host] \n");
    fprintf(stderr,"                   [-H alternate server mapper secondary host] \n");
    fprintf(stderr,"                   [-i alternate service instance] \n");
    fprintf(stderr,"                   [-l ] (Register this server as live with servermapper) \n");
    fprintf(stderr,"                   [-n alternate service subtype] \n");
    fprintf(stderr,"                   [-p alternate port] \n");
    fprintf(stderr,"                   [-o origin_lat,lon,altitude] \n");
    fprintf(stderr,"                   [-r ] (Register with server and proc mappers) \n");
    fprintf(stderr,"                   [-s file name suffix] \n");
    fprintf(stderr,"                   [-u ] (Register this server as live with servermapper) \n");

    exit(-1);
  }

  fprintf(stderr,"Running on port %d\n",gd.port);

  /*
   * Initialize the global fields that aren't affected by command
   * line arguments, or that are affected indirectly.
   */

  STRcopy(gd.service_subtype, MDV_SERVICE_NAME, SERVMAP_NAME_MAX);

  return;
}     
