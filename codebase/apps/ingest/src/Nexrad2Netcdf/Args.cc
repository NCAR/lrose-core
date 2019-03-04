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
// Jason Craig July 2007
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
           "       [ -debug ]               produce debug messages\n" 
           "       [ -verbose ]             produce verbose debug messages\n" 
           "       [ -info ]                produce info messages\n" 
           "       [ -end \"yyyy/mm/dd hh:mm:ss\"] end time - "
                                            "START_END mode implied\n"

           "       [ -f file_paths]         list of input file paths\n"
           "                                overrides radar_input_url param\n"
           "                                FILE_LIST mode implied\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -instance name]      set the instance\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -start \"yyyy/mm/dd hh:mm:ss\"] start time - "
                                            "START_END mode implied\n"
           "       [ -summary [n]]          print summary each n records\n"
           "                                (default=90)\n"
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
      sprintf( paramVal, "debug = true;" );
      TDRP_add_override( &override, paramVal );
    }

    //
    // request for verbose debug messaging
    //
    else if ( !strcmp(argv[i], "-verbose" )) {
      sprintf( paramVal, "verbose = true;" );
      TDRP_add_override( &override, paramVal );
      sprintf( paramVal, "debug = true;" );
      TDRP_add_override( &override, paramVal );
    }

    //
    // request for info messaging
    //
    else if ( !strcmp(argv[i], "-info" )) {
      sprintf( paramVal, "info = true;" );
      TDRP_add_override( &override, paramVal );
    }
    
    
    //
    // request to print summary
    //
    else if ( !strcmp(argv[i], "-summary" )) {
      sprintf( paramVal, "printSummary = TRUE;" );
      TDRP_add_override( &override, paramVal );
      
      if (i < argc - 1) {
	sprintf( paramVal, "summaryInterval = %s;", argv[i+1] );
	TDRP_add_override( &override, paramVal );
      }
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
	TDRP_add_override( &override, "mode = START_END;" );
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
	TDRP_add_override( &override, "mode = START_END;" );
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
      TDRP_add_override( &override, "mode = FILE_LIST;" );
      
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
  if( params->mode == Params::START_END ) {
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
  
  if( params->mode == Params::FILE_LIST && inputFileList.size() <= 0 ) {
    cerr  << "ERROR: Must specify a file list on the command line " <<
             "for FILE_LIST mode" << endl;
    exit (-1);
  }

  return (0);
}
