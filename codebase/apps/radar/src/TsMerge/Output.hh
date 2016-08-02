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
// Output.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// Output handles time series input from files.
//
////////////////////////////////////////////////////////////////

#ifndef Output_hh
#define Output_hh

#include <string>
#include <vector>
#include <cstdio>
#include "Params.hh"
#include "OpsInfo.hh"
#include "Pulse.hh"
using namespace std;

////////////////////////
// This class

class Output {
  
public:
  
  // constructor
  
  Output (const string &label,
          const Params &params);
  
  // destructor
  
  ~Output();
  
  // open an output file
  // Returns 0 if file already open, -1 if not
  
  int openFile(time_t startTime, 
               double startEl,
               double startAz);

  // close file
  // Returns 0 if file already open, -1 if not
  
  int closeFile();

  // write the info
  
  int writeInfo(const OpsInfo &info);

  // write the pulse header
  
  int writePulseHeader(const Pulse &pulse);
  
  // write the pulse data
  
  int writeIQ(const void *iqPacked, int nBytesPacked);

protected:
  
private:
  
  // basic
  
  string _label;
  const Params &_params;

  // output file
  
  string _outputPath;
  FILE *_out;

};

#endif

