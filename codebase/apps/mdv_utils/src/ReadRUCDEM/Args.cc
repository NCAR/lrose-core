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
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//
/////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  checkParams = FALSE;
  printParams = FALSE;
  printShort = FALSE;
  paramsFilePath = NULL;
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, stdout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      checkParams = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params")) {
      
      printParams = TRUE;
      
    } else if (!strcmp(argv[i], "-print_short")) {
      
      printShort = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {

      if (i<argc - 1){
	sprintf(tmp_str, "debug = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = 0;
      }

    } else if (!strcmp(argv[i], "-params")) {
	
      if (i < argc - 1) {
	paramsFilePath = argv[++i]; // Was argv[i+1] - Niles.
      } else {
	OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-Nx")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "Nx = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	

    } else if (!strcmp(argv[i], "-Ny")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "Ny = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-dx")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "dx = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	

    } else if (!strcmp(argv[i], "-dy")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "dy = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-lon")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "lon_origin = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-lat")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "lat_origin = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
    } else if (!strcmp(argv[i], "-AllowOutside")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "AllowOutsideDEM = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-flat")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "flat = %s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-BaseName")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "BaseName=%s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }

    } else if (!strcmp(argv[i], "-OutName")) {
	
      if (i < argc - 1) {
	sprintf(tmp_str, "OutName=%s;",argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }


    } else { // Unrecognised arg.

      usage(prog_name, stderr);
      exit(-1);

    } // if
    
  } // i

  if (!OK) {
    usage(prog_name, stderr);
    exit(-1);
  }
    
}

void Args::usage(char *prog_name, FILE *out)
{

  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug N] print debug messages if N <> 0 [0]\n"
	  "       [ -params ?] params file path\n"
	  "       [ -print_params ] print parameters with comments\n"
	  "       [ -print_short ] print parameters - short version\n"
	  "       [ -Nx n ] Set Nx [150]\n"
	  "       [ -Ny n ] Set Ny [150]\n"
	  "       [ -dx q ] Set dx [0.03]\n"
	  "       [ -dy q ] Set dy [0.03]\n"
	  "       [ -lat q ] Set latitude origin [40.5]\n"
	  "       [ -lon q ] Set longitude origin [-75.0]\n"
	  "       [ -flat n ] Assume flat earth if flat <> 0 [0]\n"
	  "       [ -BaseName ? ] Set base name of DEM files, eg. E100N40 [W100N90]\n"
	  "       [ -OutName ? ] Set output filename, eg. taiwan.mdv [Terrain.mdv]\n"
	  "       [ -AllowOutsideDEM n ] If n <> 0 use the bad data value if outside input DEM grid [0]\n"
	  "\n");


}






