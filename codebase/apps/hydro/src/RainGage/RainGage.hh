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
/////////////////////////////////////////////////////////////
// RainGage.hh
//
// RainGage object
//
// Alex Baia, August 2000
//
// Code created from BasinPrecip.hh, Mike Dixon, August 1998
//
///////////////////////////////////////////////////////////////

#ifndef RainGage_H
#define RainGage_H


#include <tdrp/tdrp.h>
#include <didss/ds_input_path.h>
#include <didss/DsInputPath.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Args.hh"
#include "Params.hh"
using namespace std;


class RainGage
{

public:

  // constructor

  RainGage(int argc, char **argv);

  // destructor
  
  ~RainGage();

  // Run the RainGage program.
  //
  // Returns the return code for the program (0 if successful, error code
  // otherwise).

  int Run();

  // data members

  bool OK;


protected:
  

private:


  /////////////////////
  // Private members //
  /////////////////////

  // Program parameters

  char *_progName;
  char *_paramsPath;
  Args *_args;
  Params *_params;

  //static string tdf;


  /////////////////////
  // Private methods //
  /////////////////////


  void ParseGageTData();

};

#endif

