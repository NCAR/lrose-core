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
 *  @version  $Id: Args.cc,v 1.8 2016/03/04 02:22:11 dixon Exp $
 */


// System/RAP include files
#include "os_config.h"
#include "toolsa/udatetime.h"
#include "toolsa/str.h"
#include "toolsa/umisc.h"


// Local include files
#include "Args.hh"
#include "Params.hh"

using namespace std;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Args::Args(int argc, char **argv, const string& progName) : 
  outFile(""),reportFile(""), fieldInfo(), _isOK(true)
//field(""), sig_diff(0.01), rms_sig_diff(0.1), _isOK(true)
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
	
		    _isOK = false;
	    } // endif -- i < argc - 1

  
    } 
    else if(STRequal_exact(argv[i], "-file1")) {
	      if(i < argc - 1) {
		      file1 = argv[i+1];
	      }
	      else{
		      cerr << "User did not specify argument for file1";
		      _isOK = false;
	      }
    } 
    else if(STRequal_exact(argv[i], "-file2")) {
	      if(i < argc - 1) {
		      file2 = argv[i+1];
	      }
	      else{
		      cerr << "User did not specify argument for file2";
		      _isOK = false;
	      }
    } 
    else if(STRequal_exact(argv[i], "-out")) {
	      if(i < argc - 1) {
		      outFile = argv[i+1];
	      }
	      else{
		      cerr << "User did not specify argument for out";
		      _isOK = false;
	      }
    } 

    else if(STRequal_exact(argv[i], "-fci")) {
	      if(i < argc - 1) {
		fieldInfo.push_back(argv[i+1]);
	      }
    } 
    else if(STRequal_exact(argv[i], "-reportFile")) {
	      if(i < argc - 1) {
		      reportFile = argv[i+1];
	      }
    } 

  } // endfor -- int i =  1; i < argc; i++

  if (file1 == "" || file2 == ""){
	  cerr << "Missing file1 or file2 argument.\n";
	  _isOK = false;
  }


  if(!_isOK) {
    _usage( progName, cerr );
  } // endif -- !_isOK
    
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
  out << "Usage: " << progName << " -file1 f1 -file2 f2 [options as below]\n"
      << "      f1 and f2 are full paths to files to be compared.\n"
      << "options:\n"
      << "      [ --, -help, -h, -man ] produce this list.\n"
      << "      [ -debug ] debugging on\n"
      << "      [ -verbose ] verbose debugging on\n"
      << "      [-i instance_name] Instance string (no blanks)\n"
      << "      [-out out] Full path to output file (difference fields)\n"
      << "      [-fci field,sigDiff,sigDiffRMS] Field info to do comaprisons on. (overrides params)\n"
      << "        field = name of field to compare\n"
      << "        sigDiff = difference required between two points to be considered significant. (overrides params) (default = 0.01)\n"
      << "        sigDiffRMS = RMS difference required between two vertical levels to be considered significang. (overrides params) (default = 0.1)\n"
      << "      [-reportFile reportFile] Full path to report file (otherwise stderr)\n"
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



