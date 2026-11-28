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
/////////////////////////////////////////////////////////
// gfsIngest top-level application class
//
//
////////////////////////////////////////////////////////
#ifndef _GFS_INGEST_INC
#define _GFS_INGEST_INC

#include <string>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include "Params.hh"
#include "Args.hh"
using namespace std;

//
// Forward class declarations
//
class Grib2Mdv;

class gfsIngest {
 public:

  // instance -- create the Singleton
  static gfsIngest *Inst(int argc, char **argv);
  static gfsIngest *Inst();

   ~gfsIngest();
   
   //
   // Initialization
   //
   int init( int argc, char**argv );


   // Flag indicating whether the program status is currently okay.

   bool okay;

   //
   // Execution
   //
   int run();
   
 private:

   // 
   // Initialization
   //
   Path _program;
   
   // Constructor -- private because this is a singleton object
   gfsIngest(int argc, char **argv);

   void _usage();
   int _processArgs( int argc, char **argv,
		     tdrp_override_t& override ,
		     int* nFiles, char*** fileList );
   
   //
   // Singleton instance pointer
   //
   static gfsIngest *_instance;  // singleton instance

   //
   // Parameter processing
   //
   char   *_paramsPath;
   int _processParams( int nFiles, char** fileList );

   //
   // Processing
   //
   string _inputFileSuffix;
   Grib2Mdv *_grib2Mdv;

   // Program parameters.

   char *_progName;
   Args *_args;
   Params *_params;

   int _nfiles;
   char *_flist;

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("gfsIngest");
  }


};

//
// Defines for success and failure returns
//
#define RI_FAILURE -1
#define RI_SUCCESS 0

//
// Prototypes for asyncrhronous handlers
//
//
// Prototypes for PORTscan select function
//
extern int fileSelect(const struct dirent *dirInfo);

#endif








