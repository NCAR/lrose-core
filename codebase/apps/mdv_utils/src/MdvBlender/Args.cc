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
/**
 *
 *  @file Args.cc
 *
 *  @class Args
 *
 *  handles command line arguments
 *
 *  @author P. Prestopnik
 * 
 *  @date July 2014
 *
 *  @version  $Id: Args.cc,v 1.4 2016/03/04 02:22:10 dixon Exp $
 */


// System/RAP include files
#include "toolsa/udatetime.h"
#include "toolsa/str.h"
#include "toolsa/umisc.h"


// Local include files
#include "Args.hh"
#include "Params.hh"
#include "Constants.hh"

using namespace std;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Args::Args(int argc, char **argv, const string& progName) : 
  isOK(true),
  _archiveStartTime(0),
  _archiveEndTime(0)
{

  TDRP_init_override(&override);

  // search for command options
  
  for(int i =  1; i < argc; i++) {

    if(STRequal_exact( argv[i], "--") ||
       STRequal_exact( argv[i], "-h") ||
       STRequal_exact( argv[i], "-help") ||
       STRequal_exact( argv[i], "--help") ||
       STRequal_exact( argv[i], "-man")) {
      _usage( progName, cout );
      exit(0);

    } 
    else if(STRequal_exact( argv[i], "-debug")) {

      _setOverride( "debug = DEBUG_NORM;" );

    } 
    else if(STRequal_exact( argv[i], "-verbose")) {

      _setOverride( "debug = DEBUG_VERBOSE;" );

    } 
    else if(STRequal_exact(argv[i], "-i")) {

      if(i < argc - 1) {
	_setOverride( "instance", argv[i+1] );
      }
      else {
	cerr << "User must provide instance name with -i option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1

    } 
    else if(STRequal_exact(argv[i], "-mode")) {

      if(i < argc - 1) {
	      _setOverride( "run_mode", argv[i+1] );
      }
      else {
	cerr << "User must provide REALTIME, ARCHIVE, FILELIST or GRIDPOINT with -mode option" <<
	  endl;
	
	isOK = false;
      } // endif -- i < argc - 1

    } // endif -- STRequal_exact(argv[i], "-mode")
    else if(STRequal_exact(argv[i], "-out_url")) {
      if(i < argc - 1) {
	_setOverride( "output_url", argv[i+1] );
      }
      else {
	cerr << "User must provide directory name with -out_url option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1
    } // endif -- STRequal_exact(argv[i], "-out_url")
    else if(STRequal_exact(argv[i], "-start")) {
      if(i < argc - 1) {
	_setOverride( "start_time",  argv[i+1]);
	_setOverride( "run_mode = ARCHIVE;" );
      }
      else {
	cerr << "User must provide timestamp 'yyyy mm dd hh mm ss' with -start option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1
    } // endif -- STRequal_exact(argv[i], "-start")
    else if(STRequal_exact(argv[i], "-end")) {
      if(i < argc - 1) {
	_setOverride( "end_time",  argv[i+1]);
	_setOverride( "run_mode = ARCHIVE;" );
      }
      else {
	cerr << "User must provide timestamp 'yyyy mm dd hh mm ss' with -start option" << endl;
	
	isOK = false;
      } // endif -- i < argc - 1
    } // endif -- STRequal_exact(argv[i], "-start")
    else if ( STRequal_exact( argv[i], "-run_time" ) ) {
      if ( i < argc - 1 ) {
	_setOverride("run_time", argv[i+1]);
	_setOverride( "run_mode = RUNTIME;" );
      }
      else {
	cerr << "User must provide timestamp 'yyyy mm dd hh mm ss' with -run_time" << endl;
	isOK = false;
      }
    }
  } // endfor -- int i =  1; i < argc; i++

  if(!isOK) {
    _usage( progName, cerr );
  } // endif -- !isOK
    
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Args::~Args()
{
  TDRP_free_override(&override);
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// _convertTimeString() - Convert a time string entered on the command
//                        line to a UNIX time value.
//
// Returns the converted time value if successful, 0 if unsuccessful.
//

time_t 
Args::_convertTimeString( const char *timeString )
{
  date_time_t timeStruct;
  
  if(sscanf(timeString, "%d %d %d %d %d %d",
	    &timeStruct.year, &timeStruct.month, &timeStruct.day,
	    &timeStruct.hour, &timeStruct.min, &timeStruct.sec) != 6) {
    return 0;
  }
  else {
    uconvert_to_utime(&timeStruct);
    return timeStruct.unix_time;
  } // endif -- (sscanf(timeString ...

}


///////////////////////////////////////////////////////////////////////
// _usage() - Print the usage for this program.
//

void Args::_usage( const string& progName, ostream &out )
{
  out << "Usage: " << progName << " [options as below]\n"
      << "options:\n"
      << "      [ --, -help, -h, -man ] produce this list.\n"
      << "      [ -debug ] debugging on\n"
      << "      [ -verbose ] verbose debugging on\n"
      << "      [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "        ARCHIVE mode only\n"
      << "      [-i instance_name] Instance string (no blanks)\n"
      << "      [-mode operational_mode] ARCHIVE, REALTIME or FILELIST\n"
      << "      [-out_url output_url] URL for output files\n"
       << "      [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "        ARCHIVE mode only\n"
      << "      [ -run_time \"yyyy mm dd hh mm ss\"] single run time\n"
      << "        ARCHIVE mode only\n"
       << endl << endl;

  Params::usage(out);
}

//////////////////////////////////////////////////
// _setOverride
//
// this function sets TDRP overrides
//
void Args::_setOverride( const string& ovStr )
{
  TDRP_add_override( &override, (char *)ovStr.c_str() );
}

void Args::_setOverride( const string& ovStr, const char* val_str )
{
  string tmpStr = ovStr + " = \"" + val_str + "\";";
  TDRP_add_override(&override, (char *)tmpStr.c_str());
}



