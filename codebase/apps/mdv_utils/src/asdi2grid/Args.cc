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
// Args.cc
//
// Command line args
//
// Jason Craig August 2007
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <iostream>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
using namespace std;

void Args::usage(string prog_name)
{
   cerr << "Usage: " << prog_name << " [options as below]\n"
           "       [ -h, -help ]            produce this list\n"
           "       [ -verbose ]             produce verbose messages\n" 
           "       [ -debug ]               produce debug messages\n"
           "       [ -instance name]        set the instance\n"
           "       [ -f file_paths ]        list of input file paths\n"
           "       [ -idir output_dir ]     input file name\n"
           "       [ -odir output_dir ]     output file name\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -start \"yyyy/mm/dd hh:mm:ss\"] ARCHIVE mode only\n"
           "       [ -end   \"yyyy/mm/dd hh:mm:ss\"] ARCHIVE mode only\n"
        << endl;

   TDRP_usage( stdout );
}

int Args::parse(int argc, char **argv, string prog_name, Params *params)
{

  int iret = 0;
  char paramVal[256];

  // intialize

  TDRP_init_override(&override);
  startTime = DateTime::NEVER;
  endTime = DateTime::NEVER;

  //
  // Process each argument
  //
  for( int i=1; i < argc; i++ ) {
    //
    // request for usage information
    //
    if ( !strcmp(argv[i], "--" ) ||
	 !strcmp(argv[i], "-h" ) ||
	 !strcmp(argv[i], "-help" ) ||
	 !strcmp(argv[i], "-man" )) {
      usage(prog_name);
      exit (0);
    }
    
    //
    // request for debug messaging
    //
    else if ( !strcmp(argv[i], "-debug" )) {
      sprintf( paramVal, "debug_level = 2;" );
      TDRP_add_override( &override, paramVal );
    }
    
    //
    // request for verbose messaging
    //
    else if ( !strcmp(argv[i], "-verbose" )) {
      sprintf( paramVal, "debug_level = 1;" );
      TDRP_add_override( &override, paramVal );
    }
   
    //
    // instance specification
    //
    else if( !strcmp(argv[i], "-instance" )) {
      if( !( ++i <= argc ) ) {
	cerr << "ERROR: Instance name must follow -instance argument." << endl;
	exit( -1 );
      }
      sprintf( paramVal, "instance = %s;", argv[i] );
      TDRP_add_override( &override, paramVal );
    }

    //
    // input directory specification
    //
    else if( !strcmp(argv[i], "-idir" )) {
      if( !( ++i <= argc ) ) {
	cerr << "ERROR: Directory must follow -idir argument." << endl;
	exit( -1 );
      }
      sprintf( paramVal, "input_dir = %s;", argv[i] );
      TDRP_add_override( &override, paramVal );
    }

    //
    // output directory specification
    //
    else if( !strcmp(argv[i], "-odir" )) {
      if( !( ++i <= argc ) ) {
	cerr << "ERROR: Directory must follow -odir argument." << endl;
	exit( -1 );
      }
      sprintf( paramVal, "output_dir = %s;", argv[i] );
      TDRP_add_override( &override, paramVal );
    }

    //
    // start time specification
    //
    else if ( !strcmp(argv[i], "-start" )) {
      if (!( ++i <= argc )){
	cerr << "ERROR: Date must follow -start argument." << endl;
	exit(-1);
      }
      startTime = DateTime::parseDateTime( argv[i] );
      if ( startTime == DateTime::NEVER ) {
	cerr << "ERROR: Bad date/time syntax in -start specification." << endl;
	usage(prog_name);
	exit(-1);
      }
      else {
	TDRP_add_override( &override, "run_mode = ARCHIVE_START_END_TIMES;" );
      }
    }
        
    //
    // end time specification
    //
    else if ( !strcmp(argv[i], "-end" )) {
      if (!( ++i < argc )){
	cerr << "ERROR: Date must follow -end argument." << endl;
	exit(-1);
      }
      endTime = DateTime::parseDateTime( argv[i] );
      if ( endTime == DateTime::NEVER ) {
	cerr << "ERROR: Bad date/time syntax in -end specification." << endl;
	usage(prog_name);
	exit(-1);
      }
      else {
	TDRP_add_override( &override, "run_mode = ARCHIVE_START_END_TIMES;" );
      }
    }

    //
    // file list specification
    //
    else if ( !strcmp(argv[i], "-f" )) {
      
      //
      // search for next arg which starts with '-'
      //
      int j;
      for( j = i+1; j < argc; j++ ) {
	if (argv[j][0] == '-')
	  break;
	else
	  inputFileList.push_back( argv[j] );
      }
      TDRP_add_override( &override, "run_mode = ARCHIVE_FILE_LIST;" );
      
      if ( inputFileList.size() == 0 ) {
	cerr << "ERROR: Missing file list specification." << endl;
	usage(prog_name);
	exit(-1);
      }
    } 
  }
    
  // Load Params
  if (params->loadFromArgs(argc, argv, override.list, &paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 
  
  //
  // Check modes
  //
  if( params->run_mode == Params::ARCHIVE_START_END_TIMES ) {
    if ( startTime == DateTime::NEVER || endTime == DateTime::NEVER ) {
      cerr << "ERROR: Must specify a start and end time on " <<
	      "the command line for START_END mode" << endl;
      usage(prog_name);
      exit (-1);
    }
    
    if( endTime < startTime ) {
      cerr << "ERROR: Start and end times do not make sense" << endl;
      exit (-1);
    }
  }
  
  if( params->run_mode == Params::ARCHIVE_FILE_LIST && inputFileList.size() <= 0 ) {
    cerr  << "ERROR: Must specify a file list on the command line " <<
             "for FILE_LIST mode" << endl;
    exit (-1);
  }

  return (0);
}
