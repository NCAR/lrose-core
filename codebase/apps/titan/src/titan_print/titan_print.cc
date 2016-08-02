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
/******************************************************************************
 * titan_print.c :
 *
 * prints view on various types of file to stdout
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * December 1990
 *
 *******************************************************************************/

#define MAIN
#include "titan_print.h"
#undef MAIN
#include <titan/zr.h>

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *file_label;
  path_parts_t progname_parts;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse command line arguments
   */

  parse_args(argc, argv);

  /*
   * read the file label
   */

  if (RfReadFileLabel(Glob->file_name, &file_label) != R_SUCCESS)
    exit(1);
  
  /*
   * show info for given file type
   */

  if (!strcmp(file_label, RADAR_VOLUME_FILE_TYPE)) {

    volume_view();

  } else if (!strcmp(file_label, STORM_HEADER_FILE_TYPE)) {

    storm_view();

  } else if (!strcmp(file_label, TRACK_HEADER_FILE_TYPE)) {

    track_view();

  } else if (!strcmp(file_label, RADAR_TO_CART_TABLE_FILE)) {

    rc_table_view();

  } else if (!strcmp(file_label, RADAR_TO_CART_SLAVE_TABLE_FILE)) {

    slave_table_view();

  } else if (!strcmp(file_label, CLUTTER_TABLE_FILE)) {

    clutter_view();

  } else if (!strcmp(file_label, ZR_FILE_TYPE)) {

    RfPrintZrFile(Glob->file_name);
    
  } else {

    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "File '%s' not recognized type.\n",
	    Glob->file_name);
    exit(1);

  }

  umalloc_verify();
  umalloc_map();

  return(0);

}
