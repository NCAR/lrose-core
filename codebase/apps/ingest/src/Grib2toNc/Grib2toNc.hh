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
/////////////////////////////////////////////////////////////////////
// Grib2toNc top-level application class
//
// Converts Grib2 files into Netcdf format
//
// -Jason Craig-  Jan 2015
////////////////////////////////////////////////////////////////////
#ifndef _GRIB2_TO_NC_HH
#define _GRIB2_TO_NC_HH

#include <string>
#include <tdrp/tdrp.h>

#include "Params.hh"
#include "Args.hh"
using namespace std;

#define Grib2toNc_VERSION 1.22

//
// Defines for success and failure returns
//
#define RI_FAILURE -1
#define RI_SUCCESS 0

//
// Forward class declarations
//
class Grib2Nc;

class Grib2toNc {
 public:

  // instance -- create the Singleton
  static Grib2toNc *Inst(int argc, char **argv);
  static Grib2toNc *Inst();

   ~Grib2toNc();
   
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
   
   // Constructor -- private because this is a singleton object
   Grib2toNc(int argc, char **argv);

  // Copyright display
   void _ucopyright(const char *prog_name);

   //
   // Singleton instance pointer
   //
   static Grib2toNc *_instance;  // singleton instance

   Grib2Nc *_grib2Nc;

   // Program parameters.
   char _progName[10];
   Args *_args;
   Params *_params;

};

#endif



