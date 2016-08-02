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

// Grib2Mdv top-level application class
// $Id: Grib2Mdv.hh,v 1.6 2016/03/07 01:23:01 dixon Exp $


#ifndef _GRIB_2_MDV_INC
#define _GRIB_2_MDV_INC

// C++ include files
#include <string>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/Path.hh>

// Local include files
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class Args;
class DataMgr;

class Grib2Mdv {
 public:

  // instance -- create the Singleton
  static Grib2Mdv *instance(int argc, char **argv);
  static Grib2Mdv *instance(void);

   virtual ~Grib2Mdv();
   
   // isOK -- check on class
   bool isOK() const { return _isOK; }

   // getErrStr -- returns error message
   string getErrStr() const { return _errStr; }

   // run -- Execution of program
   bool run();
   
 private:

   bool _isOK;
   string _progName;
   string _errStr;
   Args *_args;
   Params *_params;

   static const string _className;

   // 
   // Initialization
   //
   Path _program;
   
   Grib2Mdv(); // hide constructor -- singleton
   Grib2Mdv(int argc, char **argv);
   Grib2Mdv(const Grib2Mdv &);
   Grib2Mdv &operator=(const Grib2Mdv &);

   //
   // Singleton instance pointer
   //
   static Grib2Mdv *_instance;  // singleton instance

   //
   // Processing
   //
   string _inputFileSuffix;
   DataMgr *_dataMgr;

};

//
// Prototypes for asyncrhronous handlers
//
//
// Prototypes for PORTscan select function
//
extern int fileSelect(const struct dirent *dirInfo);

#endif

